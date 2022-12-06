#include <tyndall/log/log.h>

#include <unistd.h>
#include <signal.h>
#include <thread>

sig_atomic_t run = 1;

void cb_sig(int sig)
{
  log_info("got signal {}\n", sig);
  run = 0;
}


struct S
{
  int a;
  int b;
};

template<>
struct fmt::formatter<S>
{
  template<typename ParseContext>
  constexpr auto parse(ParseContext& ctx)
  {
    return ctx.begin();
  }

  template<typename FormatContext>
  auto format(const S& s, FormatContext& ctx)
  {
    return fmt::format_to(ctx.out(), "{}\n{}", s.a, s.b);
  }
};

//#include <iostream>
//std::ostream& operator<<(std::ostream& os, const S& rhs)
//{
//  return os << rhs.a << "\n" << rhs.b;
//}

int main()
{
  std::thread t([](){ log_debug("hei from thread\n"); });
  log_debug("hei {}\n", 4); // fmt format

  t.join();

  signal(SIGINT, cb_sig);


  while(run)
  {
    log_debug("debug statement\n");

    log_cat_debug("my_category", "CAAAAAAAAAAT\n");
    log_cat_error("my_category", "WRONG CAt {}\n", 5);

    S s = { .a = 1, .b = 2, };
    log_info("s: {}\n", s);

    usleep(100000);
  }

  return 0;
}
