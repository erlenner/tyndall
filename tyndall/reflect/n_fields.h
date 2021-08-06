#include <type_traits>
#include <climits>
#include <stddef.h>

//template<size_t size>
//using size_tc = std::integral_constant<size_t, size>;

template<typename T, size_t N>
constexpr std::enable_if_t<std::is_array_v<T>
, std::size_t> n_fields_impl() noexcept
{
  return sizeof(T) / sizeof(std::remove_all_extents_t<T>);
}

template<typename T>
constexpr size_t n_fields() noexcept
{
  constexpr size_t max_field_count = sizeof(T)*CHAR_BIT;

  using type = typename std::remove_cv<T>;

  return n_fields_impl<type, max_field_count>();
}
