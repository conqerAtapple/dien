/******************************************************************************
 *
 *  File:   dien-tests.cpp
 *  Author: Jojy G Varghese
 *
 *  Description: gtest driver for `dien` library.
 *
 ******************************************************************************/

#include <glog/logging.h>

#include <gtest/gtest.h>

int main(int argc, char** argv)
{
  FLAGS_alsologtostderr = 1;  // It will dump to console
  google::InitGoogleLogging("dient-tests");

  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
