#pragma once

/*
  typemap provides a way to map types to arbitrary data
*/

template<typename Data, typename... Types>
class typemap
{};

template<typename Data, typename Type, typename... Tail>
struct typemap<Data, Type, Tail...> : public typemap<Data, Tail...>
{
  using type = Type;
  Data data;

  explicit constexpr typemap(typemap<Data, Tail...> tl, Data data) noexcept
  : typemap<Data, Tail...>(tl)
  , data(data)
  {}

  template<typename Entry>
  constexpr typemap<Data, Entry, Type, Tail...> append(Data entry_data) const noexcept
  {
    return typemap<Data, Entry, Type, Tail...>(*this, entry_data);
  }

  template<typename Exec>
  constexpr int match_exec(bool (*match)(const Data&), Exec exec) const noexcept
  {
    if (match(data))
    {
      Type instance;
      exec(instance, data);
      return 0;
    }
    else
      return typemap<Data, Tail...>::match_exec(match, exec);
  }

  //constexpr std::optional<Data> search(bool (*cond)(const Data&)) const noexcept
  //{
  //  if (cond(data))
  //    return data;
  //  else
  //    return typemap<Data, Tail...>::search(cond);
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

template<typename Data>
struct typemap<Data>
{
  explicit constexpr typemap() noexcept
  {}

  template<typename Entry>
  constexpr typemap<Data, Entry> append(Data entry_data) const noexcept
  {
    return typemap<Data, Entry>(*this, entry_data);
  }

  template<typename Exec>
  constexpr int match_exec(bool (*match)(const Data&), Exec exec) const noexcept
  {
    return -1;
  }

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
