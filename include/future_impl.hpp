/******************************************************************************
 *
 *  File:   future_impl.hpp
 *  Author: Jojy G Varghese
 *
 ******************************************************************************/

#pragma once

#include <cassert>
#include <chrono>
#include <thread>

#include "option.hpp"

namespace dien
{
template <class T>
Future<T>::Future(Future<T>&& other) noexcept : shared_(other.shared_)
{
  other.shared_ = nullptr;
}

template <class T>
Future<T>& Future<T>::operator=(Future<T>&& other) noexcept
{
  std::swap(shared_, other.shared_);
  return *this;
}

template <class T>
template <class T2, typename>
Future<T>::Future(T2&& val)
    : shared_(new SharedData<T>(Try<T>(std::forward<T2>(val))))
{
}

template <class T>
template <typename T2>
Future<T>::Future(typename std::enable_if<std::is_same<void, T2>::value>::type*)
    : shared_(new SharedData<T>(Try<T>(T())))
{
}

template <class T>
template <typename V>
Future<T>::Future(
    typename std::enable_if<std::is_same<void, V>::value>::type val)
    : shared_(new SharedData<void>(Try<void>()))
{
}

template <class T>
Future<T>::Future(FailedFuture f)
    : shared_(new SharedData<T>())
{
  shared_->SetResult(Try<T>(f.error));
}

template <class T>
Future<T>::~Future()
{
  Detach();
}

template <class T>
void Future<T>::Detach()
{
  if (shared_) {
    shared_->DetachFuture();
    shared_ = nullptr;
  }
}

template <class T>
template <class F>
void Future<T>::SetCallback_(F&& func)
{
  assert(shared_);

  shared_->SetCallback(std::forward<F>(func));
}

template <class T>
bool Future<T>::HasValue() const
{
  return shared_->Ready();
}

template <class T>
bool Future<T>::HasError() const
{
  return shared_->HasError();
}

// Variant: returns a value
// e.g. f.then([](Try<T>&& t){ return t.value(); });
template <class T>
template <typename F, typename R, bool isTry, typename... Args>
typename std::enable_if<!R::ReturnsFuture::value, typename R::Return>::type
    Future<T>::ThenImplementation(F&& func, arg_result<isTry, F, Args...>)
{
  static_assert(sizeof...(Args) <= 1, "Then must take zero/one argument");
  typedef typename R::ReturnsFuture::Inner B;

  assert(shared_);

  std::shared_ptr<Promise<B>> p = std::make_shared<Promise<B>>();

  // grab the Future now before we lose our handle on the Promise
  auto f = p->GetFuture();

  std::function<void(Try<T> && )> fn =
      std::bind([p, func](Try<T>&& t) mutable {
                  if (!isTry && t.HasError()) {
                    p->SetError(std::move(t.GetError()));
                  } else {
                    p->SetWith([&]() {
                      return func(t.template Get<isTry, Args>()...);
                    });
                  }
                },
                std::placeholders::_1);

  SetCallback_(fn);

  return f;
}

// Variant: returns a Future
// e.g. f.then([](T&& t){ return makeFuture<T>(t); });
template <class T>
template <typename F, typename R, bool isTry, typename... Args>
typename std::enable_if<R::ReturnsFuture::value, typename R::Return>::type
    Future<T>::ThenImplementation(F&& func, arg_result<isTry, F, Args...>)
{
  static_assert(sizeof...(Args) <= 1, "Then must take zero/one argument");
  typedef typename R::ReturnsFuture::Inner B;

  assert(shared_);

  std::shared_ptr<Promise<B>> p = std::make_shared<Promise<B>>();

  // grab the Future
  auto f = p->GetFuture();

  SetCallback_(
      [ funcm = std::forward<F>(func), pm = p ](Try<T> && t) mutable {
                                                 if (!isTry && t.HasError()) {
                                                   pm->SetError(
                                                       std::move(t.GetError()));
                                                 } else {
                                                   auto f2 =
                                                       funcm(t.template Get<
                                                           isTry, Args>()...);
                                                   // that didn't throw, now we
                                                   // can steal p
                                                   f2.SetCallback_(
                                                       [p = pm](Try<B> && b) mutable {
                                                         p->SetTry(
                                                             std::move(b));
                                                       });
                                                 }
                                               });

  return f;
}

// Variant: returns a value
template <class T>
template <typename F, typename R, typename... Args>
typename std::enable_if<!R::ReturnsFuture::value, typename R::Return>::type
    Future<T>::OnErrorImplementation(F&& func, on_error_arg_result<F, Args...>)
{
  static_assert(sizeof...(Args) <= 1, "Then must take zero/one argument");
  typedef typename R::ReturnsFuture::Inner B;

  assert(shared_);

  std::shared_ptr<Promise<B>> p = std::make_shared<Promise<B>>();

  // grab the Future now before we lose our handle on the Promise
  auto f = p->GetFuture();

  std::function<void(Try<T> && )> fn =
      std::bind([p, func](Try<T>&& t) mutable {
                  if (t.HasError()) {
                    t.template WithError([&](Error&& e) {
                      p->SetWith([&] { return func(std::move(e)); });
                    });
                  }
                },
                std::placeholders::_1);

  SetCallback_(fn);

  return f;
}

}  // namespace dien
