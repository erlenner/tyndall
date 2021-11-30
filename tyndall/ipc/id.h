#pragma once
#include <string>
#include <tyndall/meta/strval.h>
#include <tyndall/meta/hash.h>

#ifndef IPC_SHMEM_PREFIX
//                              echo ipc | sha1sum  # more unique shared memory names reduce the chance of name collisions
#define IPC_SHMEM_PREFIX "ipc" "1ef42bc4e0bbfeb0ac34bc3642732768cf6f77b7"
#endif

// Shared memory file names need to be unique, and can't have folders.
// We take names that look like absolute paths, like "/my-process/my_topic".
// Then we prepend a constant unique prefix (IPC_SHMEM_PREFIX),
// we remove leading slashes,
// we prepend another prefix, which is the hash of the id without slashes,
// and lastly we replace remaining slashes with underscores.

// Note that "my-topic" gets the same id as "/my-topic" and "//my-topic" etc.

// Compile time id generation:
// Meant to take strval as template parameter.
template<typename STRING>
using id_remove_leading_slashes = decltype(STRING::template remove_leading<'/'>());

template<typename STRING>
using id_hash = decltype(to_strval<hash_fnv1a_32<STRING>()>{});

template<typename STRING>
using id_replace_slashes_with_underscores = decltype(STRING::template replace<'/', '_'>());

template<typename ID>
using id_prepare =
  decltype(
    create_strval(IPC_SHMEM_PREFIX)
    + "_"_strval
    + id_replace_slashes_with_underscores<id_remove_leading_slashes<ID>>{} // switch slashes with underscores
    + "_"_strval
    + id_hash<id_remove_leading_slashes<ID>>{} // hash id to prevent name clash between f.ex. /my/topic and my_topic
  );


// runtime id generation
template<typename STORAGE>
inline std::string id_rtid_prepare(const char* id)
{
  // remove leading slashes
  while (*id == '/')
    ++id;

  std::string prepared_id = IPC_SHMEM_PREFIX "_" + std::string{id} + "_" + std::to_string(hash_fnv1a_32(id, strlen(id)));

  // replace slash with underscore
  for (char& c : prepared_id)
    if (c == '/')
      c = '_';

  return prepared_id;
}
