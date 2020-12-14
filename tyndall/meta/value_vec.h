/*
  value_vec is a vector / array-like data structure with value semantics.
  It is mostly useful for compile time logic.
  All operations gives a new value, with a different type.
  Only stack operations are supported for now, ie. + and --.
*/

#include <type_traits>

template<typename T, int N=0, typename enable = void>
class value_vec
{};


template<typename T, int N>
class alignas(T) value_vec<T, N, typename std::enable_if<N == 0>::type>
{
public:

  explicit constexpr value_vec() noexcept
  {}

  constexpr value_vec<T, 1> operator+(T entry) const noexcept
  {
    return value_vec<T, 1>(*this, entry);
  }

  constexpr T operator[](int index) const noexcept
  {
    return T{};
  }

  constexpr const T* begin() const
  {
    return reinterpret_cast<const T*>(this);
  }

  static constexpr int size() noexcept
  {
    return 0;
  }

  template<typename UnaryFunction>
  static constexpr void iterate(UnaryFunction f) noexcept
  {
  }

  template<typename UnaryFunction>
  void for_each(UnaryFunction f) const noexcept
  {
  }
};


template<typename T, int N>
class value_vec<T, N, typename std::enable_if<N != 0>::type> : public value_vec<T, N-1>
{
  T entry;

public:

  explicit constexpr value_vec(value_vec<T, N-1> rhs, T entry) noexcept
  : value_vec<T, N-1>(rhs)
  , entry(entry)
  {}

  constexpr value_vec<T, N+1> operator+(T entry) const noexcept
  {
    return value_vec<T, N+1>(*this, entry);
  }

  constexpr value_vec<T, N-1> operator--() const noexcept
  {
    return value_vec<T, N-1>(*this);
  }

  constexpr T operator[](int index) const noexcept
  {
    if (index == N-1)
      return entry;
    else
      return value_vec<T, N-1>::operator[](index);
  }

  constexpr const T* begin() const noexcept
  {
    return value_vec<T, N-1>::begin();
  }

  constexpr const T* end() const noexcept
  {
    return begin() + N;
  }

  constexpr const T last() const noexcept
  {
    return entry;
  }

  static constexpr int size() noexcept
  {
    return N;
  }

  template<typename UnaryFunction>
  static constexpr void iterate(UnaryFunction f) noexcept
  {
    value_vec<T,N-1>::iterate(f);

    constexpr int index = size() - 1;
    f(std::integral_constant<int, index>());
  }

  template<typename UnaryFunction>
  void for_each(UnaryFunction f) const noexcept
  {
    value_vec<T,N-1>::for_each(f);
    f(last());
  }
};

//template<typename T, typename UnaryFunction>
//constexpr void value_vec_iterate(value_vec<T,0> v, UnaryFunction f) noexcept
//{
//}
//
//template<typename T, int N, typename UnaryFunction>
//constexpr void value_vec_iterate(value_vec<T,N> v, UnaryFunction f) noexcept
//{
//  value_vec_iterate(--v,f);
//  //f(v.last());
//  
//  constexpr int index = v.size() - 1;
//  f(std::integral_constant<int, index>());
//}