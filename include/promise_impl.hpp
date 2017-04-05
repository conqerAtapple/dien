/******************************************************************************
 *
 *  File:   promise_impl.hpp
 *  Author: Jojy G Varghese
 *
 *  Description: `Promise` implementation.
 *
 ******************************************************************************/

#pragma once

#include "shared_data.hpp"

namespace dien
{

template <typename T>
Promise<T>::Promise()
    : retrieved_(false), shared_(new SharedData<T>())
{
}

template <class T>
Promise<T>::Promise(Promise<T>&& other) noexcept : retrieved_(other.retrieved_),
                                                   shared_(other.shared_)
{
  other.shared_ = nullptr;
  other.retrieved_ = false;
}

template <class T>
Promise<T>& Promise<T>::operator=(Promise<T>&& other) noexcept
{
  std::swap(shared_, other.shared_);
  std::swap(retrieved_, other.retrieved_);
  return *this;
}

template <class T>
Promise<T>::~Promise()
{
  if (shared_) {
    if (!retrieved_) {
      shared_->DetachFuture();
    }

    shared_->DetachPromise();
    shared_ = nullptr;
  }
}

template <class T>
Future<T> Promise<T>::GetFuture()
{
  assert(shared_);
  assert(!retrieved_);

  retrieved_ = true;
  return Future<T>(shared_);
}

template <class T>
template <class M>
void Promise<T>::SetValue(M&& value)
{
  assert(shared_);

  assert(!shared_->Ready());

  shared_->SetResult(std::move(Try<T>(std::forward<M>(value))));
}

template <class T>
void Promise<T>::SetTry(Try<T>&& t)
{
  assert(shared_);

  assert(!shared_->Ready());

  shared_->SetResult(std::move(t));
}

template <typename F>
typename std::enable_if<
    std::is_same<typename std::result_of<F()>::type, void>::value,
    Try<void> >::type
    MakeTry_(F&& f)
{
  f();
  return Try<void>();
}

template <typename F>
typename std::enable_if<
    !std::is_same<typename std::result_of<F>::type, void>::type,
    Try<typename std::result_of<F()>::type> >::type
    MakeTry_(F&& f)
{
  typedef typename std::result_of<F>::type ResultType;

  return Try<ResultType>(f());
}

template <class T>
template <typename F>
void Promise<T>::SetWith(F&& func)
{
  assert(shared_);

  assert(!shared_->Ready());

  shared_->SetResult(std::move(MakeTryWith(std::forward<F>(func))));
}

template <class T>
void Promise<T>::SetError(Error e)
{
  assert(shared_);

  assert(!shared_->Ready());

  shared_->SetResult(Try<T>(std::move(e)));
}

template <class T>
bool Promise<T>::IsFulfilled()
{
  return (shared_ && shared_->HasResult());
}

}  // namespace dien
