#!/usr/bin/env python
from pytyndall import ipc_write_float, ipc_read_float
from time import sleep

rc = ipc_write_float(42, "/test/topic")
assert rc == 0, "write failed"

f = ipc_read_float("/test/topic")
assert f == 42, "got different value back"
