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
#include <tyndall/reflect/print_format.h>

int main(int argc, char** argv)
{
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
  char* buf = (char*)malloc(buf_size + CACHELINE_BYTES);
  char* buf_to_free = buf;

  // align buf
  {
    constexpr size_t alignment = CACHELINE_BYTES;
    size_t alignment_error = reinterpret_cast<uintptr_t>(buf) % alignment;
    if (alignment_error > 0)
      buf += alignment - alignment_error;
  }

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

          size_t printed = print_format(*f, b);
          if (printed == 0)
          {
            switch(*f)
            {
              case 's':
                printf("%ss, ", b);
                printed = strlen(b);
                b += printed;
                break;
              case 'S':
                // assumes sso optimized std::string
                printf("%ss, ", reinterpret_cast<const std::string*>(b)->c_str());
                printed = sizeof(std::string);
                b += printed;
                break;
              default:
                printf("error, wrong parameter: %c\n", *f);
            }
            if (printed == 0)
              break;
          }
          else
          {
            printf(", ");
            b += printed;
          }

          if ((size_t)(b - buf) > buf_size)
          {
            printf("fmt is too large for buffer at %zu", buf_size);
            break;
          }
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

  free(buf_to_free);

  {
    int rc = munmap(mapped, ipc_size);
    assert(rc == 0);
  }

  return 0;
}
