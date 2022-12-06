#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

#include <tyndall/ipc/ipc.h>
#define debug_stdout stdout
#define debug_stderr stderr
#include <tyndall/debug.h>

#define len(array) (int)(sizeof(array) / sizeof(array[0]))

typedef struct
{
  const char * const name;
  char* args[10]; // increase as needed
  int respawn;  // respawn on error: -1 = infinite , 0 = never, 1 = once, 2 = twice, etc.

// internal
  pid_t pid;
  int alive;

} child;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings" // ref https://gcc.gnu.org/bugzilla/show_bug.cgi?id=94554

child children[] =
{
//==================== FILL IN PROCESSES ====================
  {
    .name = "tyndall_ex_ipc_writer",
    .args = {},
    .respawn = -1,
  },
  {
    .name = "tyndall_ex_ipc_reader",
    .args = { "--my_option", "--my_other_option", },
  },
//===========================================================
};
#define n_children len(children)

#pragma GCC diagnostic pop

//==================== STATUSES THAT BYPASS RESPAWN =========
// Children that are configured to respawn will not be respawned if they return with these statuses:
int respawn_bypass_statuses[] = { 0, SIGINT };
//===========================================================

int fork_child(child *c)
{
  pid_t pid = fork();

  debug_assert(pid != -1, return 1);

  if (pid == 0)
  {
    debug("child %u (%s) was spawned by parent %u\n", getpid(), c->name, getppid());

    setpgid(0, 0); // switch process group so ctrl-c only interrupts god

    // fill in argv
    char * child_argv[len(c->args) + 2];
    child_argv[0] = (char*)(c->name);
    for (int i=0; i < len(c->args); ++i)
      child_argv[i+1] = c->args[i];
    child_argv[len(c->args) + 1] = NULL;

    int rc = execvp(c->name, child_argv);
    debug_assert(rc != -1);
  }

  c->pid = pid;
  c->alive = 1;
  debug("parent %u spawned child %u (%s)\n", getpid(), c->pid, c->name);

  return 0;
}

void exit_handler(int sig)
{
  debug("got signal %d\n", sig);

  // kill the children manually, since they're in a different group
  signal(SIGCHLD, SIG_IGN);
  for (int i=0; i < n_children; ++i)
  {
    if (children[i].alive)
    {
      debug("killing %u (%s)\n", children[i].pid, children[i].name);
      debug_assert(kill(children[i].pid, sig) == 0);
    }
  }

  ipc_cleanup();

  debug("exiting %u (god process) with status %d\n", getpid(), sig);

  exit(sig);
}

void child_handler(int sig)
{
  pid_t pid = -1;
  int status;

  for (pid_t tmp_pid; (tmp_pid = waitpid(-1, &status, WNOHANG)) > 0;)
    pid = tmp_pid;

  int child_index = -1;
  for (int i = 0; i < n_children; ++i)
    if (children[i].pid == pid)
      child_index = i;

  debug_assert(child_index != -1, return);
  child *c = children + child_index;

  c->alive = 0;
  debug("child %u (%s) exited with status %d\n", c->pid, c->name, status);

  if ((c->respawn != 0))
  {
    int bypass_statuses = 0;
    for (int i=0; i < len(respawn_bypass_statuses); ++i)
      if (status == respawn_bypass_statuses[i])
        ++bypass_statuses;

    if (bypass_statuses)
      debug("respawn bypassed\n");
    else
    {
      debug("respawning child\n");
      fork_child(c);

      --c->respawn;
    }
  }
}

int main()
{
  debug("starting %u (god process)\n", getpid());

  for (int i=0; i < n_children; ++i)
    fork_child(children + i);

  debug("%u (god process) finished spawning threads\n", getpid());

  signal(SIGINT, exit_handler);
  signal(SIGCHLD, child_handler);

  while(1)
    pause();

  return 0;
}
