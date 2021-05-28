/*
seq_lock.h

Sequence lock: https://en.wikipedia.org/wiki/Seqlock, https://lwn.net/Articles/21355/
Supports single writer, multiple readers
Reader always gets the most recent entry

Inspired by:
http://www.1024cores.net/home/lock-free-algorithms/reader-writer-problem/improved-lock-free-seqlock
https://github.com/torvalds/linux/blob/master/include/vdso/helpers.h
https://github.com/rigtorp/Seqlock
*/

#pragma once
#include <assert.h>
#include <errno.h>
#include "smp.h"


#ifdef __cplusplus


struct seq_lock_data
{
  int prev_seq = 0;
  bool has_read_once = false;
};

template<typename STORAGE>
class seq_lock
{
  static_assert(std::is_nothrow_copy_assignable_v<STORAGE>);
  static_assert(std::is_trivially_copy_assignable_v<STORAGE>);

  int seq;
  STORAGE entry;

public:

  typedef STORAGE storage;
  typedef seq_lock_data data;

  void write(const STORAGE& entry, seq_lock_data data = {0})
  {
    int seq = this->seq;

    smp_write_once(this->seq, ++seq);
    smp_wmb();

    this->entry = entry;

    smp_wmb();
    smp_write_once(this->seq, ++seq);
  }

  int read(STORAGE& entry, seq_lock_data& data)
  {
    int seq1, seq2, rc;

    seq1 = smp_read_once(this->seq);
    while(1)
    {
      if (seq1 & 1)
      {
        cpu_relax();
        seq1 = smp_read_once(this->seq);
        continue;
      }

      smp_rmb();
      entry = this->entry;
      smp_rmb();

      seq2 = smp_read_once(this->seq);
      if (seq2 == seq1)
        break;

      seq1 = seq2;
    }

    if (seq2 != data.prev_seq)
    {
      rc = 0;
      data.prev_seq = seq2;
      if (!data.has_read_once)
        data.has_read_once = true;
    }
    else if ((seq2 == 0) && !data.has_read_once)
    {
      rc = -1;
      errno = ENOMSG;
    }
    else
    {
      rc = -1;
      errno = EAGAIN;
    }
    return rc;
  }
} __attribute__ ((aligned(CACHELINE_BYTES)));

#endif
