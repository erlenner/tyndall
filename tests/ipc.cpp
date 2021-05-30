#include <tyndall/ipc/ipc.h>

int main()
{
  struct my_struct
  {
    long a;
    double b;
    char c;
    unsigned long d;
    //bool operator==(my_struct) = default; // c++20
    bool operator==(my_struct rhs)
    {
      return (rhs.a == a)
        && (rhs.b == b)
        && (rhs.c == c)
        && (rhs.d == d);
    }
  };

  my_struct ref
  {
    .a = 3,
    .b = 1,
    .c = 4,
    .d = 2,
  };

  {
    my_struct entry = ref;
    int rc = ipc_write(entry, "/test/topic");
    assert(rc == 0);
  }

  {
    my_struct entry = {0};
    int rc = ipc_read(entry, "/test/topic");
    assert(rc == 0);
    assert(entry == ref);
  }

}
