#include <tyndall/meta/macro.h>


int main()
{
  #define TEST(i, _) static_assert((0 <= i) && (i < 8));

  M_EVAL(M_RANGE(TEST, 0, 8))
}
