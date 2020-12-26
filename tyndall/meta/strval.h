#pragma once

// compile time string which obeys "str"_strval == "str"_strval

template<char... Args>
struct strval
{
  template<char... Rhs>
  constexpr auto operator+(strval<Rhs...> rhs) const noexcept
  {
    return strval<Args..., Rhs...>();
  }

  constexpr const char* c_str() const noexcept
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
