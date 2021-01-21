#include <tyndall/meta/value_vec.h>
#include <tyndall/meta/iterate.h>
#include <tyndall/meta/mux.h>
#include <tyndall/meta/strval.h>
#include <tyndall/meta/varcb.h>
#include <cstdio>

int main()
{
  printf("value_vec:\n");

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

  printf("\niteration:\n");
  static constexpr auto v2 = v1 + 11;
  printf("v2: ");
  iterate<v2.size()>
  ([](auto index){
    constexpr int v = v2[index];
    static_assert(v <= 11);
    printf("%d ", v);
  });
  printf("\n");


  printf("\nmux:\n");
  struct S0
  {
    int a = 1;
    unsigned b = 2;
  };
  constexpr value_vec<S0,5> s0;

  struct S1
  {
    short c = 3;
    char d = 4;
    char e = 5;
    //int b = 6; // error: dont use same member names as in S0 since they will be ambiguous
  };
  constexpr value_vec<S1,5> s1;

  struct S2
  {
    long f;
    short g;
  };
  constexpr value_vec<S2,5> s2{{6,7}};

  static constexpr auto s = mux(s0, s1, s2);

  iterate<s.size()>
  ([](auto index){
    constexpr auto v = s[index];
    printf("s: %d %d %d %d %d %d %d\n", v.a, v.b, v.c, v.d, v.e, v.f, v.g);
  });


  constexpr auto sv = "hei"_strval;
  printf("strval: %s\n", sv.c_str());
  printf("strval sum: %s\n", ("hei "_strval + "du"_strval).c_str());


  struct SS0{ int a; float b;};
  auto cb = create_varcb([](int a){ printf("a: %d\n", a); }, [](SS0 s){ printf("s: %d, %f\n", s.a, s.b);}, [](float b){ printf("b: %f\n", b); });

  cb(5);
  cb(SS0{5,3});

  return 0;
}
