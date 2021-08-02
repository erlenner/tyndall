#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
//#include <tyndall/ext/embed.hpp>
//#include <experimental/reflect>
#include <typeinfo>
#include <string.h>
#include <ctype.h>

// ipc
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <tyndall/ipc/smp.h>
#include <examples/ipc/my_struct.h>

int main(int argc, char** argv)
{
  //printf("%s\n", typeid(unsigned char).name());
  printf("my_struct: %s\n", typeid(my_struct).name());

  if (argc < 2)
  {
    printf("Usage: %s <ipc_name> <format>\n", argv[0]);
    exit(1);
  }

  const char* ipc_id = argv[1];
  const char *ipc_prefix = "/dev/shm/";

  if (strncmp(ipc_id, ipc_prefix, strlen(ipc_prefix)) == 0)
    ipc_id += strlen(ipc_prefix);

  //printf("id: %s\n", ipc_id);

  // ipc
  int fd = shm_open(ipc_id, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
  assert(fd != -1);

  size_t ipc_size;
  {
    struct stat st;
    int rc = fstat(fd, &st);
    assert(rc == 0);
    ipc_size = st.st_size;
  }

  void* mapped = mmap(NULL, ipc_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  assert(mapped != MAP_FAILED);

  int& ipc_seq = *(int*)mapped;
  const char* ipc_buf = (const char*)mapped + sizeof(ipc_seq);
  const size_t buf_size = ipc_size - 2*sizeof(int);
  char* buf = (char*)malloc(buf_size);

  int seq = 0;
  while(1)
  {
    bool new_buf = false;
    {
      int seq1, seq2;

      seq1 = smp_read_once(ipc_seq);
      while(1)
      {
        if (seq1 & 1)
        {
          cpu_relax();
          seq1 = smp_read_once(ipc_seq);
          continue;
        }

        smp_rmb();

        memcpy(buf, ipc_buf, buf_size);

        smp_rmb();

        seq2 = smp_read_once(ipc_seq);
        if (seq2 == seq1)
          break;

        seq1 = seq2;
      }

      if (seq1 != seq)
      {
        new_buf = true;
        seq = seq1;
      }
    }

    if (new_buf)
    {
      if (argc > 2)
      {
        char* fmt = argv[2];
        const char* b = buf;
        for (char* f = fmt; (*f!='\0'); ++f)
        {
          if (isdigit(*f))
          {
            long num = strtol(f, &f, 10);
            printf("<%ld>, ", num);
            b += num;
          }

          {
            switch(*f)
            {
              case 'i':
                printf("%di, ", *(int*)b);
                b += sizeof(int);
                break;
              case 'f':
                printf("%ff, ", *(float*)b);
                b += sizeof(float);
                break;
              case 's':
                printf("%ss, ", b);
                b += strlen(b);
                break;
            }
          }
          if ((size_t)(b - buf) > buf_size)
            printf("fmt is too large for buffer at %zu\n", buf_size);
        }
        printf("\n");
      }
      else
      {
        for (size_t i=0; i<buf_size; ++i)
          printf("%02x", buf[i]);
        printf("\n");
        //printf("int: %d\n", *(int*)buf);
      }
    }
    else
      usleep(1000);
  }

  free(buf);

  {
    int rc = munmap(mapped, ipc_size);
    assert(rc == 0);
  }

  return 0;
}
