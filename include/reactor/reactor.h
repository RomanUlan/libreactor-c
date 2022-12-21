/**
 * @file reactor.h
 * @brief This is a main iclude file with necessary types to use
 * implemented in this libray reactive event demultiplexer for file
 * based descriptors.
 * @author Roman Ulan
 * @version 1.0
 * @date 2021-07-15
 */

#ifndef REACTOR_H
#define REACTOR_H

#include "os.h"

/**
 * @brief Just a helper typedef for shorter name usage for
 * event_handler_s structure.
 */
typedef struct event_handler_s event_handler;

/**
 * @brief This is object oriented callback object, which is used
 * to parir file descriptor with function, which will be called when
 * an event occur on this descriptor.
 * Such event_handler can be registered in reactor to react on events,
 * which will occur on fd.
 */
struct event_handler_s {
  /**
   * @brief File descriptor like: pipe, socket...
   */
  int fd;
  /**
   * @brief This is a context, which can be used to keep any private
   * data (like internal state) by event handler. There is guarantee
   * that reactor will never change it.
   */
  void *ctx;
  /**
   * @brief This is a OOP like method which will be called when
   * some evet will occurr on fd.
   *
   * @param events an epoll event mask for fd.
   */
  void (*handle_event)(event_handler *self, uint32_t events);
  /**
   * @brief It is a pointer to function which will be used to
   * destroy event_handler. It's just to keep the OOP convention,
   * but it is not used by reactor at any time.
   *
   * @param self It is a pointer to an event_handler wherefrom
   * this method is called.
   */
  void (*destroy)(event_handler *self);
};

/**
 * @brief Just a helper typedef for shorter name usage for
 * reactor_s structure.
 */
typedef struct reactor_s reactor;
/**
 * @brief Just a forward declaration and helper typedef for shorter
 * name usage for reactor_ctx_s structure. It is just a place for
 * private data of reactor. As a user of reactor class, you should
 * never use this member.
 */
typedef struct reactor_ctx_s reactor_ctx;
struct reactor_s {
  /**
   * @brief It is just a place for reactor's private.
   * As a user of reactor class, you should never use this member.
   */
  reactor_ctx *ctx;
  /**
   * @brief This is a method which you can use to register your
   * event_handler. Afterwards some events will come into related
   * with event_handler fd, reactor will trigger method handle_event
   * int this event_handler.
   *
   * @param self It is a pointer to the reactor wherefrom this method
   * is called.
   * @param e An event handler. Please note: reactor does not take
   * the ownership for this pointer.
   */
  int (*register_eh)(reactor *self, event_handler *e);
  /**
   * @brief This methis should be used to unregister an event_handler
   * from the reactor. It should be always used if system is not
   * interested to react on events on related with unregiestered
   * event_handler fd.
   *
   * @param self It is a pointer to the reactor wherefrom this method
   * is called.
   * @param e An event handler. Please note: reactor does not call
   * the destructor of event_handler.
   */
  int (*unregister_eh)(reactor *self, const event_handler *e);
  /**
   * @brief This is heart of the reactor - main event loop. It is a blocking
   * method, wich has embedded loop with waiting for events at registered
   * event_handler's fd's. You can break this loop by calling stop method
   * in the reactor.
   *
   * @param self It is a pointer to the reactor wherefrom this method
   * is called.
   */
  void (*event_loop)(reactor *self);
  /**
   * @brief This method should be called to break event_loop method call.
   *
   * @param self It is a pointer to the reactor wherefrom this method
   * is called.
   */
  void (*stop)(reactor *self);
  /**
   * @brief This is destructor. You should call this method once reactor
   * won't be used anymore to avoid memory leaks. Note: if thre will be some
   * reigstered event_handler's, destructor will unregister all of them, but
   * still it will not call theris destructors.
   *
   * @param self It is a pointer to the reactor wherefrom this method
   * is called.
   */
  void (*destroy)(reactor *self);
};

/**
 * @brief It's constructor for stacked reactors.
 *
 * @param r Reactor stacked instance.
 * @param o Proxy to operating system calls.
 *
 * @return 0 in case of success, -1 otherwise.
 */
int reactor_init(reactor *r, const os *o);
/**
 * @brief It's constructor to dynamically alloc reactor.
 *
 * @param o Proxy to operating system calls.
 *
 * @return 0 in case of success, -1 otherwise.
 */
reactor * reactor_alloc(const os *o);

#endif

