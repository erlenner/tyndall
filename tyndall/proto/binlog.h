#include <google/protobuf/util/delimited_message_util.h>
#include <fcntl.h>
#include <errno.h>

#include "tyndall/meta/strval.h"

int binlog_create(const char* path)
{
  return open(path, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
}

template<typename ProtoMessage>
std::enable_if_t<std::is_base_of<::google::protobuf::Message, ProtoMessage>::value, int>
binlog_write(int fd, const ProtoMessage& proto)
{
  if (fd == -1)
  {
    errno = ENOENT;
    return -1;
  }
  else
  {
    if (::google::protobuf::util::SerializeDelimitedToFileDescriptor(proto, fd))
      return 0;
    else
    {
      errno = EPROTO;
      return -1;
    }
  }
}

int binlog_open(const char* path)
{
  return open(path, O_RDONLY);
}

template<typename ProtoMessage>
std::enable_if_t<std::is_base_of<::google::protobuf::Message, ProtoMessage>::value, int>
binlog_read(int fd, ProtoMessage& proto)
{
  if (fd == -1)
  {
    errno = ENOENT;
    return -1;
  }
  else
  {
    static ::google::protobuf::io::FileInputStream stream(fd);
    bool clean_eof;
    if (::google::protobuf::util::ParseDelimitedFromZeroCopyStream(&proto, &stream, &clean_eof))
      return 0;
    else
    {
      if (clean_eof)
        errno = EIO;
      else
        errno = EPROTO;
      return -1;
    }
  }
}
