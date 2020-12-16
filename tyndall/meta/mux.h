#pragma once
#include "value_vec.h"

template<typename A, typename B>
struct mux_entry : public A,B
{
  explicit constexpr mux_entry(A a, B b) noexcept
  : A(a), B(b)
  {}

  explicit constexpr mux_entry() noexcept
  {}
};

template<int N, typename A, typename B>
constexpr std::enable_if_t<N >= 1, value_vec<mux_entry<A,B>,N>>
mux(value_vec<A,N> a, value_vec<B,N> b)
{
  return mux(--a, --b) + mux_entry<A,B>{ a.last(), b.last() };
}

template<int N, typename A, typename B>
constexpr std::enable_if_t<N == 0, value_vec<mux_entry<A,B>,N>>
mux(value_vec<A,N> a, value_vec<B,N> b)
{
  return value_vec<mux_entry<A,B>,0>{};
}

template<typename V0, typename V1, typename... Tail>
constexpr auto mux(V0 v0, V1 v1, Tail... tail)
{
  return mux(mux(v0, v1), tail...);
}
