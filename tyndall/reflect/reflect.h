#include "reflection.h"

//template<typename T>
//constexpr auto reflect(T&& t) noexcept
//{
//  auto&& [a, b] = t;
//  return reflection<decltype(a), decltype(b)>{std::forward<decltype(a)>(a), std::forward<decltype(b)>(b)};
//}

template<typename T>
constexpr auto reflect(T&& t) noexcept
{
  auto&& [a, b, c] = t;
  return reflection<decltype(a), decltype(b), decltype(c)>{std::forward<decltype(a)>(a), std::forward<decltype(b)>(b), std::forward<decltype(c)>(c)};
}
