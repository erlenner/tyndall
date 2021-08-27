#pragma once
#include <type_traits>
#include <utility>
#include <climits>
#include <typeinfo>
#include <cstdio>

// heavy borrowing from https://github.com/apolukhin/pfr_non_boost/blob/master/include/pfr/detail/fields_count.hpp

namespace nfields_detail
{
  template<size_t... I>
  using index_sequence = std::integer_sequence<size_t, I...>;

  template<size_t N>
  using make_index_sequence = std::make_integer_sequence<size_t, N>;

  template <typename T>
  constexpr T unsafe_declval() noexcept {
    constexpr typename std::remove_reference_t<T>* ptr = 0;
    return static_cast<T>(*ptr);
  }

  struct ubiq_lref_constructor {
    int ignore;
    template <typename T> constexpr operator T&() const && noexcept {
      return unsafe_declval<T&>();
    }

    template <typename T> constexpr operator T&() const & noexcept {
      return unsafe_declval<T&>();
    }
  };

  struct ubiq_rref_constructor {
    int ignore;
    template <typename T> constexpr operator T() const && noexcept {
      return unsafe_declval<T>();
    }
  };

  template<typename T, size_t... I, typename = typename std::enable_if_t<std::is_copy_constructible_v<T>>>
  constexpr auto constructible(index_sequence<I...>) noexcept
    -> typename std::add_pointer_t<decltype(T{ ubiq_lref_constructor{I}... })>;

  template<typename T, size_t... I, typename = typename std::enable_if_t<!std::is_copy_constructible_v<T>>>
  constexpr auto constructible(index_sequence<I...>) noexcept
    -> typename std::add_pointer_t<decltype(T{ ubiq_rref_constructor{I}... })>;

  template <class T, size_t N, typename = decltype(constructible<T>(make_index_sequence<N>()))>
  using constructible_t = size_t;


  // n_fields_impl is overloaded with long and int to avoid multiple definitions
  //template<typename T, size_t N>
  //constexpr auto n_fields_impl(long long) noexcept
  //  -> typename std::enable_if_t<std::is_array_v<T>, size_t>
  //{
  //  return sizeof(T) / sizeof(typename std::remove_all_extents_t<T>);
  //}

  // n_fields_impl is overloaded with long and int to avoid multiple definitions
  template<typename T, size_t N>
  constexpr auto n_fields_impl(long) noexcept -> constructible_t<T, N>
  {
    return N;
  }

  template<typename T, size_t N>
  constexpr size_t n_fields_impl(int) noexcept
  {
    return n_fields_impl<T, N-1>(0l);
  }
}

template<typename T>
constexpr size_t n_fields() noexcept
{
  constexpr size_t max_field_count = sizeof(T)*CHAR_BIT;

  using type = std::remove_cvref_t<T>;

  static_assert(std::is_aggregate<type>::value || std::is_scalar<type>::value, "T must be aggregate initializable");

  return nfields_detail::n_fields_impl<type, max_field_count>(0l);
}
