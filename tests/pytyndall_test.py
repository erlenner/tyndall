#!/usr/bin/env python
from pytyndall import ipc_write_float, ipc_read_float
from time import sleep

rc = ipc_write_float(42, "/py/example")
assert rc == 0, "write failed"

f = ipc_read_float("/py/example")
assert f == 42, "got different value back"
