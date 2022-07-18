#pragma once

// compile time string which ensures "string A"_strval and "string A"_strval have the same type,
// while "string A"_strval and "string B"_strval have different types

#include <type_traits>

template<char... Chars>
struct strval
{
};

template<char Lhs, char... Rhs>
struct strval<Lhs, Rhs...>
{
  template<char... Args>
  constexpr auto operator+(strval<Args...> rhs) const noexcept
  {
    return strval<Lhs, Rhs..., Args...>();
  }

  static constexpr const char* c_str() noexcept
  {
    return (const char[]){ Lhs, Rhs..., '\0' };
  }

  static constexpr int length() noexcept
  {
    return 1 + sizeof... (Rhs);
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

  template<int index>
  static constexpr std::enable_if_t<index == 0,
  char> get() noexcept
  {
    return Lhs;
  }

  template<int index>
  static constexpr std::enable_if_t<0 < index,
  char> get() noexcept
  {
    return strval<Rhs...>::template get<index - 1>();
  }

  // set is volatile because it only matters in reinterpret_cast situations
  void set() volatile
  {
    const char tmp[] = { Lhs, Rhs... };
    for (int i=0; i<length(); ++i)
      data[i] = tmp[i];
  }

  char data[length()] = { Lhs, Rhs... }; // actual storage

  constexpr static inline bool is_strval(){ return true; }
};

template<>
struct strval<>
{
  char data[0]; // enforce zero size

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

  void set() volatile
  {
  }

  constexpr static inline bool is_strval(){ return true; }
};

template<char... Lhs, char... Rhs>
constexpr auto operator==(strval<Lhs...> lhs, strval<Rhs...> rhs) noexcept
{
  return std::is_same<strval<Lhs...>, strval<Rhs...>>();
}

template<char... Lhs, char... Rhs>
constexpr auto operator!=(strval<Lhs...> lhs, strval<Rhs...> rhs) noexcept
{
  return !(operator==(lhs, rhs));
}

template<typename Char, Char... Args>
constexpr strval<Args...> operator""_strval() noexcept
{
  return {};
}

template<unsigned remainder, unsigned... digits>
struct to_strval : to_strval<remainder / 10, remainder % 10, digits...> {};

template<unsigned... digits>
struct to_strval<0, digits...> : strval<('0' + digits)...>{};

#include <tyndall/meta/macro.h>
#define create_strval(str) M_CAT(str, _strval)
#define strval_t(str) decltype(M_CAT(str, _strval))

//#include <ostream>
//template<char... Args>
//std::ostream& operator<<(std::ostream& os, const strval<Args...>& rhs)
//{
//  return os << rhs.c_str();
//}
