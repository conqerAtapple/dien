/******************************************************************************
 *
 *  File:   error_tests.cpp
 *  Author: Jojy G Varghese
 *
 *  Description: Test suite for `Error` class.
 *
 ******************************************************************************/

#include <glog/logging.h>

#include "gtest/gtest.h"

#include "error.hpp"

using namespace dien;

TEST(ErrorTests, NewError)
{
  Error e(3, "test error");

  const Error::ErrorStack& s = e;

  ASSERT_EQ(s.top().Message(), "test error");
}

TEST(ErrorTests, NestedError)
{
  Error e1(1, "error 1");
  Error e2(2, "error 2");

  Error stack1 = e2.Stack(e1);

  Error e3(3, "error 3");
  Error e4(4, "error 4");

  Error stack2 = e4.Stack(e3);

  Error stack_all = stack2.Stack(stack1);

  Error::ErrorStack& s = stack_all;

  int count = 4;
  while (!s.empty()) {
    auto i = s.top();
    ASSERT_EQ(count--, i.Code());
    s.pop();
  }
}

