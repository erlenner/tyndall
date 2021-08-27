#pragma once
#include <tyndall/meta/strval.h>
#include <string>
#include <cstddef>

// get format returns the typeid character for basic types and empty otherwise

template<typename T>
constexpr auto get_format() noexcept
{
  return ""_strval;
}

template<>
constexpr auto get_format<bool>() noexcept
{
  return "b"_strval;
}

template<>
constexpr auto get_format<float>() noexcept
{
  return "f"_strval;
}

template<>
constexpr auto get_format<double>() noexcept
{
  return "d"_strval;
}

template<>
constexpr auto get_format<int>() noexcept
{
  return "i"_strval;
}

template<>
constexpr auto get_format<unsigned int>() noexcept
{
  return "j"_strval;
}

template<>
constexpr auto get_format<char>() noexcept
{
  return "c"_strval;
}

template<>
constexpr auto get_format<unsigned char>() noexcept
{
  return "h"_strval;
}

template<>
constexpr auto get_format<long>() noexcept
{
  return "l"_strval;
}

template<>
constexpr auto get_format<unsigned long>() noexcept
{
  return "m"_strval;
}

template<>
constexpr auto get_format<long long>() noexcept
{
  return "x"_strval;
}

template<>
constexpr auto get_format<unsigned long long>() noexcept
{
  return "y"_strval;
}

template<>
constexpr auto get_format<std::string>() noexcept
{
  return "S"_strval;
}

// TODO:
//template<>
//constexpr auto get_format<const char*>() noexcept
//{
//  return "s"_strval;
//}
