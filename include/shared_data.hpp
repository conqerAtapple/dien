/******************************************************************************
 *
 *  File:   shared_data.hpp
 *  Author: Jojy G Varghese
 *
 *  Description: Implementation of SharedData that is shared between
 *              `Future` and `Promise`.
 *
 ******************************************************************************/

#pragma once

#include <atomic>
#include <mutex>
#include <cassert>

#include "option.hpp"
#include "scoped_lock.hpp"
#include "try.hpp"

namespace dien
{

enum State : uint8_t {
  kStart,
  kOnlyResult,
  kOnlyCallback,
  kArmed,
  kDone
};

template <class T>
class Future;

template <class T>
class Promise;

template <typename T, bool OnStack = false>
class SharedData
{
 public:
  SharedData() : result_(), state_(kStart), attached_(2)
  {
  }

  explicit SharedData(Try<T> &&v)
      : result_(std::move(v)), state_(kOnlyResult), attached_(1)
  {
  }

  ~SharedData()
  {
    if (!OnStack) assert(attached_ == 0);
  }

  SharedData(const SharedData &) = delete;
  SharedData &operator=(const SharedData &) = delete;

  SharedData(const SharedData &&) = delete;
  SharedData &operator=(const SharedData &&) = delete;

  // unlocked version
  bool Ready_() const
  {
    switch (state_) {
      case(State::kOnlyResult) :
      case(State::kArmed) :
      case(State::kDone) :
        assert(!!result_);
        return true;

      default:
        return false;
    }
  }

  bool Ready() const
  {
    ScopedLock lock(lock_);

    return Ready_();
  }

  template <class Q = T>
  typename std::enable_if<!std::is_same<Q, void>::value, T>::type &Get()
  {
    ScopedLock lock(lock_);

    assert(Ready_());

    return result_->Value();
  }

  bool HasError() const
  {
    ScopedLock lock(lock_);

    return result_->HasError();
  }

  template <typename F>
  void SetCallback(F &&fn)
  {
    bool transition_armed = false;

    {
      ScopedLock lock(lock_);

      switch (state_) {
        case(State::kStart) :
          state_ = State::kOnlyCallback;
          callback_ = std::forward<F>(fn);
          break;

        case(State::kOnlyResult) :
          state_ = State::kArmed;
          callback_ = std::forward<F>(fn);
          transition_armed = true;
          break;

        case(State::kOnlyCallback) :
        case(State::kArmed) :
        case(State::kDone) :
          CHECK_NE(true, false) << "SetCallback called twice";
      }
    }

    if (transition_armed) {
      DoCallback();
    }
  }

  void SetResult(Try<T> &&result)
  {
    bool transition_armed = false;

    {
      ScopedLock lock(lock_);

      switch (state_) {
        case(State::kStart) :
          state_ = State::kOnlyResult;
          result_ = std::move(result);
          break;

        case(State::kOnlyCallback) :
          state_ = State::kArmed;
          result_ = std::move(result);
          transition_armed = true;
          break;

        case(State::kOnlyResult) :
        case(State::kArmed) :
        case(State::kDone) :
          CHECK_NE(true, false) << "SetCallback called twice";
      }
    }

    if (transition_armed) {
      DoCallback();
    }
  }

  void DetachFuture()
  {
    Activate();
    DetachOne();
  }

  void DetachPromise()
  {
    if (!result_) {
      result_ = Try<T>(Error("Broken Promise"));
    }

    DetachOne();
  }

  void Deactivate()
  {
    active_.store(false, std::memory_order_release);
  }

  void Activate()
  {
    active_.store(true, std::memory_order_release);
    DoCallback();
  }

  bool IsActive()
  {
    return active_.load(std::memory_order_acquire);
  }

  void DoCallback()
  {
    ScopedLock lock(lock_);

    switch (state_) {
      case(State::kArmed) :
        if (active_.load(std::memory_order_acquire)) {
          state_ = State::kDone;
          callback_(std::move(result_.Value()));
        }
        break;

      default:
        break;
    }
  }

  void DetachOne()
  {
    if (OnStack) return;

    auto a = attached_--;

    assert(a >= 1);

    if (a == 1) {
      delete this;
    }
  }

 private:
  template <class>
  friend class Future;
  template <class>
  friend class Promise;

  mutable std::atomic_flag lock_ = ATOMIC_FLAG_INIT;
  std::function<void(Try<T> &&)> callback_;
  Option<Try<T>> result_;
  State state_;
  std::atomic<bool> active_{true};
  std::atomic<unsigned int> attached_;
}; // class SharedData

}  // namespace dien
