#include <tyndall/meta/value_vec.h>
#include <tyndall/meta/iterate.h>
#include <tyndall/meta/mux.h>
#include <tyndall/meta/strval.h>
#include <tyndall/meta/varcb.h>
#include <tyndall/meta/types.h>
#include <tyndall/meta/typevals.h>
#include <cstdio>
#include <typeinfo>

template<typename T>
struct tstruct
{
  using Type = T;
  //typedef T Type;
  int i;
  T a;
  int ii;
};

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
  (void)sizeof(s);

  //iterate<s.size()>
  //([](auto index){
  //  constexpr auto v = s[index];
  //  printf("s: %d %d %d %d %d %d %d\n", v.a, v.b, v.c, v.d, v.e, v.f, v.g);
  //});


  constexpr auto sv = "hei"_strval;
  printf("strval: %s\n", sv.c_str());
  printf("strval from type: %s\n", decltype(sv)::c_str());
  printf("strval length: %d\n", (""_strval).length());
  printf("strval sum: %s\n", ("hei"_strval + "du"_strval).c_str());
  printf("occurrences: %d\n", ("hei din sei"_strval).occurrences('i'));
  printf("replace: %s\n", ("hei din sei"_strval).replace<'i', 'y'>().c_str());
  printf("remove_leading: %s\n", ("///hei din sei"_strval).remove_leading<'/'>().c_str());
  printf("to_strval: %s\n", to_strval<42>::c_str());


  struct SS0{ int a; float b;};
  auto cb = create_varcb([](int a){ printf("a: %d\n", a); }, [](SS0 s){ printf("s: %d, %f\n", s.a, s.b);}, [](float b){ printf("b: %f\n", b); });

  cb(5);
  cb(SS0{5,3});


  {
    printf("types:\n");
    struct TLs{ int a; float b;};
    struct S{ int a; float b;};

    static constexpr auto tm =
    types{}
    + bool{}
    + TLs{}
    + S{}
    ;
    printf("tm size: %d\n", tm.size());
    constexpr auto res = tm.get<1>();
    printf("tm res: %s\n", typeid(res).name());
    constexpr auto res2 = tm[std::integral_constant<int, 1>()];
    printf("tm res2: %s\n", typeid(res2).name());
  }

  {
    printf("typevals:\n");
    {
      struct A{int a; float b;};
      struct B{int a; float b;};
      static constexpr auto col =
      typevals{}
      + 5
      + A{4,2}
      + B{3,9}
      ;
      printf("col size: %d\n", col.size());

      constexpr auto res = col.get<2>();
      printf("col res: %s\n", typeid(res).name());
      constexpr auto res2 = col[std::integral_constant<int, 2>()];
      printf("col res2: %s\n", typeid(res2).name());
    }

    {
      static constexpr auto c =
      typevals{}
      + tstruct<int>{5}
      ;
      constexpr auto res = c.get<0>();
      printf("c res: %s\n", typeid(res).name());
      //tstruct<int>::Type t;
      decltype(res)::Type t;
      (void)sizeof(t);
    }

    {
      static constexpr auto c =
      typevals{}
      + tstruct<int>{-5}
      + tstruct<float>{3}
      + tstruct<unsigned>{8}
      ;

      iterate<c.size()>
      ([](auto index){
        //printf("index: %d\n", index());
        static_assert((index >= 0) && (index < c.size()));
        printf("index: %s\n", typeid(index).name());
      });
    }
  }

  //if(tm.match_exec(
  //  [](auto& arg, const TLs& match) -> bool
  //  {
  //    return match.a == 4;
  //  }
  //  ,
  //  [](auto& arg, const TLs& match)
  //  {
  //    printf("match: %d, %f\n", match.a, match.b);
  //    printf("type: %s\n", typeid(arg).name());
  //  }
  //) == 0)
  //  printf("tls success\n");
  //else
  //  printf("no tls match\n");

  return 0;
}
