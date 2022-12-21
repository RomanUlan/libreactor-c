/**
 * @file main.c
 * @brief This is very basic telnet echo server, which demonstrates
 * how to use reactor library.
 * To build this project just run make command in one up folder.
 * To run this project just run run.sh script in one up folder.
 * @author Roman Ulan
 * @version 1.0
 * @date 2021-07-15
 */

#include "reactor/reactor.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

static void sig_handler(int sig);
static event_handler * init_srv_eh(int port);
static event_handler * init_cli_eh(int cli_fd);
static void destroy_eh(event_handler *eh);

static void accept_client(event_handler *self, uint32_t events);
static void echo_reply(event_handler *self, uint32_t events);

reactor REACTOR;

int main(int argc, char **argv)
{
  os o;
  const int port = 5555;
  event_handler *srv_eh = init_srv_eh(port);

  if (!srv_eh) {
    perror("Cannot setup server.");
    return 1;
  }

  signal(SIGINT, sig_handler);

  os_linux_init(&o);
  reactor_init(&REACTOR, &o);

  REACTOR.register_eh(&REACTOR, srv_eh);

  printf("Server setup using port %d.\n", port);
  printf("Press <ctrl>+<c> to stop it.\n");
  REACTOR.event_loop(&REACTOR);
  printf("\nServer interrupted, bye...\n");

  REACTOR.destroy(&REACTOR);
  srv_eh->destroy(srv_eh);

  return 0;
}

static void sig_handler(int sig)
{
  REACTOR.stop(&REACTOR);
}

static event_handler * init_srv_eh(int port)
{
  struct sockaddr_in addr;
  event_handler *srv_eh = 0;
  int srv_fd = socket(AF_INET, SOCK_STREAM, 0);

  if (0 > srv_fd) {
    return srv_eh;
  }

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = INADDR_ANY;

  if (0 != bind(srv_fd, (struct sockaddr*) &addr, sizeof(addr))) {
    return srv_eh;
  }

  if (listen(srv_fd, 512) < 0) {
    return srv_eh;
  }

  srv_eh = malloc(sizeof(event_handler));
  memset(srv_eh, 0, sizeof(event_handler));
  srv_eh->fd = srv_fd;
  srv_eh->handle_event = accept_client;
  srv_eh->destroy = destroy_eh;

  return srv_eh;
}

static event_handler * init_cli_eh(int cli_fd)
{
  event_handler *cli_eh = malloc(sizeof(event_handler));
  memset(cli_eh, 0, sizeof(event_handler));
  cli_eh->fd = cli_fd;
  cli_eh->handle_event = echo_reply;
  cli_eh->destroy = destroy_eh;

  return cli_eh;
}

static void destroy_eh(event_handler *eh)
{
  close(eh->fd);
  memset(eh, 0, sizeof(event_handler));
  free(eh);
}

static void accept_client(event_handler *self, uint32_t events)
{
  const int cli_fd = accept(self->fd, 0, 0);
  if (0 < cli_fd) {
    event_handler *cli_eh = init_cli_eh(cli_fd);
    REACTOR.register_eh(&REACTOR, cli_eh);
    printf("New connection to client %d...\n", cli_fd);
  }
}

static void echo_reply(event_handler *self, uint32_t events)
{
  const size_t frame_size = 1024;
  char buff[frame_size];
  memset(buff, 0, sizeof(buff));
  const ssize_t cnt = read(self->fd, buff, frame_size);
  if (1 > cnt) {
    printf("Connection lost to client %d...\n", self->fd);
    REACTOR.unregister_eh(&REACTOR, self);
    free(self);
  }
  else {
    if (cnt != write(self->fd, buff, cnt)) {
      printf("Connection lost to client %d...\n", self->fd);
      REACTOR.unregister_eh(&REACTOR, self);
      free(self);
    }
  }
}

