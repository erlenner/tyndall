#pragma once
#include <type_traits>
#include <utility>

#include "tyndall/meta/strval.h"
#include "get_format.h"

template<typename T>
constexpr auto reflect(T&& t) noexcept;

template<typename... Args>
struct reflection
{};

template<typename Lhs, typename... Rhs>
struct reflection<Lhs, Rhs...> : public reflection<Rhs...>
{
  const Lhs& lhs;

  explicit constexpr reflection(Lhs&& lhs, Rhs&&... rhs) noexcept
  : reflection<Rhs...>{std::forward<Rhs>(rhs)...}
  , lhs(lhs)
  {}

  template<size_t index>
  constexpr auto get() noexcept
  {
    return get_impl<index>(*this);
  }

  constexpr size_t size() noexcept
  {
    return 1 + sizeof...(Rhs);
  }

  constexpr auto get_format() noexcept
  {
    constexpr auto lhs_format = ::get_format<std::remove_cv_t<Lhs>>();

    if constexpr(lhs_format == ""_strval)
      return reflect(lhs).get_format() + reflection<Rhs...>::get_format();
    else
      return lhs_format + reflection<Rhs...>::get_format();
  }

protected:

  template<size_t index, typename = std::enable_if_t<index == 0>>
  static constexpr const Lhs& get_impl(const reflection<Lhs, Rhs...>& refl) noexcept
  {
    return refl.lhs;
  }

  template<size_t index, typename = std::enable_if_t<0 < index>>
  static constexpr auto get_impl(const reflection<Lhs, Rhs...>& refl) noexcept
  {
    return reflection<Rhs...>::template get_impl<index-1>(static_cast<const reflection<Rhs...>&>(refl));
  }
};

template<>
struct reflection<>
{
public:
  explicit constexpr reflection() noexcept
  {}

  template<size_t index>
  static constexpr size_t get_impl(const reflection<>& refl) noexcept
  {
    return -1;
  }

  constexpr size_t size() noexcept
  {
    return 0;
  }

  constexpr auto get_format() noexcept
  {
    return ""_strval;
  }
};

#include "reflect.h"
