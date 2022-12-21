#ifdef __cplusplus
  extern "C" {
    #include "reactor/reactor.h"
  }
#endif

#include <string.h>
#include <vector>
#include <map>
#include <thread>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <cmock/cmock.h>

using namespace std;
using namespace testing;

//static int mock_epoll_create1(int);
//static int mock_epoll_ctl(int, int, int, struct epoll_event *);
//static int mock_close(int fd);

class mock_os: public CMockMocker<mock_os>
{
  public:
    CMOCK_MOCK_METHOD(int, mock_epoll_create1, (int));
    CMOCK_MOCK_METHOD(int, mock_epoll_ctl, (int, int, int, struct epoll_event *));
    CMOCK_MOCK_METHOD(int, mock_epoll_wait, (int, struct epoll_event *, int, int));
    CMOCK_MOCK_METHOD(int, mock_close, (int));
};

CMOCK_MOCK_FUNCTION(mock_os, int, mock_epoll_create1, (int));
CMOCK_MOCK_FUNCTION(mock_os, int, mock_epoll_ctl, (int, int, int, struct epoll_event *));
CMOCK_MOCK_FUNCTION(mock_os, int, mock_epoll_wait, (int, struct epoll_event *, int, int));
CMOCK_MOCK_FUNCTION(mock_os, int, mock_close, (int fd));

class mock_eh: public CMockMocker<mock_eh>
{
  public:
    CMOCK_MOCK_METHOD(void, mock_handle_event, (event_handler *, uint32_t));
};

CMOCK_MOCK_FUNCTION(mock_eh, void, mock_handle_event, (event_handler *, uint32_t));

/*TEST(tests_reactor, test_fw)
{
  ASSERT_TRUE(false);
}*/


TEST(tests_reactor, init_constructor_and_destructor_with_nulls)
{
  ASSERT_NE(reactor_init(0, 0), 0);

  os o;
  memset(&o, 0 ,sizeof(os));

  ASSERT_NE(reactor_init(0, &o), 0);

  reactor r;
  ASSERT_NE(reactor_init(&r, 0), 0);
}

TEST(tests_reactor, epoll_create_fails)
{
  mock_os mos;
  os o;
  memset(&o, 0 ,sizeof(os));
  o.epoll_create1 = mock_epoll_create1;

  EXPECT_CALL(mos, mock_epoll_create1(_)).WillOnce(Return(-1));

  reactor r;
  ASSERT_NE(reactor_init(&r, &o), 0);
}

TEST(tests_reactor, epoll_create_success)
{
  mock_os mos;
  os o;
  memset(&o, 0 ,sizeof(os));
  o.epoll_create1 = mock_epoll_create1;
  o.close = mock_close;

  const int epoll_fd = 10;
  EXPECT_CALL(mos, mock_epoll_create1(_)).WillOnce(Return(epoll_fd));
  EXPECT_CALL(mos, mock_close(epoll_fd)).WillOnce(Return(0));

  reactor r;
  ASSERT_EQ(reactor_init(&r, &o), 0);
  ASSERT_NE(r.ctx, nullptr);
  r.destroy(&r);
  ASSERT_EQ(r.ctx, nullptr);
}


TEST(tests_reactor, alloc_constructor_with_null)
{
  reactor *r = reactor_alloc(0);
  ASSERT_EQ(r, nullptr);
}

TEST(tests_reactor, alloc_constructor_and_destructor)
{
  mock_os mos;
  os o;
  memset(&o, 0 ,sizeof(os));
  o.epoll_create1 = mock_epoll_create1;
  o.close = mock_close;

  const int epoll_fd = 10;
  EXPECT_CALL(mos, mock_epoll_create1(_)).WillOnce(Return(epoll_fd));
  EXPECT_CALL(mos, mock_close(epoll_fd)).WillOnce(Return(0));

  reactor *r = reactor_alloc(&o);
  ASSERT_NE(r, nullptr);
  r->destroy(r);
}

TEST(tests_reactor, register_eh_null)
{
  mock_os mos;
  os o;
  memset(&o, 0 ,sizeof(os));
  o.epoll_create1 = mock_epoll_create1;
  o.close = mock_close;
  o.epoll_ctl = mock_epoll_ctl;

  const int epoll_fd = 10;
  EXPECT_CALL(mos, mock_epoll_create1(_)).WillOnce(Return(epoll_fd));
  EXPECT_CALL(mos, mock_close(epoll_fd)).WillOnce(Return(0));

  reactor r;
  ASSERT_EQ(reactor_init(&r, &o), 0);
  ASSERT_NE(r.ctx, nullptr);

  ASSERT_NE(r.register_eh(0, 0), 0);
  ASSERT_NE(r.register_eh(&r, 0), 0);

  r.destroy(&r);
  ASSERT_EQ(r.ctx, nullptr);
}

TEST(tests_reactor, register_eh_fails)
{
  mock_os mos;
  os o;
  memset(&o, 0 ,sizeof(os));
  o.epoll_create1 = mock_epoll_create1;
  o.close = mock_close;
  o.epoll_ctl = mock_epoll_ctl;

  const int epoll_fd = 10;
  const int registered_fd = 20;
  EXPECT_CALL(mos, mock_epoll_create1(_)).WillOnce(Return(epoll_fd));
  EXPECT_CALL(mos, mock_close(epoll_fd)).WillOnce(Return(0));
  EXPECT_CALL(mos, mock_epoll_ctl(epoll_fd, _, registered_fd, Ne(nullptr))).WillOnce(Return(-1));

  reactor r;
  ASSERT_EQ(reactor_init(&r, &o), 0);
  ASSERT_NE(r.ctx, nullptr);

  event_handler eh;
  memset(&eh, 0, sizeof(eh));
  eh.fd = registered_fd;

  ASSERT_NE(r.register_eh(&r, &eh), 0);

  r.destroy(&r);
  ASSERT_EQ(r.ctx, nullptr);
}

TEST(tests_reactor, unregister_eh_fails_with_nulls)
{
  mock_os mos;
  os o;
  memset(&o, 0 ,sizeof(os));
  o.epoll_create1 = mock_epoll_create1;
  o.close = mock_close;

  const int epoll_fd = 10;
  EXPECT_CALL(mos, mock_epoll_create1(_)).WillOnce(Return(epoll_fd));
  EXPECT_CALL(mos, mock_close(epoll_fd)).WillOnce(Return(0));

  reactor r;
  ASSERT_EQ(reactor_init(&r, &o), 0);
  ASSERT_NE(r.ctx, nullptr);

  ASSERT_NE(r.unregister_eh(0, 0), 0);
  ASSERT_NE(r.unregister_eh(&r, 0), 0);
  event_handler eh;
  ASSERT_NE(r.unregister_eh(0, &eh), 0);

  r.destroy(&r);
  ASSERT_EQ(r.ctx, nullptr);
}

TEST(tests_reactor, register_double_unregister)
{
  mock_os mos;
  os o;
  memset(&o, 0 ,sizeof(os));
  o.epoll_create1 = mock_epoll_create1;
  o.close = mock_close;
  o.epoll_ctl = mock_epoll_ctl;

  const int epoll_fd = 10;
  const int registered_fd = 20;
  EXPECT_CALL(mos, mock_epoll_create1(_)).WillOnce(Return(epoll_fd));
  EXPECT_CALL(mos, mock_close(epoll_fd)).WillOnce(Return(0));
  EXPECT_CALL(mos, mock_epoll_ctl(epoll_fd, _, registered_fd, Ne(nullptr))).WillOnce(Return(0));
  EXPECT_CALL(mos, mock_epoll_ctl(epoll_fd, _, registered_fd, Eq(nullptr))).WillOnce(Return(0));

  reactor r;
  ASSERT_EQ(reactor_init(&r, &o), 0);
  ASSERT_NE(r.ctx, nullptr);

  event_handler eh;
  eh.fd = registered_fd;
  ASSERT_EQ(r.register_eh(&r, &eh), 0);

  ASSERT_EQ(r.unregister_eh(&r, &eh), 0);
  ASSERT_NE(r.unregister_eh(&r, &eh), 0);

  r.destroy(&r);
  ASSERT_EQ(r.ctx, nullptr);
}

TEST(tests_reactor, double_register_the_same_eh_with_the_same_socket_fails_unregister)
{
  mock_os mos;
  os o;
  memset(&o, 0 ,sizeof(os));
  o.epoll_create1 = mock_epoll_create1;
  o.close = mock_close;
  o.epoll_ctl = mock_epoll_ctl;

  const int epoll_fd = 10;
  const int registered_fd = 20;
  EXPECT_CALL(mos, mock_epoll_create1(_)).WillOnce(Return(epoll_fd));
  EXPECT_CALL(mos, mock_close(epoll_fd)).WillOnce(Return(0));
  EXPECT_CALL(mos, mock_epoll_ctl(epoll_fd, _, registered_fd, Ne(nullptr))).WillOnce(Return(0));
  EXPECT_CALL(mos, mock_epoll_ctl(epoll_fd, _, registered_fd, Eq(nullptr))).WillOnce(Return(0));

  reactor r;
  ASSERT_EQ(reactor_init(&r, &o), 0);
  ASSERT_NE(r.ctx, nullptr);

  event_handler eh;
  eh.fd = registered_fd;
  ASSERT_EQ(r.register_eh(&r, &eh), 0);
  ASSERT_NE(r.register_eh(&r, &eh), 0);

  ASSERT_EQ(r.unregister_eh(&r, &eh), 0);

  r.destroy(&r);
  ASSERT_EQ(r.ctx, nullptr);
}

TEST(tests_reactor, double_register_the_same_eh_with_different_socket_fails_unregister)
{
  mock_os mos;
  os o;
  memset(&o, 0 ,sizeof(os));
  o.epoll_create1 = mock_epoll_create1;
  o.close = mock_close;
  o.epoll_ctl = mock_epoll_ctl;

  const int epoll_fd = 10;
  const int registered_fd = 20;
  const int registered_fd2 = 30;
  EXPECT_CALL(mos, mock_epoll_create1(_)).WillOnce(Return(epoll_fd));
  EXPECT_CALL(mos, mock_close(epoll_fd)).WillOnce(Return(0));
  EXPECT_CALL(mos, mock_epoll_ctl(epoll_fd, _, registered_fd, Ne(nullptr))).WillOnce(Return(0));
  EXPECT_CALL(mos, mock_epoll_ctl(epoll_fd, _, registered_fd, Eq(nullptr))).WillOnce(Return(0));

  reactor r;
  ASSERT_EQ(reactor_init(&r, &o), 0);
  ASSERT_NE(r.ctx, nullptr);

  event_handler eh;
  eh.fd = registered_fd;
  ASSERT_EQ(r.register_eh(&r, &eh), 0);

  eh.fd = registered_fd2;
  ASSERT_NE(r.register_eh(&r, &eh), 0);

  eh.fd = registered_fd;
  ASSERT_EQ(r.unregister_eh(&r, &eh), 0);

  r.destroy(&r);
  ASSERT_EQ(r.ctx, nullptr);
}

TEST(tests_reactor, double_register_two_ehs_for_the_same_socket_fails)
{
  mock_os mos;
  os o;
  memset(&o, 0 ,sizeof(os));
  o.epoll_create1 = mock_epoll_create1;
  o.close = mock_close;
  o.epoll_ctl = mock_epoll_ctl;

  const int epoll_fd = 10;
  const int registered_fd = 20;
  EXPECT_CALL(mos, mock_epoll_create1(_)).WillOnce(Return(epoll_fd));
  EXPECT_CALL(mos, mock_close(epoll_fd)).WillOnce(Return(0));
  EXPECT_CALL(mos, mock_epoll_ctl(epoll_fd, _, registered_fd, Ne(nullptr))).WillOnce(Return(0));
  EXPECT_CALL(mos, mock_epoll_ctl(epoll_fd, _, registered_fd, Eq(nullptr))).WillOnce(Return(0));

  reactor r;
  ASSERT_EQ(reactor_init(&r, &o), 0);
  ASSERT_NE(r.ctx, nullptr);

  event_handler eh1;
  eh1.fd = registered_fd;
  ASSERT_EQ(r.register_eh(&r, &eh1), 0);

  event_handler eh2;
  eh2.fd = registered_fd;
  ASSERT_NE(r.register_eh(&r, &eh2), 0);

  ASSERT_EQ(r.unregister_eh(&r, &eh1), 0);

  r.destroy(&r);
  ASSERT_EQ(r.ctx, nullptr);
}

TEST(tests_reactor, register_unregister_x_event_handlers)
{
  mock_os mos;
  os o;
  memset(&o, 0 ,sizeof(os));
  o.epoll_create1 = mock_epoll_create1;
  o.close = mock_close;
  o.epoll_ctl = mock_epoll_ctl;

  const int epoll_fd = 5;
  EXPECT_CALL(mos, mock_epoll_create1(_)).WillOnce(Return(epoll_fd));
  EXPECT_CALL(mos, mock_close(epoll_fd)).WillOnce(Return(0));

  const int ehs_cnt = 100;
  const int registered_fd_starting = 10;
  vector<event_handler> ehs(ehs_cnt);

  {
    InSequence s1;
    for (int i = 0; i < ehs_cnt; ++i) {
      ehs[i].fd = registered_fd_starting * (i+1);
      EXPECT_CALL(mos, mock_epoll_ctl(epoll_fd, _, ehs[i].fd, Ne(nullptr))).WillOnce(Return(0)).RetiresOnSaturation();
    }
  }

  {
    InSequence s2;
    for (auto & eh: ehs) {
      EXPECT_CALL(mos, mock_epoll_ctl(epoll_fd, _, eh.fd, Eq(nullptr))).WillOnce(Return(0)).RetiresOnSaturation();
    }
  }

  reactor r;
  ASSERT_EQ(reactor_init(&r, &o), 0);
  ASSERT_NE(r.ctx, nullptr);


  for (auto & eh: ehs) {
    ASSERT_EQ(r.register_eh(&r, &eh), 0);
  }

  for (auto & eh: ehs) {
    ASSERT_EQ(r.unregister_eh(&r, &eh), 0);
  }

  r.destroy(&r);
  ASSERT_EQ(r.ctx, nullptr);
}

TEST(tests_reactor, destructor_after_x_eh_registration)
{
  mock_os mos;
  os o;
  memset(&o, 0 ,sizeof(os));
  o.epoll_create1 = mock_epoll_create1;
  o.epoll_ctl = mock_epoll_ctl;
  o.close = mock_close;

  const int epoll_fd = 5;
  EXPECT_CALL(mos, mock_epoll_create1(_)).WillOnce(Return(epoll_fd));
  EXPECT_CALL(mos, mock_close(epoll_fd)).WillOnce(Return(0));

  const int ehs_cnt = 2;
  const int registered_fd_starting = 10;
  vector<event_handler> ehs(ehs_cnt);

  for (int i = 0; i < ehs_cnt; ++i) {
    ehs[i].fd = registered_fd_starting * (i+1);
    EXPECT_CALL(mos, mock_epoll_ctl(epoll_fd, _, ehs[i].fd, Ne(nullptr))).WillOnce(Return(0)).RetiresOnSaturation();
    EXPECT_CALL(mos, mock_epoll_ctl(epoll_fd, _, ehs[i].fd, Eq(nullptr))).WillOnce(Return(0));
  }

  reactor r;
  ASSERT_EQ(reactor_init(&r, &o), 0);
  ASSERT_NE(r.ctx, nullptr);


  for (auto & eh: ehs) {
    ASSERT_EQ(r.register_eh(&r, &eh), 0);
  }

  r.destroy(&r);
  ASSERT_EQ(r.ctx, nullptr);
}

ACTION_P(set_events, events)
{
  ASSERT_LE(events.size(), (size_t) arg2);

  size_t i = 0;
  for (auto e: events) {
    arg1[i].data.fd = e.first;
    arg1[i++].events = e.second;
  }
}

TEST(tests_reactor, handle_X_events_for_one_eh)
{
  mock_os mos;
  os o;
  memset(&o, 0 ,sizeof(os));
  o.epoll_create1 = mock_epoll_create1;
  o.close = mock_close;
  o.epoll_ctl = mock_epoll_ctl;
  o.epoll_wait = mock_epoll_wait;

  const int epoll_fd = 10;
  const int registered_fd = 20;
  EXPECT_CALL(mos, mock_epoll_create1(_)).WillOnce(Return(epoll_fd));
  EXPECT_CALL(mos, mock_close(epoll_fd)).WillOnce(Return(0));
  EXPECT_CALL(mos, mock_epoll_ctl(epoll_fd, _, registered_fd, Ne(nullptr))).WillOnce(Return(0));
  EXPECT_CALL(mos, mock_epoll_ctl(epoll_fd, _, registered_fd, Eq(nullptr))).WillOnce(Return(0));

  const int events_cnt = 100;
  map<int, uint32_t> events = {
                                { registered_fd, EPOLLIN }
                              };
  {
    InSequence s;
    for (int i = 0; events_cnt > i; ++i) {
      EXPECT_CALL(mos, mock_epoll_wait(epoll_fd, _, _, _))
        .WillOnce(DoAll(set_events(events), Return(events.size())))
        .RetiresOnSaturation();
    }
    EXPECT_CALL(mos, mock_epoll_wait(epoll_fd, _, _, _)).WillOnce(Return(-1)).RetiresOnSaturation();
  }

  mock_eh meh;
  event_handler eh;
  memset(&eh, 0, sizeof(eh));
  eh.fd = registered_fd;
  eh.handle_event = mock_handle_event;
  EXPECT_CALL(meh, mock_handle_event(&eh, EPOLLIN)).Times(events_cnt);

  reactor r;
  ASSERT_EQ(reactor_init(&r, &o), 0);
  ASSERT_NE(r.ctx, nullptr);

  ASSERT_EQ(r.register_eh(&r, &eh), 0);
  r.event_loop(&r);

  r.destroy(&r);
  ASSERT_EQ(r.ctx, nullptr);
}

TEST(tests_reactor, check_event_loop_break)
{
  mock_os mos;
  os o;
  memset(&o, 0 ,sizeof(os));
  o.epoll_create1 = mock_epoll_create1;
  o.close = mock_close;
  o.epoll_wait = mock_epoll_wait;

  const int epoll_fd = 10;
  EXPECT_CALL(mos, mock_epoll_create1(_)).WillOnce(Return(epoll_fd));
  EXPECT_CALL(mos, mock_close(epoll_fd)).WillOnce(Return(0));
  EXPECT_CALL(mos, mock_epoll_wait(epoll_fd, _, _, _)).WillRepeatedly(Return(0));

  reactor r;
  ASSERT_EQ(reactor_init(&r, &o), 0);
  ASSERT_NE(r.ctx, nullptr);

  thread reactor_thread([&r] () { r.event_loop(&r); });
  this_thread::sleep_for(chrono::milliseconds(250));
  r.stop(&r);
  reactor_thread.join();

  r.destroy(&r);
  ASSERT_EQ(r.ctx, nullptr);
}

TEST(tests_reactor, check_null_validation_in_public_methods)
{
  mock_os mos;
  os o;
  memset(&o, 0 ,sizeof(os));
  o.epoll_create1 = mock_epoll_create1;
  o.close = mock_close;

  const int epoll_fd = 10;
  EXPECT_CALL(mos, mock_epoll_create1(_)).WillOnce(Return(epoll_fd));
  EXPECT_CALL(mos, mock_close(epoll_fd)).WillOnce(Return(0));

  reactor *r = reactor_alloc(&o);
  ASSERT_NE(r, nullptr);
  ASSERT_NE(r->register_eh(0, 0), 0);
  ASSERT_NE(r->unregister_eh(0, 0), 0);
  r->event_loop(0);
  r->stop(0);
  r->destroy(0);
  r->destroy(r); //just to avoid memory leak
}

