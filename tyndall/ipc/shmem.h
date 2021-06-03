#pragma once
#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <typeinfo>
#include <typeindex>
#include <tyndall/meta/strval.h>

enum shmem_error
{
  SHMEM_SHM_FAILED = 1,
  SHMEM_TRUNCATE_FAILED,
  SHMEM_MAP_FAILED,
};

static inline int shmem_create(void** addr, const char* id, int size)
{
  // memfd_create
  int fd = shm_open(id, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
  if (fd == -1)
    return SHMEM_SHM_FAILED;

  int rc = ftruncate(fd, size);
  if (rc != 0)
    return SHMEM_TRUNCATE_FAILED;

  *addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if(*addr == MAP_FAILED)
    return SHMEM_MAP_FAILED;

  return 0;
}

static inline int shmem_unmap(void *addr, int size)
{
  return munmap(addr, size);
}

static inline int shmem_unlink(const char* id)
{
  // shm_open cleanup
  return shm_unlink(id);
}

static inline int shmem_unlink_all(const char *prefix)
{
  DIR *dir;
  struct dirent *ent;
  dir = opendir("/dev/shm");
  if (dir == NULL)
    return -1;

  while((ent = readdir(dir)) != NULL)
  {
    const char *dir_name = ent->d_name;
    int match = 1;

    for (int i=0; i < (int)strlen(prefix); ++i)
      if (dir_name[i] != prefix[i])
        match = 0;

    if (match)
      shmem_unlink(dir_name);
  }
  return 0;
}

#ifdef __cplusplus
#include <type_traits>
#include <assert.h>
#include <tyndall/meta/typeinfo.h>
#include "smp.h"

enum shmem_permission
{
  SHMEM_READ = 1<<0,
  SHMEM_WRITE = 1<<1,
};
template<typename DATA_STRUCTURE, int PERMISSIONS, typename ID = strval_t("")>
class shmem_data
{
  void *buf;
  typename DATA_STRUCTURE::data data;
  typedef typename DATA_STRUCTURE::storage storage;

public:

  shmem_data() noexcept
  : buf(NULL)
  {
    static_assert(ID::occurrences('/') == 0, "Id can't have slashes");

    init(ID::c_str());
  }

  shmem_data(const char* id) noexcept
  {
    static_assert(ID::length() == 0, "Static id should be empty when specifying runtime id");

    init(id);
  }

  void init(const char *id) noexcept
  {
    //static_assert(type_info_hash(DATA_STRUCTURE) == 0);
    //printf("%u\n\n", type_info_hash(DATA_STRUCTURE));

    constexpr auto type_hash = type_info_hash(DATA_STRUCTURE);

    constexpr int size = sizeof(DATA_STRUCTURE) + sizeof(type_hash);
    int rc = shmem_create(&buf, id, size);

    assert(rc == 0);

    // put type hash in the end for validation
    auto hash_loc = reinterpret_cast<decltype(&type_hash)>(reinterpret_cast<DATA_STRUCTURE*>(buf) + 1);
    if (smp_cmp_xch(*hash_loc, 0, type_hash) == -1)
      assert(*hash_loc == type_hash);
  }

  ~shmem_data() noexcept
  {
    if ((buf != NULL) && (buf != (void*)-1))
      shmem_unmap(buf, sizeof(DATA_STRUCTURE));
  }

  int write(const storage& entry) noexcept
  {
    static_assert(PERMISSIONS & SHMEM_WRITE, "needs write permission");
    return reinterpret_cast<DATA_STRUCTURE*>(buf)->write(entry, data);
  }

  int read(storage& entry) noexcept
  {
    static_assert(PERMISSIONS & SHMEM_READ, "needs read permission");
    return reinterpret_cast<DATA_STRUCTURE*>(buf)->read(entry, data);
  }

  // disable copy
  shmem_data operator=(const shmem_data) noexcept = delete;
  shmem_data(shmem_data& other) noexcept = delete;
  shmem_data operator=(shmem_data&& other) noexcept = delete;

  shmem_data(shmem_data&& other)
    : buf(other.buf)
    , data(other.data)
  {
    other.buf = NULL; // invalidate shared memory
  }
};
#endif
