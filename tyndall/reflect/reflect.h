#include <type_traits>
#include <tyndall/meta/macro.h>
#include "reflection.h"
#include "n_fields.h"

template<int I>
using int_ = std::integral_constant<int, I>;

#define REFLECT_BINDING(I, _) M_CAT(a, I)
#define REFLECT_DECLTYPE_BINDING(I, _) decltype(M_CAT(a, I))
#define REFLECT_FORWARD_DECLTYPE_BINDING(I, _) std::forward<decltype(M_CAT(a, I))>(M_CAT(a, I))

#define REFLECT_I(I, _) \
template<typename T, int_<I>> \
constexpr auto reflect_impl(T&& t) noexcept \
{ \
  auto&& [ M_RANGE_WITH_COMMA(REFLECT_BINDING, 0, I) ] = t; \
  return reflection<M_RANGE_WITH_COMMA(REFLECT_DECLTYPE_BINDING, 0, I)>{M_RANGE_WITH_COMMA(REFLECT_FORWARD_DECLTYPE_BINDING, 0,I)}; \
}
M_EVAL(M_RANGE(REFLECT_I, 1, 27))

template<typename T>
constexpr auto reflect(T&& t) noexcept
{
  return reflect_impl<T, int_<n_fields<T>()>{}>(std::forward<T>(t));
}
