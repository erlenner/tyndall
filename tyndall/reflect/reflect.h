#include <type_traits>
#include <utility>


template<typename... Args>
struct reflection
{};

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

template<typename Lhs, typename... Rhs>
struct reflection<Lhs, Rhs...> : public reflection<Rhs...>
{
  static_assert(std::is_nothrow_copy_assignable_v<Lhs>);
  static_assert(std::is_trivially_copy_assignable_v<Lhs>);

  const Lhs& lhs;

  explicit constexpr reflection(Lhs&& lhs, Rhs&&... rhs) noexcept
  : reflection<Rhs...>{std::forward<Rhs>(rhs)...}
  , lhs(lhs)
  {}

  template<int index>
  constexpr auto get() noexcept
  {
    return reflect_get<index>(*this);
  }

protected:

};

template<>
struct reflection<>
{
public:
  explicit constexpr reflection() noexcept
  {}
};

template<typename T>
constexpr auto reflect(T&& t) noexcept
{
  auto&& [a, b] = t;
  return reflection<decltype(a), decltype(b)>{std::forward<decltype(a)>(a), std::forward<decltype(b)>(b)};
}
