#include <signal.h>

#include "my_struct.h"

#include <tyndall/ipc/ipc.h>
#define debug_stdout stdout
#define debug_stderr stderr
#include <tyndall/debug.h>

int run = 1;

void sig_handler(int sig)
{
  run = 0;
}


int main()
{
  signal(SIGINT, sig_handler);

  my_struct entry = {};

  while (run)
  {
    ipc_write(entry, "/my/topic");

    my_struct_inc(entry);
    debug("sending entry:\t");
    my_struct_print(debug_plain, entry, "\n");

    usleep(1 * 1000);
  }

  return 0;
}
