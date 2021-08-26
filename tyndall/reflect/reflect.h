#include <type_traits>
#include <cstddef>
#include <tyndall/meta/macro.h>
#include "reflection.h"
#include "n_fields.h"


#ifndef REFLECT_MAX_FIELDS
#define REFLECT_MAX_FIELDS 27
#endif
static_assert(REFLECT_MAX_FIELDS < M_MAX_ARITHMETIC);

template<size_t I>
using size_t_ = std::integral_constant<size_t, I>;

#define REFLECT_BINDING(I, _) M_CAT(a, I)
#define REFLECT_DECLTYPE_BINDING(I, _) decltype(M_CAT(a, I))
#define REFLECT_FORWARD_DECLTYPE_BINDING(I, _) std::forward<decltype(M_CAT(a, I))>(M_CAT(a, I))

#define REFLECT_I(I, _) \
template<typename T, size_t_<I>> \
constexpr auto reflect_impl(T&& t) noexcept \
{ \
  auto&& [ M_RANGE_WITH_COMMA(REFLECT_BINDING, 0, I) ] = t; \
  return reflection<M_RANGE_WITH_COMMA(REFLECT_DECLTYPE_BINDING, 0, I)>{M_RANGE_WITH_COMMA(REFLECT_FORWARD_DECLTYPE_BINDING, 0,I)}; \
}
M_EVAL(M_RANGE(REFLECT_I, 1, REFLECT_MAX_FIELDS))

// The range above produces items like the following (for I=3):
//template<typename T, int_<3>>
//constexpr auto reflect_impl(T&& t) noexcept
//{
//  auto&& [a0, a1, a2] = t;
//  return reflection<decltype(a0), decltype(a1), decltype(a2)>{std::forward<decltype(a0)>(a0), std::forward<decltype(a1)>(a1), std::forward<decltype(a2)>(a2)};
//}

template<typename T>
constexpr auto reflect(T&& t) noexcept
{
  return reflect_impl<T, size_t_<n_fields<T>()>{}>(std::forward<T>(t));
}
