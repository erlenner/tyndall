#pragma once
#include <type_traits>

/*
  iterate calls function with integral constants from 0 to specified index.
  Useful for iterating at compile time.
*/

template<int index, typename UnaryFunction>
requires(index >= 1)
constexpr void iterate(UnaryFunction f) noexcept
{
  constexpr int prev_index = index - 1;

  iterate<prev_index>(f);

  f(std::integral_constant<int, prev_index>());
}

template<int index, typename UnaryFunction>
requires(index == 0)
constexpr void iterate(UnaryFunction f) noexcept
{
}
