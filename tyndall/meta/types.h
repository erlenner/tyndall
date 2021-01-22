#pragma once

/*
  types is a list of types
*/

template<typename... Types>
class types
{};

template<typename Type, typename... Tail>
struct types<Type, Tail...> : public types<Tail...>
{
  explicit constexpr types() noexcept
  : types<Tail...>()
  {}

  template<typename Entry>
  static constexpr types<Entry, Type, Tail...> join() noexcept
  {
    return types<Entry, Type, Tail...>();
  }

  template<typename Entry>
  constexpr types<Entry, Type, Tail...> operator+(Entry&&) const noexcept
  {
    return join<Entry>();
  }

  template<int index>
  static constexpr std::enable_if_t<index == sizeof...(Tail),
  Type> get() noexcept
  {
    return Type();
  }

  template<int index>
  static constexpr std::enable_if_t<index < sizeof...(Tail),
  decltype(types<Tail...>::template get<index>())> get() noexcept
  {
    return types<Tail...>::template get<index>();
  }
};

template<>
struct types<>
{
  explicit constexpr types() noexcept
  {}

  template<typename Entry>
  static constexpr types<Entry> join() noexcept
  {
    return types<Entry>();
  }

  template<typename Entry>
  constexpr types<Entry> operator+(Entry&&) const noexcept
  {
    return join<Entry>();
  }

  template<int index>
  static constexpr int get() noexcept
  {
    return 0;
  }
};
