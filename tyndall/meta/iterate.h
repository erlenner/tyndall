#pragma once
#include <type_traits>

/*
  iterate calls function with integral constants from 0 to specified index.
  Useful for iterating at compile time.
*/

template<int index, typename UnaryFunction>
constexpr std::enable_if_t<index >= 1, void>
iterate(UnaryFunction f) noexcept
{
  constexpr int prev_index = index - 1;

  iterate<prev_index>(f);

  f(std::integral_constant<int, prev_index>());
}

template<int index, typename UnaryFunction>
constexpr std::enable_if_t<index == 0, void>
iterate(UnaryFunction f) noexcept
{
}
