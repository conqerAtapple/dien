/******************************************************************************
 *
 *  File:   future_tests.cpp
 *  Author: Jojy G Varghese
 *
 *  Description: Test suite for `Future` class.
 *
 ******************************************************************************/

#include <glog/logging.h>

#include "gtest/gtest.h"

#include "future.hpp"

using namespace dien;

TEST(FutureTests, SharedData)
{
  SharedData<int, true> sd;

  ASSERT_FALSE(sd.Ready());
  sd.DetachOne();
  sd.DetachOne();
}

TEST(FutureTests, SharedDataCallBack)
{
  SharedData<int, true> sd;
  ASSERT_FALSE(sd.Ready());

  bool is_callback_called = false;

  std::function<void(Try<int> && )> callback = [&](Try<int>&& data) {
    is_callback_called = true;
  };

  sd.SetCallback(callback);

  sd.SetResult(1024);
  ASSERT_TRUE(sd.Ready());

  ASSERT_TRUE(is_callback_called);
}

#include <cstdlib>
#include <memory>
#include <cxxabi.h>

std::string demangle(const char* name)
{

  int status = -4;  // some arbitrary value to eliminate the compiler warning

  // enable c++11 by passing the flag -std=c++11 to g++
  std::unique_ptr<char, void (*)(void*)> res{
      abi::__cxa_demangle(name, NULL, NULL, &status), std::free};

  return (status == 0) ? res.get() : name;
}

TEST(FutureTests, ThenCallback)
{
  {
    bool is_continuation_called = false;
    int value = 0;
    Promise<int> promise;
    Future<int> fu = promise.GetFuture();

    Future<int> r = fu.Then([&](Try<int>& v) {
      is_continuation_called = true;
      value = v.Value();
      return 2;
    });

    ASSERT_FALSE(r.HasValue());
    promise.SetValue(3);

    ASSERT_TRUE(is_continuation_called);
    ASSERT_EQ(value, 3);

    ASSERT_TRUE(r.HasValue());
  }
#if 1
  {
    bool is_continuation_called = false;
    int value = 0;
    Promise<int> promise;
    Future<int> fu = promise.GetFuture();

    Future<void> r = fu.Then([&](Try<int>& v) {
      is_continuation_called = true;
      value = v.Value();
      return;
    });

    // ASSERT_FALSE(r.HasValue());
    promise.SetValue(3);

    ASSERT_TRUE(is_continuation_called);
    ASSERT_EQ(value, 3);
  }
#endif
}

TEST(FutureTests, OnErrorCallbackWithValue)
{
  {
    bool is_then_continuation_called = false;
    bool is_err_continuation_called = false;
    std::string error_msg;

    Promise<int> promise;
    Future<int> fu = promise.GetFuture();

    Future<int> r = fu.Then([&](int v) {
                              is_then_continuation_called = true;
                              return 2;
                            })
                        .OnError([&](Error&& err) {
                           is_err_continuation_called = true;
                           const Error::ErrorStack& s = err;
                           error_msg = s.top().Message();
                           return 2;
                         });

    ASSERT_FALSE(r.HasValue());
    promise.SetError(Error("test error"));

    ASSERT_TRUE(is_err_continuation_called);
    ASSERT_FALSE(is_then_continuation_called);
    ASSERT_EQ(error_msg, "test error");

    ASSERT_EQ(r.Value(), 2);
  }
}

TEST(FutureTests, OnErrorCallbackWithTry)
{
  {
    bool is_then_continuation_called = false;
    bool is_err_continuation_called = false;
    std::string error_msg;

    Promise<int> promise;
    Future<int> fu = promise.GetFuture();

    Future<int> r = fu.Then([&](Try<int> & v)->Future<int> {
                              LOG(INFO) << "then callback called";
                              is_then_continuation_called = true;

                              if (v.HasError()) {
                                return FailedFuture();
                              } else {
                                return 1;
                              }
                            })
                        .OnError([&](Error&& err) {
                           is_err_continuation_called = true;
                           const Error::ErrorStack& s = err;
                           error_msg = s.top().Message();
                           return 2;
                         });

    ASSERT_FALSE(r.HasValue());
    promise.SetError(Error("test error"));

    ASSERT_TRUE(is_err_continuation_called);
    ASSERT_TRUE(is_then_continuation_called);

    ASSERT_TRUE(r.HasValue());
    ASSERT_EQ(r.Value(), 2);
  }
}

