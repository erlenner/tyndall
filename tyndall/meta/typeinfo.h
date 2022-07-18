#pragma once

#include <type_traits>
#include <string_view>
#include "strval.h"
#include "hash.h"
#include "macro.h"

#if __cplusplus > 201703L
// C++20
template<typename Type>
using typeinfo_remove_cvref_t = typename std::remove_cvref_t<Type>;
#else
template<typename Type>
using typeinfo_remove_cvref_t = typename std::remove_cv_t<typename std::remove_reference_t<Type>>;
#endif

template<typename Type>
struct typeinfo_raw
{
  static constexpr uint32_t hash() noexcept
  {
    //printf("FUNC: %s\n\n", __PRETTY_FUNCTION__);
    return hash_fnv1a_32(__PRETTY_FUNCTION__, std::string_view(__PRETTY_FUNCTION__).length());
  }
};

template<typename Type>
using typeinfo = typeinfo_raw<typeinfo_remove_cvref_t<Type>>;

#define typeinfo_hash(T) typeinfo<T>::hash()
