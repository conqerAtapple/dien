/******************************************************************************
 *
 *  File:   try.hpp
 *  Author: Jojy G Varghese
 *
 *  Description: `Try` interface.
 *
 ******************************************************************************/

#pragma once

#include <type_traits>
#include <exception>
#include <algorithm>

#include "error.hpp"

namespace dien
{

template <class T>
class Try
{
  static_assert(!std::is_reference<T>::value,
                "Try may not be used with reference types");

public:
  enum Contains : uint8_t {
    kValue,
    kError,
    kNothing,
  };

  typedef T element_type;

  Try() : contains_(kNothing) {}

  Try(const T& v) : contains_(kValue), value_(v) {}

  explicit Try(T&& v) : contains_(kValue), value_(std::move(v)) {}

  explicit Try(Error e) : contains_(kError), e_(std::move(e)) {}

  Try(Try<T>&& t) noexcept;
  Try& operator=(Try<T>&& t) noexcept;

  Try(const Try& t);
  Try& operator=(const Try& t);

  ~Try();

  T& Value() & ;
  T&& Value() && ;
  const T& Value() const&;

  const T& operator*() const { return Value(); }
  T& operator*() { return Value(); }

  const T* operator->() const { return &Value(); }
  T* operator->() { return &Value(); }

  bool HasValue() const { return contains_ == kValue; }
  bool HasError() const { return contains_ == kError; }

  Error& GetError()
  {
    assert(contains_ == kError);
    return e_;
  }

  const Error& GetError() const
  {
    assert(contains_ == kError);
    return e_;
  }

  template <bool isTry, typename R>
  typename std::enable_if<isTry, R>::type Get()
  {
    return std::forward<R>(*this);
  }

  template <bool isTry, typename R>
  typename std::enable_if<!isTry, R>::type Get()
  {
    return std::forward<R>(Value());
  }

  template <class F>
  bool WithError(F func)
  {
    if (!HasError()) {
      return false;
    }

    func(std::move(e_));
    return true;
  }

private:
  Contains contains_;
  union
  {
    T value_;
    Error e_;
  };
};

template <>
class Try<void>
{
public:
  /*
   * The value type for the Try
   */
  typedef void element_type;

  // Construct a Try holding a successful and void result
  Try() : hasValue_(true) {}

  explicit Try(Error e) : hasValue_(false), e_(std::move(e)) {}

  // Copy assigner
  Try& operator=(const Try<void>& t)
  {
    if (&t != this) {
      this->~Try();

      if (t.HasError()) {
        new (&e_) Error(t.e_);
      } else {
        hasValue_ = true;
      }
    }

    return *this;
  }

  // Copy constructor
  Try(const Try<void>& t)
  {
    if (t.HasError()) {
      new (&e_) Error(t.e_);
    } else {
      hasValue_ = true;
    }
  }

  Try(Try<void>&& t) noexcept
  {
    if (t.HasError()) {
      new (&e_) Error(std::move(t.e_));
    } else {
      hasValue_ = true;
    }
  }

  Try& operator=(Try<void>&& t) noexcept
  {
    if (this == &t) {
      return *this;
    }

    this->~Try();

    if (t.HasError()) {
      new (&e_) Error(std::move(t.e_));
    }

    return *this;
  }

  ~Try()
  {
    if (HasError()) {
      e_.Clear();
    }
  }

  // If the Try contains an exception, throws it
  void Value() const { assert(hasValue_); }

  void operator*() const { return Value(); }

  bool HasValue() const { return hasValue_; }

  bool HasError() const { return !hasValue_; }

  Error& GetError()
  {
    assert(HasError());
    return e_;
  }

  const Error& GetError() const
  {
    assert(HasError());
    return e_;
  }

  /*
   * If the Try contains an error  execute func(Ex)
   *
   * @param func a function that takes a single parameter of type const Error&
   *
   * @returns True if the Try held an Error and func was executed, false
   *otherwise
   */
  template <class F>
  bool WithError(F func) const
  {
    if (!HasError()) {
      return false;
    }

    func(std::move(e_));
    return true;
  }

  template <bool, typename R>
  R Get()
  {
    return std::forward<R>(*this);
  }

private:
  bool hasValue_;
  union
  {
    Error e_;
  };
}; // class Try

/*
 * @param f a function to execute and capture the result of (value or exception)
 *
 * @returns Try holding the result of f
 */
template <typename F>
typename std::enable_if<
    !std::is_same<typename std::result_of<F()>::type, void>::value,
    Try<typename std::result_of<F()>::type>>::type
makeTryWith(F&& f);

/*
 * Specialization of makeTryWith for void return
 *
 * @param f a function to execute and capture the result of
 *
 * @returns Try<void> holding the result of f
 */
template <typename F>
typename std::enable_if<
    std::is_same<typename std::result_of<F()>::type, void>::value,
    Try<void>>::type
makeTryWith(F&& f);

}  // namespace dien

#include "try_impl.hpp"
