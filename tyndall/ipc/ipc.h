#pragma once
#include "shmem.h"

//                                    echo ipc | sha1sum
const char ipc_shmem_prefix[] = "ipc" "1ef42bc4e0bbfeb0ac34bc3642732768cf6f77b7";

#ifdef __cplusplus

#include "seq_lock.h"

template<typename STORAGE>
using ipc_writer = shmem_data<seq_lock<STORAGE>, SHMEM_WRITE, ipc_shmem_prefix>;
template<typename STORAGE>
using ipc_reader = shmem_data<seq_lock<STORAGE>, SHMEM_READ, ipc_shmem_prefix>;


#include <tyndall/meta/strval.h>
#define ipc_read(entry, id) ipc_static_read(entry, id ## _strval)
#define ipc_write(entry, id) ipc_static_write(entry, id ## _strval)

template<typename STORAGE, typename Id>
int ipc_static_write(const STORAGE& entry, Id)
{
  static ipc_writer<STORAGE> writer(Id::c_str());

  return writer.write(entry);
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
  return shmem_unlink_all(ipc_shmem_prefix);
}
