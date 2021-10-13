#include <boost/python.hpp>
#include <tyndall/ipc/ipc.h>

template<template<typename>typename TRANSPORT, typename STORAGE>
inline TRANSPORT<STORAGE>& create_ipc_rtid_lazy(const char* id)
{
  std::string prepared_id = id_rtid_prepare<STORAGE>(id);

  // manual registry is needed as substitution for compile time template matching
  struct registry_item
  {
    std::string id;
    TRANSPORT<STORAGE> transport;
  };
  static std::vector<registry_item> registry;

  {
    for (auto& item : registry)
      if (item.id == prepared_id)
        return item.transport;
  }

  {
    registry.push_back(registry_item{ .id = prepared_id, .transport = TRANSPORT<STORAGE>{prepared_id.c_str()} });
  }

  return registry.back().transport;
}
#define ipc_rtid_write(entry, id) create_ipc_rtid_lazy<ipc_rtid_writer, typeinfo_remove_cvref_t<decltype(entry)>>(id).write(entry)
#define ipc_rtid_read(entry, id) create_ipc_rtid_lazy<ipc_rtid_reader, typeinfo_remove_cvref_t<decltype(entry)>>(id).read(entry)


template<typename STORAGE>
void py_ipc_write(const STORAGE& entry, const char* id)
{
  ipc_rtid_write(entry, id);
}

template<typename STORAGE>
STORAGE py_ipc_read(const char* id)
{
  STORAGE entry;
  if (ipc_rtid_read(entry, id) == 0)
    return entry;
  else
    return {};
}

BOOST_PYTHON_MODULE(pytyndall)
{
  using namespace boost::python;
  def("ipc_write_float", py_ipc_write<float>);
  def("ipc_read_float", py_ipc_read<float>);
  def("ipc_write_bool", py_ipc_write<bool>);
  def("ipc_read_bool", py_ipc_read<bool>);
}

/*
>>> from pytyndall import ipc_write_float, ipc_read_float
>>> ipc_write_float(5, "my_topic")
>>> ipc_read_float("my_topic")
5.0
*/
