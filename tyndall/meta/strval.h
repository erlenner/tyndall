#pragma once

// compile time string which ensures "string A"_strval and "string A"_strval have the same type,
// while "string A"_strval and "string B"_strval have different types

template<char... Chars>
struct strval
{
};

template<char Lhs, char... Rhs>
struct strval<Lhs, Rhs...>
{
  template<char... Args>
  constexpr auto operator+(strval<Args...> args) const noexcept
  {
    return strval<Lhs, Rhs..., Args...>();
  }

  static constexpr const char* c_str() noexcept
  {
    return (const char[]){ Lhs, Rhs..., '\0' };
  }

  static constexpr int length() noexcept
  {
    return sizeof... (Rhs);
  }

  static constexpr int occurrences(char c) noexcept
  {
    return ((Lhs == c) ? 1 : 0) + strval<Rhs...>::occurrences(c);
  }

  template<char pattern, char replacement>
  static constexpr auto replace() noexcept
  {
    return strval<((Lhs == pattern) ? replacement : Lhs)>() + strval<Rhs...>::template replace<pattern, replacement>();
  }

  template<char pattern>
  static constexpr auto remove_leading() noexcept
  {
    if constexpr (Lhs == pattern)
      return strval<Rhs...>::template remove_leading<pattern>();
    else
      return strval<Lhs, Rhs...>{};
  }
};

template<>
struct strval<>
{
  template<char... Args>
  constexpr auto operator+(strval<Args...> args) const noexcept
  {
    return strval<Args...>();
  }

  static constexpr const char* c_str() noexcept
  {
    return (const char[]){ '\0' };
  }

  static constexpr int length() noexcept
  {
    return 0;
  }

  static constexpr int occurrences(char c) noexcept
  {
    return 0;
  }

  template<char pattern, char replacement>
  static constexpr auto replace() noexcept
  {
    return strval<>();
  }

  template<char pattern>
  static constexpr auto remove_leading() noexcept
  {
    return strval<>{};
  }
};

template<typename Char, Char... Args>
constexpr strval<Args...> operator""_strval() noexcept
{
  return {};
}

#include <tyndall/meta/macro.h>
#define create_strval(str) M_CAT(str, _strval)
#define strval_t(str) typeof(M_CAT(str, _strval))

//#include <ostream>
//template<char... Args>
//std::ostream& operator<<(std::ostream& os, const strval<Args...>& rhs)
//{
//  return os << rhs.c_str();
//}
