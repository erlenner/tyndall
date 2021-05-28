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

template<typename STORAGE>
using ipc_writer = shmem_data<seq_lock<STORAGE>, SHMEM_WRITE, strval_t(IPC_SHMEM_PREFIX)>;
template<typename STORAGE>
using ipc_reader = shmem_data<seq_lock<STORAGE>, SHMEM_READ, strval_t(IPC_SHMEM_PREFIX)>;


#define ipc_read(entry, id) ipc_static_read(entry, id ## _strval)
#define ipc_write(entry, id) ipc_static_write(entry, id ## _strval)

template<typename STORAGE, typename Id>
void ipc_static_write(const STORAGE& entry, Id)
{
  static ipc_writer<STORAGE> writer(Id::c_str());

  writer.write(entry);
}

template<typename STORAGE, typename Id>
int ipc_static_read(STORAGE& entry, Id)
{
  static ipc_reader<STORAGE> reader(Id::c_str());

  return reader.read(entry);
}

#endif

int ipc_cleanup()
{
  return shmem_unlink_all(IPC_SHMEM_PREFIX);
}
