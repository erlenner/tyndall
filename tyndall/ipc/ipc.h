#pragma once
#include "shmem.h"
#include <tyndall/meta/strval.h>

#ifndef IPC_SHMEM_PREFIX
//                              echo ipc | sha1sum  # more unique shared memory names reduce the chance of name collisions
#define IPC_SHMEM_PREFIX "ipc" "1ef42bc4e0bbfeb0ac34bc3642732768cf6f77b7"
#endif

#ifdef __cplusplus

#include "seq_lock.h"
#include <tyndall/meta/strval.h>


// Standard, lazy ipc methods, templated on storage type and id,
// so that the transport will be initiated only on calls with new combinations of storage type and id.
#define ipc_write(entry, id) ipc_lazy_write(entry, id ## _strval)
#define ipc_read(entry, id) ipc_lazy_read(entry, id ## _strval)

// remove leading slashes and replace the rest of the slashes with underscores
template<typename ID>
using ipc_convert_id = typeof(create_strval(IPC_SHMEM_PREFIX) + ID::template remove_leading<'/'>().template replace<'/', '_'>());

template<typename STORAGE, typename ID>
using ipc_writer = shmem_data<seq_lock<STORAGE>, SHMEM_WRITE, ipc_convert_id<ID>>;
template<typename STORAGE, typename ID>
using ipc_reader = shmem_data<seq_lock<STORAGE>, SHMEM_READ, ipc_convert_id<ID>>;

template<typename STORAGE, typename ID>
int ipc_lazy_write(const STORAGE& entry, ID)
{
  static ipc_writer<STORAGE, ID> writer;

  return writer.write(entry);
}

template<typename STORAGE, typename ID>
int ipc_lazy_read(STORAGE& entry, ID)
{
  static ipc_reader<STORAGE, ID> reader;

  return reader.read(entry);
}

// Ipc methods with id specified at runtime.
// These methods are templated on storage type only,
// and need to explicitly keep track of lazy initialization of transport.
#define ipc_rtid_write(entry, id) ipc_rtid_lazy_get<ipc_rtid_writer, type_info_get_base<typeof(entry)>>(id).write(entry)
#define ipc_rtid_read(entry, id) ipc_rtid_lazy_get<ipc_rtid_reader, type_info_get_base<typeof(entry)>>(id).read(entry)

#include <string>
#include <vector>
#include <tyndall/meta/type_info.h>

template<typename STORAGE>
using ipc_rtid_writer = shmem_data<seq_lock<STORAGE>, SHMEM_WRITE>;
template<typename STORAGE>
using ipc_rtid_reader = shmem_data<seq_lock<STORAGE>, SHMEM_READ>;

template<template<typename>typename TRANSPORT, typename STORAGE>
TRANSPORT<STORAGE>& ipc_rtid_lazy_get(const char* id)
{
  // remove leading slashes
  while (*id == '/')
    ++id;

  std::string converted_id = IPC_SHMEM_PREFIX + std::string{id};
  // replace slash with underscore
  for (char& c : converted_id)
    if (c == '/')
      c = '_';

  struct registry_item
  {
    std::string id;
    TRANSPORT<STORAGE> transport;
  };
  static std::vector<registry_item> registry;

  for (auto& item : registry)
    if (item.id == converted_id)
      return item.transport;

  registry.push_back(registry_item{ .id = converted_id, .transport = TRANSPORT<STORAGE>(converted_id.c_str()) });
  return registry.back().transport;
}

#endif

int ipc_cleanup()
{
  return shmem_unlink_all(IPC_SHMEM_PREFIX);
}
