/******************************************************************************
 *
 *  File:   future_inc.hpp
 *  Author: Jojy G Varghese
 *
 *  Description: `Future` implementation helpers
 *
 ******************************************************************************/

#pragma once

#include <functional>
#include <memory>
#include <type_traits>

#include "error.hpp"
#include "try.hpp"

namespace dien
{

template <typename T>
struct is_future : std::false_type {
  using Inner =
      typename std::conditional<std::is_same<T, void>::value, void, T>::type;
};

template <typename T>
struct is_future<Future<T>> : std::true_type {
  typedef T Inner;
};

template <typename T>
struct is_try : std::false_type
{};

template <typename T>
struct is_try<Try<T>> : std::true_type
{};

template <typename...>
struct ArgType;

template <typename Arg, typename... Args>
struct ArgType<Arg, Args...> {
  typedef Arg FirstArg;
};

template <>
struct ArgType<> {
  typedef void FirstArg;
};

template<typename F, typename... Args>
using ResultOf = decltype(std::declval<F>()(std::declval<Args>()...));

template <bool isTry, typename F, typename... Args>
struct arg_result
{
  using Result = ResultOf<F, Args...>;
  typedef typename ArgType<Args...>::FirstArg FirstArg;
};

template <bool isTry, typename F>
struct arg_result<isTry, F, void>
{
  using Result = ResultOf<F>;
};

template <typename F, typename... Args>
struct call_signature_match
{
    template<typename T,
             typename = ResultOf<T, Args...>>
    static constexpr std::true_type
    check(std::nullptr_t) { return std::true_type{}; };

    template<typename>
    static constexpr std::false_type
    check(...) { return std::false_type{}; };

    typedef decltype(check<F>(nullptr)) type;
    static constexpr bool value = type::value;
};

template <typename F>
struct call_signature_match<F, void>
{
    template<typename T,
             typename = ResultOf<T>>
    static constexpr std::true_type
    check(std::nullptr_t) { return std::true_type{}; };

    template<typename>
    static constexpr std::false_type
    check(...) { return std::false_type{}; };

    typedef decltype(check<F>(nullptr)) type;
    static constexpr bool value = type::value;
};

template <typename T, typename F>
struct callable_result
{
  typedef typename std::conditional<
    call_signature_match<F, void>::value,
    arg_result<false, F, void>,
    typename std::conditional<
      call_signature_match<F, T&&>::value,
      arg_result<false, F, T&&>,
      typename std::conditional<
        call_signature_match<F, T&>::value,
        arg_result<false, F, T&>,
        typename std::conditional<
          call_signature_match<F, Try<T>&&>::value,
          arg_result<true, F, Try<T>&&>,
          arg_result<true, F, Try<T>&>>::type>::type>::type>::type Arg;

  typedef typename Arg::FirstArg FirstArg;
  typedef is_future<typename Arg::Result> ReturnsFuture;
  typedef Future<typename ReturnsFuture::Inner> Return;
};

template <typename F>
struct callable_result<void, F>
{
  typedef arg_result<false, F> Arg;

  typedef is_future<typename Arg::Result> ReturnsFuture;
  typedef Future<typename ReturnsFuture::Inner> Return;
};

template <typename F, typename... Args>
struct on_error_arg_result
{
  using Result = ResultOf<F, Args...>;
  typedef typename ArgType<Args...>::FirstArg Arg;
};

template <typename F>
struct on_error_arg_result<F, void>
{
  using Result = ResultOf<F>;
  typedef void Arg;
};

template <typename T, typename F>
struct on_error_callable_result
{
  typedef typename std::conditional<
    call_signature_match<F>::value,
    on_error_arg_result<F>,
    typename std::conditional<
      call_signature_match<F, Error&&>::value,
      on_error_arg_result<F, Error&&>,
      on_error_arg_result<F, Error&>>::type>::type Arg;

  typedef is_future<typename Arg::Result> ReturnsFuture;
  typedef Future<typename ReturnsFuture::Inner> Return;
};

} // namespace dien
