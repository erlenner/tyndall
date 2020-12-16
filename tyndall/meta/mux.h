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

template<typename A, typename B, int N>
constexpr std::enable_if_t<N >= 1, value_vec<mux_entry<A,B>,N>>
mux(value_vec<A,N> a, value_vec<B,N> b)
{
  return mux(--a, --b) + mux_entry<A,B>{ a.last(), b.last() };
}

template<typename A, typename B, int N>
constexpr std::enable_if_t<N == 0, value_vec<mux_entry<A,B>,N>>
mux(value_vec<A,N> a, value_vec<B,N> b)
{
  return value_vec<mux_entry<A,B>,0>{};
}
