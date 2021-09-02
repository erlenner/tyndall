#include <assert.h>
#include <tyndall/ipc/ipc.h>
#include <tyndall/meta/macro.h>

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

const my_struct ref
{
  .a = 3,
  .b = 1,
  .c = 4,
  .d = 2,
};

#define check(cond) do { if (!(cond)){ ipc_cleanup(); printf( __FILE__ ":" M_STRINGIFY(__LINE__) " " "Assertion failed: " #cond "\n"); exit(1); }} while(0)

int main()
{
  ipc_cleanup();
  {
    {
      my_struct entry = ref;
      ipc_write(entry, "/test/standard");
    }

    {
      my_struct entry = {};
      int rc = ipc_read(entry, "/test/standard");
      check(rc == 0);
      check(entry == ref);
    }
  }

  {
    my_struct entry = ref;
    check(ipc_read(entry, "/test/errors") == -1);
    check(errno == ENOMSG);
    ipc_write(entry, "/test/errors");
    check(ipc_read(entry, "/test/errors") == 0);
    check(ipc_read(entry, "/test/errors") == -1);
    check(errno == EAGAIN);
    check(ipc_read(entry, "/test/errors") == -1);
    check(errno == EAGAIN);
    ipc_write(entry, "/test/errors");
    check(ipc_read(entry, "/test/errors") == 0);
    check(ipc_read(entry, "/test/errors") == -1);
    check(errno == EAGAIN);
  }

  {
    int rc = system(R"(python3 -c "
from pytyndall import ipc_write_float, ipc_read_float
ipc_write_float(42, '/test/pytopic')

f = ipc_read_float('/test/pytopic')
assert f == 42, 'got different value back'
    ")");
    check(rc == 0);
  }

  {
    {
      int rc = system(R"(python3 -c "
from pytyndall import ipc_write_float, ipc_read_float
ipc_write_float(42, '/test/py2c++topic')
    ")");
      check(rc == 0);
    }
    {
      float entry;
      int rc = ipc_read(entry, "/test/py2c++topic");
      check(rc == 0);
      check(entry == 42);
    }
  }

  {
    ipc_rtid_write(ref, "/test/rtid");

    my_struct entry;
    int rc = ipc_rtid_read(entry, "/test/rtid");
    check(rc == 0);
    check(entry == ref);
  }

  {
    {
      //ipc_writer<my_struct, strval_t("test/mt_safe")> writer;
      auto writer = create_ipc_writer(my_struct, "test/mt_safe");
      writer.write(ref);
    }

    {
      //ipc_reader<my_struct, strval_t("test/mt_safe")> reader;
      auto reader = create_ipc_reader(my_struct, "test/mt_safe");
      my_struct entry;
      int rc = reader.read(entry);
      check(rc == 0);
      check(entry == ref);
    }
  }

  {
    {
      auto writer = create_ipc_rtid_writer<my_struct>("test/rtid_mt_safe");
      writer.write(ref);
    }

    {
      auto reader = create_ipc_rtid_reader<my_struct>("test/rtid_mt_safe");
      my_struct entry;
      int rc = reader.read(entry);
      check(rc == 0);
      check(entry == ref);
    }
  }

  // moving
  {
    ipc_rtid_reader<my_struct> a = create_ipc_rtid_reader<my_struct>("test/moving");

    ipc_rtid_reader<my_struct> b;
    b = std::move(a);
  }

  ipc_cleanup();
}
