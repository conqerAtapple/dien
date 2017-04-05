/******************************************************************************
 *
 *  File:   promise.hpp
 *  Author: Jojy G Varghese
 *
 *  Description: `Promise` interface.
 *
 ******************************************************************************/

#pragma once

#include <functional>

#include "error.hpp"
#include "try.hpp"

namespace dien
{

template <class T>
class Future;

template <typename T>
class Promise
{
 public:
  Promise();
  ~Promise();

  Promise(const Promise&) = delete;
  Promise& operator=(const Promise&) = delete;

  Promise(Promise&&) noexcept;
  Promise& operator=(Promise&&) noexcept;

  // Can only be called once.
  Future<T> GetFuture();

  template <class M>
  void SetValue(M&& value);

  template <class F>
  void SetWith(F&& func);

  void SetError(Error error);

  bool IsFulfilled();

  void Detach();

 private:
  typedef typename Future<T>::SharedDataPtr SharedDataPtr;
  template <class>
  friend class Future;

  bool retrieved_;
  SharedDataPtr shared_;

  void SetTry(Try<T>&& t);
};  // class Promise

}  // namespace dien

#include "promise_impl.hpp"

