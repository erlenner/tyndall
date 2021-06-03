#pragma once

#include <type_traits>
#include "strval.h"
#include "hash.h"
#include "macro.h"

template<typename Type>
using typeinfo_remove_cvref_t = typename std::remove_cv_t<typename std::remove_reference_t<Type>>;

template<typename Type>
struct typeinfo_raw
{
  static constexpr int32_t hash() noexcept
  {
    //printf("FUNC: %s\n\n", __PRETTY_FUNCTION__);
    return operator""_hash_fnv1a_32(__PRETTY_FUNCTION__, strlen(__PRETTY_FUNCTION__));
  }
};

template<typename Type>
using typeinfo = typeinfo_raw<typeinfo_remove_cvref_t<Type>>;

#define type_info_hash(T) typeinfo<T>::hash()
