/******************************************************************************
 *
 *  File:   scoped_lock.hpp
 *  Author: Jojy G Varghese
 *
 *  Description: An RAII based lock for std::atomic_flag
 *
 ******************************************************************************/

#pragma once

#include <atomic>

namespace dien
{

struct ScopedLock
{
  ScopedLock(std::atomic_flag& flag) : lock_(flag)
  {
    while (lock_.test_and_set(std::memory_order_acquire)) {}
  }

  ~ScopedLock()
  {
    lock_.clear(std::memory_order_release);
  }

  std::atomic_flag& lock_;
};  // ScopedLock

}  // namespace dien
