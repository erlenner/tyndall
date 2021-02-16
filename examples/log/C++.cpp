//#define LOG_PRINTF // printf format
#define LOG_FMT // fmtlib format
#include <tyndall/log/log.h>

#include <unistd.h>
#include <signal.h>

sig_atomic_t run = 1;

void cb_sig(int sig)
{
  log_info("got signal %d\n", sig);
  run = 0;
}


struct S
{
  int a;
  int b;
};

#include <iostream>
std::ostream& operator<<(std::ostream& os, const S& rhs)
{
  return os << rhs.a << "\n" << rhs.b;
}

int main()
{

  log_init({.file_path = "out.log"});

  log_debug("hei %d\n", 3); // printf format
  log_debug("hei {}\n", 4); // fmt format

  {
    FILE* fp = fopen("/tmp/asdlkjfslkdjalfjdslrdflkjnnnnnjkklsdjjjfdslkmnkl", "rb");
    if (fp == NULL)
      log_errno("couldn't open the file\n");
  }

  signal(SIGINT, cb_sig);


  while(run)
  {
    log_debug("debug statement\n");

    log_cat_debug("my_category", "CAAAAAAAAAAT\n");
    log_cat_once_error("my_category", "WRONG CAt %d\n", 5);

    log_once_error("error statement printed once\n");

    S s = { .a = 1, .b = 2, };
    log_once_info("s: {}\n", s);

    usleep(100000);
  }
}
