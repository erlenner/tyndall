# Tyndall

Blueye Robotics open source c++ components

### Build
```
mkdir build
cd build
cmake ..
make
```

### Install
```
make install
```

### Build examples
```
make examples
```

# Highlights

## meta/typevals
Container where entries can have different types.

Example:
```cpp
#include <tyndall/meta/typevals.h>

struct A{int a; float b;};
struct B{double x; char y; char z;};

static constexpr auto tv = typevals{} + 5 + A{4,2} + B{3,9,1};
static_assert(tv.size() == 3);

constexpr auto A42 = tv.get<1>();
static_assert(A42.b == 2);
constexpr auto B391 = tv[std::integral_constant<int, 2>()];
static_assert(B391.x == 3);
```

## meta/iterate

Function that calls the given callback with integral constants from 0 to the specified index.
Useful for iterating at compile time.

Example:
```cpp
#include <tyndall/meta/iterate.h>

iterate<10>(
[](auto index){
  // index = std::integral_constant<int, 0>(), std::integral_constant<int, 1>(), ... ,std::integral_constant<int, 9>()
  static_assert((index >= 0) && (index < 10));
  printf("%d ", index()); // 0 1 2 3 4 5 6 7 8 9
});
```

## zmq\_proto

[ZeroMQ](https://zeromq.org/) ([libzmq](https://github.com/zeromq/libzmq)) / [Protobuf](https://developers.google.com/protocol-buffers) based tcp communication.

Example:

### Publisher
```cpp
#include <tyndall/proto/zmq_proto.h>

zmq_proto::context_t context{1};
zmq_proto::socket_t<zmq_proto::PUB> socket(context, "tcp://*:5444");

google::protobuf::Int32Value msg;
msg.set_value(42);

int rc = zmq_proto::send(msg, socket);
if (rc != 0)
  printf("errno: %s\n", strerror(errno));
```

### Subscriber
```cpp
#include <tyndall/proto/zmq_proto.h>

zmq_proto::context_t context{1};
zmq_proto::socket_t<zmq_proto::SUB> socket(context, "tcp://127.0.0.1:5444");

while(1)
{
  google::protobuf::Int32Value msg;

  int rc = zmq_proto::recv(msg, socket);
  if (rc != 0)
  {
    printf("errno: %s\n", strerror(errno));
    exit(1);
  }
}
```
Full examples in [examples/proto/zmq\_proto\_pub.cpp](examples/proto/zmq_proto_pub.cpp) and [examples/proto/zmq\_proto\_sub.cpp](examples/proto/zmq_proto_sub.cpp).

## ipc
Inter process communication in Linux based on shared memory and lockless data structures.

Shared memory is allocated / mapped in `ipc_write` and `ipc_read` on the first call with a new combination of entry type and id.
A custom [seqlock](https://en.wikipedia.org/wiki/Seqlock) implementation is used for synchronization.

Example:

#### Writer
```cpp
#include <tyndall/ipc/ipc.h>

my_struct entry = { .field = 42 };

ipc_write(entry, "my_topic");
```

#### Reader
```cpp
#include <tyndall/ipc/ipc.h>

while (1)
{
  my_struct entry;

  if (ipc_read(entry, "my_topic") == 0)
    printf("entry field: %d\n", entry.field);
}
```
Full example in [examples/ipc/](examples/ipc/).
