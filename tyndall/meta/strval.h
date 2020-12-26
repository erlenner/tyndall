#pragma once

// compile time string which obeys "str"_strval == "str"_strval

template<char Head, char... Tail>
struct strval
{
  char head = Head;
  strval<Tail...> tail = strval<Tail...>();

  template<char... Rhs>
  constexpr auto operator+(strval<Rhs...> rhs) const noexcept
  {
    return strval<Head, Tail..., Rhs...>();
  }

  constexpr const char* c_str() const noexcept
  {
    return &head;
  }
};

template<char Last>
struct strval<Last>
{
  char last = Last;
  char null = '\0';

  template<char... Rhs>
  constexpr auto operator+(strval<Rhs...> rhs) const noexcept
  {
    return strval<Last, Rhs...>();
  }

  constexpr const char* c_str() const noexcept
  {
    return &last;
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
