#include <zmq.h>
#include <google/protobuf/message.h>
#include <google/protobuf/any.pb.h>
#include <google/protobuf/empty.pb.h>

#include <errno.h>
#include <string.h>

#ifdef ZMQ_PROTO_DEBUG
#define zmq_proto_assert(expr) do { (void)sizeof(expr); } while(0)
#else
#define zmq_proto_assert(expr) do { if (!(expr)) { fprintf(stderr, "[ %s:%d %s ] " "assertion failed: (" #expr ") errno: %s\n", __FILE__, __LINE__, __func__, zmq_strerror(zmq_errno())); exit(1); } } while(0)
#endif

namespace zmq_proto
{

// Wrapper types around zeromq contexts, sockets and messages, as per https://linux.die.net/man/7/zmq :

class context_t
{
  void *context;

public:

  context_t(int n_threads)
  {
    int rc = init(n_threads);
    zmq_proto_assert(rc == 0);
  }

  int init(int n_threads)
  {
    context = zmq_ctx_new();
    zmq_proto_assert(context != NULL);

    {
      int rc = zmq_ctx_set(context, ZMQ_IO_THREADS, n_threads);
      zmq_proto_assert(rc == 0);
    }

    return 0;
  }

  void* zmq_handle() const
  {
    return context;
  }

  void* zmq_handle()
  {
    return context;
  }

  ~context_t()
  {
    int rc;
    do {
      rc = zmq_ctx_destroy(context);
    } while (rc == -1 && zmq_errno() == EINTR);

    zmq_proto_assert(rc == 0);
    context = NULL;
  }
};

enum socket_type_t{ REQ = ZMQ_REQ, REP = ZMQ_REP, PUB = ZMQ_PUB, SUB = ZMQ_SUB };

enum socket_mode_t{ UNSPECIFIED, BIND, CONNECT };

template<socket_type_t socket_type>
class socket_t
{
  void *socket;

public:

  socket_t(const context_t& context, const char *addr, socket_mode_t mode = UNSPECIFIED)
  {
    socket = zmq_socket(context.zmq_handle(), static_cast<int>(socket_type));
    zmq_proto_assert(socket != NULL);

    {
      int rc;

      switch(mode)
      {
        case BIND:
          rc = zmq_bind(socket, addr);
          break;

        case CONNECT:
          rc = zmq_connect(socket, addr);
          break;

        case UNSPECIFIED:
          switch(socket_type)
          {
            case REP:
            case PUB:
              rc = zmq_bind(socket, addr);
              break;
            case REQ:
            case SUB:
              rc = zmq_connect(socket, addr);
              break;
          }
          break;
      }

      zmq_proto_assert(rc == 0);

      if (socket_type == SUB)
        rc = zmq_setsockopt(socket, ZMQ_SUBSCRIBE, "", 0);

      zmq_proto_assert(rc == 0);
    }
  }

  void* zmq_handle() const
  {
    return socket;
  }

  void* zmq_handle()
  {
    return socket;
  }

  ~socket_t()
  {
    int rc = zmq_close(socket);
    zmq_proto_assert(rc == 0);

    socket = NULL;
  }
};


class msg_t
{
  zmq_msg_t msg;

public:

  msg_t()
  {
    int rc = zmq_msg_init(&msg);
    zmq_proto_assert (rc == 0);
  }

  const zmq_msg_t* zmq_handle() const
  {
    return &msg;
  }

  zmq_msg_t* zmq_handle()
  {
    return &msg;
  }

  const char* data()
  {
    return static_cast<const char*>(zmq_msg_data(&msg));
  }

  int size() const
  {
    return static_cast<int>(zmq_msg_size(&msg));
  }

  ~msg_t()
  {
    int rc = zmq_msg_close(&msg);
    zmq_proto_assert(rc == 0);
  }

  int match(const char* pattern, int pattern_length)
  {
    int rc;

    if ((size() != pattern_length)
      || (strncmp(data(), pattern, pattern_length) != 0))
    {
      rc = -1;
      errno = EBADMSG;
    }
    else
      rc = 0;

    return rc;
  }

  template<int pattern_size>
  int match(const char(&pattern)[pattern_size])
  {
    constexpr int pattern_length = pattern_size - 1;

    return match(pattern, pattern_length);
  }

  int match(const char* pattern)
  {
    const int pattern_length = strlen(pattern);

    return match(pattern, pattern_length);
  }
};

// Send and receive functions for zeromq messages with protobuf payloads:

// Message format consists of two frames.
// First frame is a byte-array denoting endpoint-id / topic.
// Second frame is a serialized protobuf::Any.

// send is used to send a message, or a frame
// recv is used to receive the first, or all frames.
// recv_more is used to receive the second frame, or all frames after the first frame.

enum send_recv_flags { NONE = 0, DONTWAIT = ZMQ_DONTWAIT, SNDMORE = ZMQ_SNDMORE };

template<socket_type_t socket_type>
int send(const google::protobuf::Any& payload, const socket_t<socket_type>& socket, const char *id, int id_len, send_recv_flags flags = NONE)
{
  int rc;

  // first send id
  const int sent = zmq_send(socket.zmq_handle(), id, id_len, static_cast<int>(flags | SNDMORE));
  if (sent < 0)
  {
    zmq_proto_assert(zmq_errno() == EAGAIN);
    rc = -1;
    errno = EAGAIN;
  }
  else
  {
    const int payload_size = payload.ByteSizeLong();
    char payload_buf[payload_size];
    payload.SerializeToArray(payload_buf, payload_size);

    // then send payload
    const int sent = zmq_send(socket.zmq_handle(), payload_buf, payload_size, static_cast<int>(flags));
    if (sent < 0)
    {
      zmq_proto_assert(zmq_errno() == EAGAIN);
      rc = -1;
      errno = EAGAIN;
    }
    else
      rc = 0;
  }

  return rc;
}

template<socket_type_t socket_type, typename Message>
std::enable_if_t<std::is_base_of<::google::protobuf::Message, Message>::value, int>
send(const Message& msg, const socket_t<socket_type>& socket, const char *id, int id_len, send_recv_flags flags = NONE)
{

  google::protobuf::Any payload;
  payload.PackFrom(msg);

  return send(payload, socket, id, id_len, flags);
}

template<socket_type_t socket_type, typename Message, int id_size>
std::enable_if_t<std::is_base_of<::google::protobuf::Message, Message>::value, int>
send(const Message& msg, const socket_t<socket_type>& socket, const char(&id)[id_size], send_recv_flags flags = NONE)
{
  // static id
  constexpr int id_len = id_size - 1;

  return send<socket_type, Message>(msg, socket, id, id_len, flags);
}

template<socket_type_t socket_type>
int recv(msg_t& msg, const socket_t<socket_type>& socket, send_recv_flags flags = NONE)
{
  int rc;

  const int received = zmq_msg_recv(msg.zmq_handle(), socket.zmq_handle(), static_cast<int>(flags));

  if (received < 0)
  {
    zmq_proto_assert(zmq_errno() == EAGAIN);
    rc = -1;
    errno = EAGAIN;
  }
  else
  {
    rc = 0;
    zmq_proto_assert(msg.size() == received);
  }

  return rc;
}

template<socket_type_t socket_type>
int recv_more(msg_t& msg, const socket_t<socket_type>& socket, send_recv_flags flags = NONE)
{
  int64_t more;
  size_t more_size = sizeof(more);

  int rc = zmq_getsockopt(socket.zmq_handle(), ZMQ_RCVMORE, &more, &more_size);
  zmq_proto_assert(rc == 0);

  if (more == 1)
  {
    rc = recv(msg, socket, flags);
  }
  else
  {
    assert(more == 0);
    rc = -1;
    errno = EOPNOTSUPP;
  }

  return rc;
}

template<socket_type_t socket_type>
int recv_more(const socket_t<socket_type>& socket, send_recv_flags flags = NONE)
{
  int rc;
  do
  {
    msg_t msg;
    rc = recv_more(msg, socket, flags);
  } while (rc == 0);

  return rc;
}

template<socket_type_t socket_type, typename Message>
std::enable_if_t<std::is_base_of<::google::protobuf::Message, Message>::value, int>
recv_more(Message& proto_msg, const socket_t<socket_type>& socket, send_recv_flags flags = NONE)
{
  msg_t msg;
  google::protobuf::Any any;

  int rc = recv_more(msg, socket, flags);

  if (rc != 0)
    rc = -1;
  else if (!any.ParseFromArray(msg.data(), msg.size()))
  {
    rc = -1;
    errno = EPROTO;
  }
  else if (!any.Is<Message>())
  {
    rc = -1;
    errno = ENOMSG;
  }
  else
  {
    rc = 0;
    bool unpack_rc = any.UnpackTo(&proto_msg);
    zmq_proto_assert(unpack_rc);
  }

  return rc;
}

template<socket_type_t socket_type>
int recv_more(google::protobuf::Any& proto_msg, const socket_t<socket_type>& socket, send_recv_flags flags = NONE)
{
  msg_t msg;

  int rc = recv_more(msg, socket, flags);

  if (rc != 0)
    rc = -1;
  else if (!proto_msg.ParseFromArray(msg.data(), msg.size()))
  {
    rc = -1;
    errno = EPROTO;
  }
  else
    rc = 0;

  return rc;
}

template<socket_type_t socket_type, typename Message, int id_size>
std::enable_if_t<std::is_base_of<::google::protobuf::Message, Message>::value, int>
recv(Message& proto_msg, const socket_t<socket_type>& socket, const char(&id)[id_size], send_recv_flags flags = NONE)
{
  msg_t id_msg;

  int rc;
  (0 == (rc = recv(id_msg, socket, flags)))
    && (0 == (rc = id_msg.match(id)))
    && (0 == (rc = recv_more(proto_msg, socket, flags)));

  return rc;
}

template<socket_type_t socket_type>
int recv(const socket_t<socket_type>& socket, send_recv_flags flags = NONE)
{
  msg_t msg;

  int rc;
  (0 == (rc = recv(msg, socket, flags)))
    && (0 == (rc = recv_more(socket, flags)));

  return rc;
}

}
