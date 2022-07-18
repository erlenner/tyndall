#include <assert.h>
#include <thread>
#include <chrono>
#include <stdlib.h>

#include "tyndall/ipc/ipc.h"
#include "tyndall/meta/macro.h"

struct my_struct
{
  long a;
  double b;
  char c;
  unsigned long d;
  //bool operator==(const my_struct&) const = default; // c++20
  bool operator==(const my_struct& rhs)
  {
    return (rhs.a == a) && (rhs.b == b) && (rhs.c == c) && (rhs.d == d);
  }

  my_struct& operator++()
  {
    ++a;
    ++b;
    ++c;
    ++d;
    return *this;
  }

  bool has_consistent_even_ness() const
  {
    int first_bit = (a & 1);

    return (((int)b & 1) == first_bit)
      && ((c & 1) == first_bit)
      && ((d & 1) == first_bit);
  }
};

#define check(cond) do { if (!(cond)){ ipc_cleanup(); printf( __FILE__ ":" M_STRINGIFY(__LINE__) " " "Assertion failed: " #cond "\n"); exit(1); }} while(0)

int main()
{
  ipc_cleanup();

  const int n_threads = 4;
  std::thread threads[n_threads];

  const int n_ids = 10 * n_threads;
  const int id_length = 15;
  char ids[n_ids][id_length + 1];

  const char id_char_begin = 'a';
  const char id_char_end = 'z' + 1;
  //printf("begin, end-1: %c %c\n", id_char_begin, id_char_end-1);

  for (int i=0; i < n_ids; ++i)
  {
    for (int j=0; j < id_length; ++j)
      ids[i][j] = id_char_begin + rand() % (id_char_end - id_char_begin);
    ids[i][id_length] = '\0';
    //printf("id %d: %s\n", i, &ids[i][0]);
  }

  // check for duplicates
  {
    bool duplicate = false;
    do
    {
      for (int i=0; i < n_ids; ++i)
        for (int j=0; j < n_ids; ++j)
          if ((i!=j) && (strcmp(&ids[i][0], &ids[j][0]) == 0))
          {
            duplicate = true;

            for (int j=0; j < id_length; ++j)
              ids[i][j] = id_char_begin + rand() % (id_char_end - id_char_begin);
          }
    } while (duplicate);
  }

  const int max_ids_per_thread = n_ids / 2 ?: 1;
  static_assert(max_ids_per_thread <= n_ids);
  //printf("max_ids_per_thread: %d\n", max_ids_per_thread);
  // array to divide the ids between threads
  int ids_per_thread[n_threads][max_ids_per_thread];
  std::fill(&ids_per_thread[0][0], &ids_per_thread[n_threads][max_ids_per_thread], -1);

  int writing_thread_for_id[n_ids];
  std::fill(&writing_thread_for_id[0], &writing_thread_for_id[n_ids], -1);
  

  static_assert(n_ids >= n_threads);
  for (int i=0; i < n_threads; ++i)
  {
    // give ids to the thread
    const int thread_ids = 1 + rand() % max_ids_per_thread;
    for (int j=0; j < thread_ids; ++j)
    {
      ids_per_thread[i][j] = rand() % n_ids;

      // dont insert duplicate ids
      bool duplicate;
      do {
        duplicate = false;
        for (int k = 0; k < j; ++k)
        {
          if (ids_per_thread[i][k] == ids_per_thread[i][j])
          {
            duplicate = true;
            ids_per_thread[i][j] = (ids_per_thread[i][j] + 1) % n_ids;
          }
        }
      } while (duplicate);

      // register thread as writer if there is no other writer of the id
      if (writing_thread_for_id[ids_per_thread[i][j]] == -1)
        writing_thread_for_id[ids_per_thread[i][j]] = i;
    }

    // initialize thread
    threads[i] = std::thread{[thread_index = i, ids, n_ids, ids_per_thread, thread_ids, writing_thread_for_id]()
      {
        int n_writers = 0, n_readers = 0;
        ipc_rtid_writer<my_struct> writers[thread_ids];
        ipc_rtid_reader<my_struct> readers[thread_ids];

        // initialise writers and readers
        for (int i=0; i < thread_ids; ++i)
        {
          const int id_index = ids_per_thread[thread_index][i];
          const char* id = &ids[id_index][0];
          std::string prepared_id = id_rtid_prepare(id);
          if (writing_thread_for_id[id_index] == thread_index)
          {
            writers[n_writers++].init(prepared_id.c_str());
            //printf("%d writes to %s\n", thread_index, prepared_id.c_str());
          }
          else
            readers[n_readers++].init(prepared_id.c_str());
        }

        my_struct write_entry = {0};

        const int n_iterations = 10000;
        for (int i=0; i<n_iterations; ++i)
        {
          // write
          for (int j=0; j < n_writers; ++j)
          {
            ++write_entry;
            //printf("thread %d: writing %ld %f %d %lu\n", thread_index, write_entry.a, write_entry.b, write_entry.c, write_entry.d);
            writers[j].write(write_entry);
          }

          // read
          for (int j=0; j < n_readers; ++j)
          {
            my_struct read_entry;
            int rc = readers[j].read(read_entry);

            if (rc == 0)
            {
              //printf("thread %d: checking %ld\n", read_entry.a);
              //printf("thread %d: reading %ld %f %d %lu\n", thread_index, read_entry.a, read_entry.b, read_entry.c, read_entry.d);

              if (!read_entry.has_consistent_even_ness())
              {
                printf("thread %d: wrong evenness: %ld %f %d %lu\n", thread_index, read_entry.a, read_entry.b, read_entry.c, read_entry.d);
                ipc_cleanup();
                exit(1);
              }
            }
          }
          //std::this_thread::sleep_for(std::chrono::milliseconds{1});
        }

        //printf("thread %d: done\n", thread_index);
      }
    };
  }

  for (auto& thread : threads)
    thread.join();

  ipc_cleanup();
}
