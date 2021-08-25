#include <type_traits>
#include <utility>
#include <climits>
#include <typeinfo>
#include <cstdio>

// heavy borrowing from https://github.com/apolukhin/pfr_non_boost/blob/master/include/pfr/detail/fields_count.hpp

namespace detail
{
  template<int... I>
  using index_sequence = std::integer_sequence<int, I...>;

  template<int N>
  using make_index_sequence = std::make_integer_sequence<int, N>;

  template <class T>
  constexpr T unsafe_declval() noexcept {
      constexpr typename std::remove_reference_t<T>* ptr = 0;
      return static_cast<T>(*ptr);
  }

  struct ubiq_lref_constructor {
      int ignore;
      template <class T> constexpr operator T&() const && noexcept {
          return unsafe_declval<T&>();
      }
  
      template <class T> constexpr operator T&() const & noexcept {
          return unsafe_declval<T&>();
      }
  };

  struct ubiq_rref_constructor {
      int ignore;
      template <class T> constexpr operator T() const && noexcept {
          return unsafe_declval<T>();
      }
  };

  template<typename T, int... I, class = typename std::enable_if_t<std::is_copy_constructible_v<T>>>
  constexpr auto constructible(index_sequence<I...>) noexcept
    -> typename std::add_pointer_t<decltype(T{ ubiq_lref_constructor{I}... })>;

  template<typename T, int... I, class = typename std::enable_if_t<!std::is_copy_constructible_v<T>>>
  constexpr auto constructible(index_sequence<I...>) noexcept
    -> typename std::add_pointer_t<decltype(T{ ubiq_rref_constructor{I}... })>;

  //template<typename T, int... I>
  //constexpr auto constructible(index_sequence<I...>) noexcept
  //  -> decltype(T{I...});

  template <class T, int N, class = decltype( constructible<T>(make_index_sequence<N>()) ) >
  using constructible_t = int;

  //template<typename T, int N>
  //constexpr std::enable_if_t<std::is_array_v<T>
  //, int> n_fields_impl() noexcept
  //{
  //  return sizeof(T) / sizeof(std::remove_all_extents_t<T>);
  //}

  template<typename T, int N>
  constexpr auto n_fields_impl() noexcept -> constructible_t<T, N>
  {
    return N;
  }

  //template<typename T, int N>
  //constexpr auto n_fields_match() noexcept -> constructible_t<T, N-1>
  //{
  //  return N-1;
  //}

}

template<typename T>
constexpr int n_fields() noexcept
{
  //constexpr int max_field_count = sizeof(T)*CHAR_BIT;
  constexpr int max_field_count = 4;

  using type = std::remove_cvref_t<T>;

  return detail::n_fields_impl<type, max_field_count>();
}
