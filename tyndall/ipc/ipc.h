#pragma once
#include "shmem.h"

#ifdef __cplusplus

#include "seq_lock.h"
#include "id.h"
#include <tyndall/meta/strval.h>

// ipc_write and ipc_read are templated on storage type and id, so that the transport will be initiated only on calls with new combinations of storage type and id.
// The id needs to be a string literal, otherwise use ipc_lazy_write and ipc_lazy_read directly.
#define ipc_write(entry, id) ipc_lazy_write(entry, id ## _strval)

// ipc_read return value: 0 on success, -1 on failure.
// On failure, ERRNO is ENOMSG or EAGAIN.
// ENOMSG means there is no message available.
// EAGAIN means there is no new message available, and the last message received will be returned instead.
#define ipc_read(entry, id) ipc_lazy_read(entry, id ## _strval)

template<typename STORAGE, typename ID>
using ipc_writer = shmem_data<seq_lock<STORAGE>, SHMEM_WRITE, id_prepare<ID>>;
template<typename STORAGE, typename ID>
using ipc_reader = shmem_data<seq_lock<STORAGE>, SHMEM_READ, id_prepare<ID>>;

#define create_ipc_writer(storage_type, id) (ipc_writer<storage_type, decltype(id ## _strval)>{})
#define create_ipc_reader(storage_type, id) (ipc_reader<storage_type, decltype(id ## _strval)>{})

template<typename STORAGE, typename ID>
inline void ipc_lazy_write(const STORAGE& entry, ID)
{
  static ipc_writer<STORAGE, ID> writer;

  writer.write(entry);
}

template<typename STORAGE, typename ID>
inline int ipc_lazy_read(STORAGE& entry, ID)
{
  static ipc_reader<STORAGE, ID> reader;

  return reader.read(entry);
}

// Ipc methods with id specified at runtime.
#include <vector>
#include <tyndall/meta/typeinfo.h>
#include <shared_mutex>

template<typename STORAGE>
using ipc_rtid_writer = shmem_data<seq_lock<STORAGE>, SHMEM_WRITE>;
template<typename STORAGE>
using ipc_rtid_reader = shmem_data<seq_lock<STORAGE>, SHMEM_READ>;

template<typename STORAGE>
inline ipc_rtid_writer<STORAGE> create_ipc_rtid_writer(const char* id)
{
  std::string prepared_id = id_rtid_prepare<STORAGE>(id);
  return ipc_rtid_writer<STORAGE>{prepared_id.c_str()};
}

template<typename STORAGE>
inline ipc_rtid_reader<STORAGE> create_ipc_rtid_reader(const char* id)
{
  std::string prepared_id = id_rtid_prepare<STORAGE>(id);
  return ipc_rtid_reader<STORAGE>{prepared_id.c_str()};
}

#endif

inline int ipc_cleanup()
{
  return shmem_unlink_all(IPC_SHMEM_PREFIX);
}
