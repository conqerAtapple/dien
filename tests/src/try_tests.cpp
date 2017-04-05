/******************************************************************************
 *
 *  File:   try_tests.cpp
 *  Author: Jojy G Varghese
 *
 *  Description: Test suite for `Try` class.
 *
 ******************************************************************************/

#include <glog/logging.h>

#include "gtest/gtest.h"

#include "try.hpp"

using namespace dien;

TEST(TryTests, Empty)
{
  Try<int> t;

  ASSERT_FALSE(t.HasValue());
}

TEST(TryTests, SetGet)
{
  Try<int> t(1024);

  ASSERT_TRUE(t.Value());

  ASSERT_EQ(t.Value(), 1024);
}

TEST(TryTests, Error)
{
  Try<int> t(Error(-1, "test error"));

  ASSERT_TRUE(t.HasError());

  const Error& error = t.GetError();

  const Error::ErrorStack& stack = error;
  ASSERT_EQ(stack.top().Code(), -1);
}

