/******************************************************************************
 *
 *  File:   try_impl.hpp
 *  Author: Jojy G Varghese
 *
 *  Description: Implementation of `Try` interface.
 *
 ******************************************************************************/

#pragma once

namespace dien
{

template <class T>
Try<T>::Try(Try<T>&& t) noexcept : contains_(t.contains_)
{
  if (contains_ == Try::kValue) {
    new (&value_) T(std::move(t.value_));
  } else if (contains_ == Try::kError) {
    new (&e_) Error(std::move(t.e_));
  }
}

template <class T>
Try<T>& Try<T>::operator=(Try<T>&& t) noexcept
{
  if (this == &t) {
    return *this;
  }

  this->~Try();
  contains_ = t.contains_;
  if (contains_ == Try::kValue) {
    new (&value_) T(std::move(t.value_));
  } else if (contains_ == Try::kError) {
    new (&e_) Error(std::move(t.e_));
  }

  return *this;
}

template <class T>
Try<T>::Try(const Try<T>& t)
{
  static_assert(std::is_copy_constructible<T>::value,
                "T must be copyable for Try<T> to be copyable");
  contains_ = t.contains_;
  if (contains_ == Try::kValue) {
    new (&value_) T(t.value_);
  } else if (contains_ == Try::kError) {
    new (&e_) Error(t.e_);
  }
}

template <class T>
Try<T>& Try<T>::operator=(const Try<T>& t)
{
  static_assert(std::is_copy_constructible<T>::value,
                "T must be copyable for Try<T> to be copyable");
  this->~Try();
  contains_ = t.contains_;
  if (contains_ == Try::kValue) {
    new (&value_) T(t.value_);
  } else if (contains_ == Try::kError) {
    new (&e_) Error(t.e_);
  }
  return *this;
}

template <class T>
Try<T>::~Try()
{
  if (contains_ == Try<T>::kValue) {
    value_.~T();
  }

  if (contains_ == Try<T>::kError) {
    e_.~Error();
  }
}

template <class T>
T& Try<T>::Value() &
{
  assert(contains_ == Try::kValue);
  return value_;
}

template <class T>
T&& Try<T>::Value() &&
{
  assert(contains_ == Try::kValue);
  return std::move(value_);
}

template <class T>
const T& Try<T>::Value() const &
{
  assert(contains_ == Try::kValue);
  return value_;
}

template <typename F>
typename std::enable_if<
    !std::is_same<typename std::result_of<F()>::type, void>::value,
    Try<typename std::result_of<F()>::type>>::type
    MakeTryWith(F&& f)
{
  typedef typename std::result_of<F()>::type ResultType;
  return Try<ResultType>(f());
}

template <typename F>
typename std::enable_if<
    std::is_same<typename std::result_of<F()>::type, void>::value,
    Try<void>>::type
    MakeTryWith(F&& f)
{
  f();
  return Try<void>();
}

}  // namespace dien
