#pragma once
#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <tyndall/meta/strval.h>

enum shmem_error
{
  SHMEM_SHM_FAILED = 1,
  SHMEM_TRUNCATE_FAILED,
  SHMEM_MAP_FAILED,
};

static inline int shmem_create(void** addr, const char* storage_id, int size)
{
  // memfd_create
  int fd = shm_open(storage_id, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
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

static inline int shmem_unlink(const char* storage_id)
{
  // shm_open cleanup
  return shm_unlink(storage_id);
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
enum shmem_permission
{
  SHMEM_READ = 1<<0,
  SHMEM_WRITE = 1<<1,
};
template<typename DATA_STRUCTURE, int PERMISSIONS, typename PREFIX = strval_t("shmem")>
class shmem_data
{
  DATA_STRUCTURE *ds;
  typename DATA_STRUCTURE::data data;
  typedef typename DATA_STRUCTURE::storage storage;

public:

  shmem_data() : ds(NULL) {}

  shmem_data(const char *id)
  {
    init(id);
  }

  int init(const char *id)
  {
    char handle[PREFIX::size() + 1 + strlen(id) + 1];
    sprintf(handle, "%s_%s", PREFIX::c_str(), id);

    int rc = shmem_create((void**)&ds, handle, sizeof(DATA_STRUCTURE));

    return rc;
  }

  ~shmem_data()
  {
    shmem_unmap((void*)ds, sizeof(DATA_STRUCTURE));
  }

  void write(const storage& entry)
  {
    static_assert(PERMISSIONS & SHMEM_WRITE, "not a writer");
    ds->write(entry, data);
  }

  int read(storage& entry)
  {
    static_assert(PERMISSIONS & SHMEM_READ, "not a reader");
    return ds->read(entry, data);
  }

};
#endif
