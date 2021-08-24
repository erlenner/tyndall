#include "reflection.h"

template<typename T>
constexpr auto reflect(T&& t) noexcept
{
  auto&& [a, b] = t;
  return reflection<decltype(a), decltype(b)>{std::forward<decltype(a)>(a), std::forward<decltype(b)>(b)};
}
