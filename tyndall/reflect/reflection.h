#include <type_traits>
#include <utility>

template<typename... Args>
struct reflection
{};

namespace detail
{
  template<int index, typename Lhs, typename... Rhs, typename = std::enable_if_t<index == 0>>
  static constexpr const Lhs& reflect_get(const reflection<Lhs, Rhs...>& refl) noexcept
  {
    return refl.lhs;
  }
  
  template<int index, typename Lhs, typename... Rhs, typename = std::enable_if_t<0 < index>>
  static constexpr auto reflect_get(const reflection<Lhs, Rhs...>& refl) noexcept -> decltype(reflect_get<index-1>(static_cast<const reflection<Rhs...>&>(refl)))
  {
    return reflect_get<index-1>(static_cast<const reflection<Rhs...>&>(refl));
  }
}

template<typename Lhs, typename... Rhs>
struct reflection<Lhs, Rhs...> : public reflection<Rhs...>
{
  const Lhs& lhs;

  explicit constexpr reflection(Lhs&& lhs, Rhs&&... rhs) noexcept
  : reflection<Rhs...>{std::forward<Rhs>(rhs)...}
  , lhs(lhs)
  {}

  template<int index>
  constexpr auto get() noexcept
  {
    return detail::reflect_get<index>(*this);
  }

  constexpr int size() noexcept
  {
    return 1 + sizeof...(Rhs);
  }

protected:

};

template<>
struct reflection<>
{
public:
  explicit constexpr reflection() noexcept
  {}

  constexpr int size() noexcept
  {
    return 0;
  }
};
