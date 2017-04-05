/******************************************************************************
 *
 *  File:   option_tests.cpp
 *  Author: Jojy G Varghese
 *
 *  Description: Test suite for `Option` class.
 *
 ******************************************************************************/

#include <glog/logging.h>

#include "gtest/gtest.h"

#include "option.hpp"
#include "try.hpp"

using namespace dien;

TEST(OptionTests, NewIntegerValue)
{
  Option<int> o(5);
  ASSERT_EQ(o.Value(), 5);
}

TEST(OptionTests, NonTrivialDestructableObjectValue)
{
  static int v = 1;
  struct test
  {
    test() : pm(1)
    {
    }

    int Value()
    {
      return pm;
    }

   private:
    int pm = {0};
  };

  test t = test();
  Option<test> o(t);
  int pm = o.Value().Value();

  ASSERT_EQ(pm, 1);

  o.Clear();
  ASSERT_EQ(v, 1);
}

TEST(OptionTests, TrivialDestructableObjectValue)
{
  static int v = 1;
  struct test
  {
    test() : pm(1)
    {
    }

    ~test()
    {
      v = 0;
    }

    int Value()
    {
      return pm;
    }

   private:
    int pm = {0};
  };

  test t = test();
  Option<test> o(t);

  auto pm = o.Value().Value();

  ASSERT_EQ(pm, 1);

  o.Clear();
  ASSERT_EQ(v, 0);
}

TEST(OptionTests, UnsetHasValueFalse)
{
  Option<int> o;
  ASSERT_FALSE(o.HasValue());
}

TEST(OptionTests, Assignment)
{
  Option<int> o1(2);
  Option<int> o2(3);

  o2 = o1;

  ASSERT_EQ(o2.Value(), 2);
}

TEST(OpionTests, TryError)
{
  Option<Try<int>> o;
  o = Try<int>(Error("test error"));

  Try<int> v = o.Value();
  ASSERT_TRUE(v.HasError());

  ASSERT_EQ(v.GetError().Top().Message(), "test error");
}
