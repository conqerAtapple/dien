/******************************************************************************
 *
 *  File:   future.hpp
 *  Author: Jojy G Varghese
 *
 *  Description: `Future` interface.
 *
 ******************************************************************************/

#pragma once

#include <chrono>

#include "promise.hpp"
#include "shared_data.hpp"

#include "future_inc.hpp"

namespace dien
{

typedef std::chrono::milliseconds MilliSeconds;
typedef std::chrono::seconds Seconds;

struct FailedFuture
{
  FailedFuture() : error(Error("failed"))
  {
  }

  FailedFuture(Error e) : error(std::move(e))
  {
  }

  Error error;
};

template <typename T>
class Future
{
 public:
  typedef T value_type;

  // not copyable
  Future(Future const &) = delete;
  Future &operator=(Future const &) = delete;

  // movable
  Future(Future &&) noexcept;
  Future &operator=(Future &&) noexcept;

  template <class U, typename = typename std::enable_if<
                         std::is_convertible<U, T>::value &&
                         !std::is_same<void, T>::value &&
                         !std::is_same<void, U>::value>::type>
  Future(Future<U> &&) noexcept;

  template <class U, typename = typename std::enable_if<
                         std::is_convertible<U, T>::value>::type>
  Future &operator=(Future<U> &&) noexcept;

  // Construct a Future from a value (perfect forwarding)
  template <class T2 = T,
            typename = typename std::enable_if<
                !is_future<typename std::decay<T2>::type>::value>::type>
  Future(T2 &&val);

  template <class T2 = T>
  Future(typename std::enable_if<std::is_same<void, T2>::value>::type * =
             nullptr);

  template <class V = T>
  Future(typename std::enable_if<std::is_same<void, V>::value>::type val);

  Future(FailedFuture);

  ~Future();

  template <class T2 = T>
  typename std::enable_if<!std::is_same<T2, void>::value, T>::type &Value()
  {
    return shared_->Get();
  }

  template <class T2 = T>
  typename std::enable_if<!std::is_same<T2, void>::value, const T>::type &
      Value() const
  {
    return shared_->Get();
  }

  bool IsReady() const;

  bool HasValue() const;

  bool HasError() const;

  void Detach();

  template <class T2 = T>
  typename std::enable_if<!std::is_same<T2, void>::value, T>::type Get()
  {
    assert(HasValue());
    return Value();
  }

#if 0
  // TODO (jojy): Implement
  Try<T>& getTry();

  Optional<Try<T>> Poll();


  T Get(Seconds dur);

  T Get(MilliSeconds dur);
#endif

  // TODO(jojy): private?
  template <class F>
  void SetCallback_(F &&func);

  template <typename F, typename R = callable_result<T, F>>
  typename R::Return Then(F &&func)
  {
    typedef typename R::Arg Arguments;
    return ThenImplementation<F, R>(std::forward<F>(func), Arguments());
  }

  template <typename R, typename Caller, typename... Args>
  Future<typename is_future<R>::Inner> Then(R (Caller::*func)(Args...),
                                            Caller *instance);

  template <typename F, typename R = on_error_callable_result<T, F>>
  typename R::Return OnError(F &&func)
  {
    typedef typename R::Arg Arguments;
    return OnErrorImplementation<F, R>(std::forward<F>(func), Arguments());
  }

 private:
  friend class Promise<T>;
  template <class>
  friend class Future;

  typedef SharedData<T> *SharedDataPtr;

  // shared core state object
  SharedDataPtr shared_;

  explicit Future(SharedDataPtr obj) : shared_(obj)
  {
  }

  // Variant: returns a value
  // e.g. f.Then([](Try<T> t){ return t.value(); });
  template <typename F, typename R, bool isTry, typename... Args>
  typename std::enable_if<!R::ReturnsFuture::value, typename R::Return>::type
      ThenImplementation(F &&func, arg_result<isTry, F, Args...>);

  // Variant: returns a Future
  // e.g. f.Then([](Try<T> t){ return makeFuture<T>(t); });
  template <typename F, typename R, bool isTry, typename... Args>
  typename std::enable_if<R::ReturnsFuture::value, typename R::Return>::type
      ThenImplementation(F &&func, arg_result<isTry, F, Args...>);

  template <typename F, typename R, typename... Args>
  typename std::enable_if<!R::ReturnsFuture::value, typename R::Return>::type
      OnErrorImplementation(F &&func, on_error_arg_result<F, Args...>);
};  // class Future

}  // namespace dien

#include "future_impl.hpp"
