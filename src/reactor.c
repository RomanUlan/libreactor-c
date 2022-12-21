#include "reactor/reactor.h"
#include <stdlib.h>
#include <string.h>

typedef struct event_handler_node_s {
  event_handler *eh;
  struct event_handler_node_s *next;
} event_handler_node;

struct reactor_ctx_s {
  int epoll_fd;
  const os *o;
  event_handler_node *root;
  int run;
};

static void reactor_terminate(reactor *self);
static void reactor_free(reactor *self);
static int reactor_register_eh(reactor *self, event_handler *e);
static int reactor_unregister_eh(reactor *self, const event_handler *e);
static void reactor_event_loop(reactor *self);
static void reactor_stop(reactor *self);
static int reactor_validateDuplicate(event_handler_node *root, const event_handler *eh);
static event_handler_node * reactor_find_eh(event_handler_node *root, const int fd);

int reactor_init(reactor *r, const os *o)
{
  if ( (!r) || (!o) )
    return -1;

  const int epoll_fd = o->epoll_create1(0);
  if (epoll_fd < 0) {
    return -1;
  }

  memset(r, 0, sizeof(reactor));
  reactor_ctx *ctx = (reactor_ctx *) malloc(sizeof(reactor_ctx));
  memset(ctx, 0, sizeof(reactor_ctx));

  ctx->o = o;
  ctx->epoll_fd = epoll_fd;

  r->ctx = ctx;
  r->register_eh = reactor_register_eh;
  r->unregister_eh = reactor_unregister_eh;
  r->event_loop = reactor_event_loop;
  r->stop = reactor_stop;
  r->destroy = reactor_terminate;

  return 0;
}

reactor * reactor_alloc(const os *o)
{

  reactor *res = 0;
  if (o) {
    res = (reactor *) malloc(sizeof(reactor));
    reactor_init(res, o);
    res->destroy = reactor_free;
  }

  return res;
}

static void reactor_terminate(reactor *self)
{
  if (self && self->ctx && self->ctx->o) {
    while (self->ctx->root) {
      reactor_unregister_eh(self, self->ctx->root->eh);
    }
    self->ctx->o->close(self->ctx->epoll_fd);
    free(self->ctx);
    self->ctx = 0;
  }
}

static void reactor_free(reactor *self)
{
  if (self) {
    reactor_terminate(self);
    free(self);
    self = 0;
  }
}

static int reactor_register_eh(reactor *self, event_handler *eh)
{
  if ( (!self) || (!self->ctx) || (!eh) ) {
    return -1;
  }

  if (0 != reactor_validateDuplicate(self->ctx->root, eh)) {
      return -1;
  }

  const int epoll_fd = self->ctx->epoll_fd;
  const int fd = eh->fd;

  struct epoll_event ee;
  memset(&ee, 0, sizeof(ee));
  ee.data.fd = fd;
  ee.events = 0 | EPOLLIN;

  int res = self->ctx->o->epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ee);

  if (0 == res) {
    event_handler_node *new_root = (event_handler_node*) malloc(sizeof (event_handler_node));
    memset(new_root, 0, sizeof(event_handler_node));
    new_root->eh = eh;
    new_root->next = self->ctx->root;
    self->ctx->root = new_root;
  }

  return res;
}

static int reactor_unregister_eh(reactor *self, const event_handler *eh)
{
  if ( (!self) || (!self->ctx) || (!eh) ) {
    return -1;
  }

  const int epoll_fd = self->ctx->epoll_fd;
  const int fd = eh->fd;

  event_handler_node *prev = 0;
  event_handler_node *curr = self->ctx->root;
  for (; 0 != curr; prev = curr, curr = curr->next)
    if ( eh == curr->eh )
      break;

  int res = -1;
  if (curr) {
    if (curr == self->ctx->root)
      self->ctx->root = curr->next;
    else
      prev->next = curr->next;

    free(curr);
    res = self->ctx->o->epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, 0);
  }

  return res;
}

static void reactor_event_loop(reactor *self)
{
  if ( (!self) || (!self->ctx) || (!self->ctx->o) ) {
    return;
  }

  const int epoll_fd = self->ctx->epoll_fd;
  const int wait_timeout_ms = 250;
  const int max_events = 10;
  struct epoll_event evs[max_events];
  self->ctx->run = 1;
  while (self->ctx->run) {
    const int events_cnt = self->ctx->o->epoll_wait(epoll_fd, evs, max_events, wait_timeout_ms);
    if (events_cnt < 0) {
      return;
    }
    else {
      for (int i = 0; i < events_cnt; ++i) {
        const int fd = evs[i].data.fd;
        event_handler_node* ehn = reactor_find_eh(self->ctx->root, fd);
        if (ehn)
          ehn->eh->handle_event(ehn->eh, evs[i].events);
      }
    }
  }
}

static void reactor_stop(reactor *self)
{
  if ( (!self) || (!self->ctx) ) {
    return;
  }

  self->ctx->run = 0;
}

static int reactor_validateDuplicate(event_handler_node *root, const event_handler *eh)
{
  int res = 0;
  for (event_handler_node *curr = root; 0 != curr; curr = curr->next) {
    if ( ( (eh) && (curr->eh) && (eh->fd == curr->eh->fd) ) || (eh == curr->eh) ) {
      res = -1;
      break;
    }
  }

  return res;
}

static event_handler_node * reactor_find_eh(event_handler_node *root, const int fd)
{
  event_handler_node *res = 0;

  for (event_handler_node *curr = root; 0 != curr; curr = curr->next) {
    if ( (curr->eh) && (fd == curr->eh->fd) ) {
      res = curr;
      break;
    }
  }

  return res;
}

