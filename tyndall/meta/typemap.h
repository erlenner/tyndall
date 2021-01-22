#pragma once

/*
  typemap provides a way to map types to arbitrary data
*/

template<typename... Types>
class typemap
{};

template<typename Type, typename... Tail>
struct typemap<Type, Tail...> : public typemap<Tail...>
{
  //using type = Type;
  //Type data;

  //explicit constexpr typemap(typemap<Tail...> tl, Type data) noexcept
  //: typemap<Tail...>(tl)
  //, data(data)
  //{}
  explicit constexpr typemap() noexcept
  : typemap<Tail...>()
  {}

  //template<typename Entry>
  //constexpr typemap<Entry, Type, Tail...> join(Entry entry) const noexcept
  //{
  //  return typemap<Entry, Type, Tail...>(*this, entry);
  //}
  template<typename Entry>
  static constexpr typemap<Entry, Type, Tail...> join() noexcept
  {
    return typemap<Entry, Type, Tail...>();
  }

  template<int index>
  static constexpr std::enable_if_t<index == sizeof...(Tail),
  Type> get() noexcept
  {
    return Type();
  }

  template<int index>
  static constexpr std::enable_if_t<index < sizeof...(Tail),
  decltype(typemap<Tail...>::template get<index>())> get() noexcept
  {
    return typemap<Tail...>::template get<index>();
  }

  //constexpr std::optional<Data> search(bool (*cond)(const Data&)) const noexcept
  //{
  //  if (cond(data))
  //    return data;
  //  else
  //    return typemap<Data, Tail...>::search(cond);
  //}

  //template<typename Match, typename Exec>
  //constexpr int match_exec(Match match, Exec exec) const noexcept
  //{
  //  Type instance;
  //  if (match(instance, data))
  //  {
  //    exec(instance, data);
  //    return 0;
  //  }
  //  else
  //    return typemap<Data, Tail...>::match_exec(match, exec);
  //}

  //constexpr auto operator[](int index) const noexcept
  //{
  //  constexpr int match = sizeof...(Tail);
  //  if (index >= match)
  //    return *this;
  //  else
  //    return typemap<Data, Tail...>::operator[](index);
  //}

  //static constexpr int size() noexcept
  //{
  //  return 1 + sizeof...(Tail);
  //}

};

template<>
struct typemap<>
{
  explicit constexpr typemap() noexcept
  {}

  template<typename Entry>
  static constexpr typemap<Entry> join() noexcept
  {
    return typemap<Entry>();
  }

  template<int index>
  static constexpr int get() noexcept
  {
    return 0;
  }


  //template<typename Match, typename Exec>
  //constexpr int match_exec(Match match, Exec exec) const noexcept
  //{
  //  return -1;
  //}

  //constexpr std::optional<Data> search(bool (*cond)(const Data&)) const noexcept
  //{
  //  return {};
  //}


  //constexpr auto operator[](int index) const noexcept
  //{
  //  return *this;
  //}

  //static constexpr int size() noexcept
  //{
  //  return 0;
  //}

};
