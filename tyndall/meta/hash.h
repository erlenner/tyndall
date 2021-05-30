#pragma once

#include <stdint.h>
#include <stddef.h>

// FNV-1a 32bit hashing algorithm.
constexpr uint32_t hash_fnv1a_32(const char* s, size_t count)
{
  return count ? (((hash_fnv1a_32(s, count-1)) ^ s[count-1]) * 16777619u) : 2166136261u;
}

constexpr uint32_t operator ""_hash_fnv1a_32(const char* s, size_t size)
{
  return hash_fnv1a_32(s, size);
}
