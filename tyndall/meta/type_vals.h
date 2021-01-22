#pragma once

/*
  type_vals is a container with entries that have different types
*/

template<typename... Types>
class type_vals
{};

template<typename Type, typename... Tail>
struct type_vals<Type, Tail...> : public type_vals<Tail...>
{
  Type data;

  explicit constexpr type_vals(type_vals<Tail...> tl, Type data) noexcept
  : type_vals<Tail...>(tl)
  , data(data)
  {}

  template<typename Entry>
  constexpr type_vals<Entry, Type, Tail...> join(Entry entry) const noexcept
  {
    return type_vals<Entry, Type, Tail...>(*this, entry);
  }

  template<int index>
  constexpr std::enable_if_t<index == sizeof...(Tail),
  const Type&> get() const noexcept
  {
    return data;
  }

  template<int index>
  constexpr std::enable_if_t<index < sizeof...(Tail),
  decltype(type_vals<Tail...>::template get_type<index>())> get() const noexcept
  {
    return type_vals<Tail...>::template get<index>();
  }

protected:
  template<int index>
  static constexpr std::enable_if_t<index == sizeof...(Tail),
  const Type&> get_type() noexcept
  {
    return Type();
  }

  template<int index>
  static constexpr std::enable_if_t<index < sizeof...(Tail),
  decltype(type_vals<Tail...>::template get_type<index>())> get_type() noexcept
  {
    return type_vals<Tail...>::template get_type<index>();
  }

};

template<>
struct type_vals<>
{
  explicit constexpr type_vals() noexcept
  {}

  template<typename Entry>
  constexpr type_vals<Entry> join(Entry entry) const noexcept
  {
    return type_vals<Entry>(*this, entry);
  }

protected:
  template<int index>
  static constexpr int get_type() noexcept
  {
    return 0;
  }
};
