#include <tyndal/meta/value_vec.h>
#include <cstdio>

int main()
{
  // value semantics:

  // value semantics version of push_back
  constexpr auto v0 = value_vec<int>{} + 4 + 2 + -5;
  printf("v0: ");
  for (auto v : v0)
    printf("%d ", v);
  printf("\n");

  // value semantics version of pop
  constexpr auto v1 = --v0;
  printf("v1: ");
  for (int i=0; i< sizeof(v1) / sizeof(v1[0]); ++i)
  {
    int v = v1[i];
    printf("%d ", v);
  }
  printf("\n");

  // compile time iteration:
  static constexpr auto v2 = v1 + 11;
  printf("v2: ");
  v2.iterate([](auto index)
  {
    constexpr int v = v2[index];
    static_assert(v <= 11);
    printf("%d ", v);
  });
  printf("\n");

  return 0;
}
