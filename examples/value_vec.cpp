#include <tyndall/meta/value_vec.h>
#include <tyndall/meta/iterate.h>
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
  for (unsigned i=0; i< sizeof(v1) / sizeof(v1[0]); ++i)
  {
    int v = v1[i];
    printf("%d ", v);
  }
  printf("\n");

  // compile time iteration:
  static constexpr auto v2 = v1 + 11;
  printf("v2: ");
  iterate<v2.size()>
  ([](auto index){
    constexpr int v = v2[index];
    static_assert(v <= 11);
    printf("%d ", v);
  });
  printf("\n");


  struct S0
  {
    int a = 1;
    unsigned b = 2;
  };

  struct S1
  {
    short c = 3;
    char d = 4;
    char e = 5;
  };

  constexpr value_vec<S0,5> s0;
  constexpr value_vec<S1,5> s1;

  static constexpr auto s = mux(s0, s1);

  iterate<s.size()>
  ([](auto index){
    constexpr auto v = s[index];
    printf("s: %d %d %d %d %d\n", v.a.a, v.a.b, v.b.c, v.b.d, v.b.e);
  });

  return 0;
}
