/**
 * @file os.h
 * @brief This header contains declaration of proxy object (os)
 * to system calls.
 * @author Roman Ulan
 * @version 1.0
 * @date 2021-07-15
 */

#ifndef OS_H
#define OS_H

#include <sys/socket.h>
#include <sys/epoll.h>

/**
 * @brief This is structure which is a proxy to system calls.
 * It should be used to enable unit/module testing of whole
 * so called business logic embedded in modules, which relies
 * on system calls.
 */
typedef struct os_s {
  int (*epoll_create1)(int);
  int (*epoll_ctl)(int, int, int, struct epoll_event *);
  int (*epoll_wait)(int, struct epoll_event *, int, int);
  int (*close)(int);
  int (*socket)(int, int, int);
  int (*bind)(int, const struct sockaddr *, socklen_t );
  int (*listen)(int, int);
  int (*accept)(int, struct sockaddr *, socklen_t *);
  ssize_t (*read)(int, void *, size_t);
  ssize_t (*write)(int, const void *, size_t);
} os;

/**
 * @brief A constructor with sets the pointers to operating system calls.
 * Please note: there is not special cleanup needed at the end.
 *
 * @param o A pointer to os object.
 */
void os_linux_init(os *o);

#endif

