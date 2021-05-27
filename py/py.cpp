#include <boost/python.hpp>
#include <tyndall/ipc/ipc.h>

template<typename STORAGE>
void py_ipc_write(const STORAGE& entry, const char* id)
{
  static ipc_writer<STORAGE> writer(id);

  writer.write(entry);
}

template<typename STORAGE>
STORAGE py_ipc_read(const char* id)
{
  static ipc_reader<STORAGE> reader(id);

  STORAGE entry;
  if (reader.read(entry) == 0)
    return entry;
  else
    return {};
}

BOOST_PYTHON_MODULE(pytyndall)
{
  using namespace boost::python;
  def("ipc_write_float", py_ipc_write<float>);
  def("ipc_read_float", py_ipc_read<float>);
}

/*
>>> from pytyndall import ipc_write_float, ipc_read_float
>>> ipc_write_float(5, "my_topic")
>>> ipc_read_float("my_topic")
5.0
*/
