#pragma once

// compile time string which ensures "string A"_strval and "string A"_strval have the same type,
// while "string A"_strval and "string B"_strval have different types

template<char... Args>
struct strval
{
  template<char... Rhs>
  constexpr auto operator+(strval<Rhs...> rhs) const noexcept
  {
    return strval<Args..., Rhs...>();
  }

  static constexpr const char* c_str() noexcept
  {
    return (const char[]){ Args..., '\0'};
  }
};

template<typename Char, Char... Args>
constexpr strval<Args...> operator""_strval() noexcept
{
  return {};
}

//#include <ostream>
//template<char... Args>
//std::ostream& operator<<(std::ostream& os, const strval<Args...>& rhs)
//{
//  return os << rhs.c_str();
//}
