/******************************************************************************
 *
 *  File:   error.hpp
 *  Author: Jojy G Varghese
 *
 ******************************************************************************/

#pragma once

#include <stdarg.h>

#include <memory>
#include <string>
#include <stack>

namespace dien
{

enum ErrorCodes {
  kFailed,
};

struct ErrorCode
{
  ErrorCode(int code, const std::string& msg) : error_code(code), message(msg)
  {
  }

  ErrorCode(const char* fmt, ...) : error_code(kFailed)
  {
    char buffer[256];
    va_list args;

    va_start(args, fmt);
    vsnprintf(buffer, 256, fmt, args);

    message = buffer;

    va_end(args);
  }

  ErrorCode(int code, const char* fmt, ...) : error_code(code)
  {
    char buffer[256];
    va_list args;

    va_start(args, fmt);
    vsnprintf(buffer, 256, fmt, args);

    message = buffer;

    va_end(args);
  }

  std::string Message()
  {
    return message;
  }

  const std::string& Message() const
  {
    return message;
  }

  int Code() const
  {
    return error_code;
  }

 protected:
  int error_code;
  std::string message;
};  // class ErrorCode

struct Error
{
  using ErrorStack = std::stack<ErrorCode>;

  ErrorStack error_stack;

  Error(int code, std::string&& msg)
  {
    ErrorStack new_stack;

    error_stack.swap(new_stack);
    error_stack.emplace(ErrorCode(code, std::forward<std::string>(msg)));
  }

  Error(ErrorCode&& error_code)
  {
    error_stack.emplace(std::move(error_code));
  }

  Error(const ErrorStack& stack) : error_stack(stack)
  {
  }

  Error(const ErrorCode& errorCode)
  {
    error_stack.emplace(ErrorCode(errorCode));
  }

  Error& Stack(const Error& error)
  {
    ErrorStack temp;
    while (!error_stack.empty()) {
      temp.emplace(error_stack.top());
      error_stack.pop();
    }

    error_stack = error.error_stack;
    while (!temp.empty()) {
      error_stack.emplace(temp.top());
      temp.pop();
    }

    return *this;
  }

  Error& Stack(const ErrorCode& errorCode)
  {
    error_stack.emplace(ErrorCode(errorCode));

    return *this;
  }

  const ErrorCode& Top() const &
  {
    return error_stack.top();
  }

  void Clear()
  {
    while (!error_stack.empty()) {
      error_stack.pop();
    }
  }

  &operator ErrorStack() &
  {
    return error_stack;
  }

  &operator const ErrorStack() const &
  {
    return error_stack;
  }
};  // class Error

}  // namespace dien
