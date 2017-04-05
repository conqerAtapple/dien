/******************************************************************************
 *
 *  File:   option.hpp
 *  Author: Jojy G Varghese
 *
 *  Description: `Option` interface and implementaion.
 *
 ******************************************************************************/

#pragma once

namespace dien
{

template <typename T>
class Option
{
  typedef T value_type;

  static_assert(!std::is_reference<T>::value,
                "Option can not be used with reference types");
  static_assert(!std::is_abstract<T>::value,
                "Option can not be used with abstract types");

public:
  Option() {}

  Option(const Option& other)
  {
    if (other.HasValue()) {
      Construct(std::move(other.Value()));
    }
  }

  Option(const Option&& other)
  {
    if (other.HasValue()) {
      Construct(std::move(other.Value()));
      other.Clear();
    }
  }

  Option(const T& value) { Construct(value); }

  Option(T&& value) { Construct(std::move(value)); }

  void Assign(const Option& oth)
  {
    if (oth.HasValue()) {
      Assign(oth.Value());
    } else {
      Clear();
    }
  }

  void Assign(const Option&& oth)
  {
    if (oth.HasValue()) {
      Assign(oth.Value());
    } else {
      Clear();
    }
  }

  void Assign(const T& newVal)
  {
    if (HasValue()) {
      storage_.value = newVal;
    } else {
      Construct(newVal);
    }
  }

  void Assign(const T&& newVal)
  {
    if (HasValue()) {
      storage_.value = std::move(newVal);
    } else {
      Construct(std::move(newVal));
    }
  }

  template <typename Arg>
  Option& operator=(Arg&& arg)
  {
    Assign(std::forward<Arg>(arg));
    return *this;
  }

  Option& operator=(const Option& other)
  {
    Assign(other);
    return *this;
  }

  Option& operator=(const Option&& other)
  {
    Assign(std::move(other));
    return *this;
  }

  template <typename... Arg>
  void Emplace(Arg&&... args)
  {
    Clear();
    Construct(std::forward<Arg>(args)...);
  }

  void Clear() { storage_.Clear(); }

  const T& Value() const &
  {
    AssertValue();
    return storage_.value;
  }

  T& Value() &
  {
    AssertValue();
    return storage_.value;
  }

  T&& Value() &&
  {
    AssertValue();
    return storage_.value;
  }

  const T&& Value() const &&
  {
    AssertValue();
    return storage_.value;
  }

  bool HasValue() const { return storage_.hasValue; }

  operator bool() const { return HasValue(); }

  const T& operator*() const & { return Value(); }

  T& operator*() & { return Value(); }

  const T&& operator*() const && { return std::move(Value()); }

  T&& operator*() && { return std::move(Value()); }

  const T* operator->() const { return &Value(); }

  T* operator->() { return &Value(); }

  template <class U>
  T ValueOr(U&& dflt) const &
  {
    if (storage_.hasValue) {
      return storage_.value;
    }

    return std::forward<U>(dflt);
  }

  template <class U>
  T ValueOr(U&& dflt) &&
  {
    if (storage_.hasValue) {
      return std::move(storage_.value);
    }

    return std::forward<U>(dflt);
  }

private:
  void AssertValue() const { assert(storage_.hasValue); }

  template <class... Args>
  void Construct(Args&&... args)
  {
    const void* ptr = &storage_.value;
    // for supporting const types
    new (const_cast<void*>(ptr)) T(std::forward<Args>(args)...);
    storage_.hasValue = true;
  }

  struct StorageTriviallyDestructible
  {
    union
    {
      bool hasValue;
      struct
      {
        bool paddingForHasValue_[1];
        T value;
      };
    };

    StorageTriviallyDestructible() : hasValue{false} {}

    void Clear() { hasValue = false; }
  };

  struct StorageNonTriviallyDestructible
  {
    // See StorageTriviallyDestructible's union
    union
    {
      bool hasValue;
      struct
      {
        bool paddingForHasValue_[1];
        T value;
      };
    };

    StorageNonTriviallyDestructible() : hasValue{false} {}
    ~StorageNonTriviallyDestructible() { Clear(); }

    void Clear()
    {
      if (hasValue) {
        hasValue = false;
        value.~T();
      }
    }
  };

  using Storage = typename std::conditional<
      std::is_trivially_destructible<T>::value, StorageTriviallyDestructible,
      StorageNonTriviallyDestructible>::type;

  Storage storage_;
}; // class Option

}  // namespace dien
