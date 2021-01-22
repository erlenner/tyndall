#pragma once

/*
  typevals is a container with entries that have different types
*/

template<typename... Types>
class typevals
{};

template<typename Type, typename... Tail>
struct typevals<Type, Tail...> : public typevals<Tail...>
{
  Type data;

  explicit constexpr typevals(typevals<Tail...> tl, Type data) noexcept
  : typevals<Tail...>(tl)
  , data(data)
  {}

  template<typename Entry>
  constexpr typevals<Entry, Type, Tail...> join(Entry entry) const noexcept
  {
    return typevals<Entry, Type, Tail...>(*this, entry);
  }

  template<int index>
  constexpr std::enable_if_t<index == sizeof...(Tail),
  const Type&> get() const noexcept
  {
    return data;
  }

  template<int index>
  constexpr std::enable_if_t<index < sizeof...(Tail),
  decltype(typevals<Tail...>::template get_type<index>())> get() const noexcept
  {
    return typevals<Tail...>::template get<index>();
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
  decltype(typevals<Tail...>::template get_type<index>())> get_type() noexcept
  {
    return typevals<Tail...>::template get_type<index>();
  }
};

template<>
struct typevals<>
{
  explicit constexpr typevals() noexcept
  {}

  template<typename Entry>
  constexpr typevals<Entry> join(Entry entry) const noexcept
  {
    return typevals<Entry>(*this, entry);
  }

protected:
  template<int index>
  static constexpr int get_type() noexcept
  {
    return 0;
  }
};
