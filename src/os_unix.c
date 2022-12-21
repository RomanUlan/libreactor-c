#include "reactor/os.h"
#include <unistd.h>

void os_linux_init(os *o)
{
  if (o) {
    o->epoll_create1 = epoll_create1;
    o->epoll_ctl = epoll_ctl;
    o->epoll_wait = epoll_wait;
    o->close = close;
    o->socket = socket;
    o->bind = bind;
    o->listen = listen;
    o->accept = accept;
    o->read = read;
    o->write = write;
  }
}

