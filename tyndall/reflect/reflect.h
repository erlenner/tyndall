#include <type_traits>
#include "reflection.h"
#include "n_fields.h"

//template<typename T>
//constexpr auto reflect(T&& t) noexcept
//{
//  auto&& [a, b] = t;
//  return reflection<decltype(a), decltype(b)>{std::forward<decltype(a)>(a), std::forward<decltype(b)>(b)};
//}

template<int I>
using int_ = std::integral_constant<int, I>;

template<typename T, int_<3>>
constexpr auto reflect_impl(T&& t) noexcept
{
  auto&& [a0, a1, a2] = t;
  return reflection<decltype(a0), decltype(a1), decltype(a2)>{std::forward<decltype(a0)>(a0), std::forward<decltype(a1)>(a1), std::forward<decltype(a2)>(a2)};
}

template<typename T, int_<4>>
constexpr auto reflect_impl(T&& t) noexcept
{
  auto&& [a0, a1, a2, a3] = t;
  return reflection<decltype(a0), decltype(a1), decltype(a2), decltype(a3)>{std::forward<decltype(a0)>(a0), std::forward<decltype(a1)>(a1), std::forward<decltype(a2)>(a2), std::forward<decltype(a2)>(a2)};
}

template<typename T>
constexpr auto reflect(T&& t) noexcept
{
  return reflect_impl<T, int_<n_fields<T>()>{}>(std::forward<T>(t));
}
