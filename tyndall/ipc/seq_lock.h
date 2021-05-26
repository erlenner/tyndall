/*
seq_lock.h

Sequence lock: https://en.wikipedia.org/wiki/Seqlock, https://lwn.net/Articles/21355/
Supports single producer, multiple consumers
Reader always gets the most recent entry, potentially skipping older entries

Inspired by:
http://www.1024cores.net/home/lock-free-algorithms/reader-writer-problem/improved-lock-free-seqlock
https://github.com/torvalds/linux/blob/master/include/vdso/helpers.h
https://github.com/rigtorp/Seqlock
*/

#pragma once
#include <assert.h>
#include "smp.h"


#ifdef __cplusplus


struct seq_lock_data
{
  int last_seq;
};

template<typename STORAGE>
class seq_lock
{
  static_assert(std::is_nothrow_copy_assignable<STORAGE>::value);
  static_assert(std::is_trivially_copy_assignable<STORAGE>::value);

  int seq;
  STORAGE entry;

public:
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
    int seq1, seq2, ret;

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

    ret = (seq2 != data.last_seq) ? 0 : -1;
    data.last_seq = seq2;
    return ret;
  }

  typedef STORAGE storage;
  typedef seq_lock_data data;

} __attribute__ ((aligned(CACHELINE_BYTES)));

#endif
