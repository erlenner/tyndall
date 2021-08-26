#include <tyndall/reflect/reflect.h>
#include <string>

int main()
{

  {
    struct S
    {
      float floating_point;
      int integer;
      unsigned char unsigned_char = 5;
    };

    constexpr S s
    {
      .floating_point = 3.14f,
      .integer = 42,
      .unsigned_char = 255,
    };

    constexpr auto floating_point = reflect(s).get<0>();
    static_assert(floating_point == s.floating_point);

    constexpr auto integer = reflect(s).get<1>();
    static_assert(integer == s.integer);

    constexpr auto unsigned_char = reflect(s).get<2>();
    static_assert(unsigned_char == s.unsigned_char);

    constexpr int size = reflect(s).size();
    static_assert(size == 3);
  }

  {
    struct S
    {
      float floating_point;
      int integer;
      std::string std_string;
      int another_integer;
      unsigned int unsigned_integer;
      unsigned char unsigned_char;
      char signed_char;
    };

    S s
    {
      .floating_point = 3.14f,
      .integer = -3111696,
      .std_string = "hei",
      .another_integer = 5,
      .unsigned_integer = 3111696,
      .unsigned_char = 42,
      .signed_char = -42,
    };

    constexpr int size = reflect(s).size();
    static_assert(size == 7);
  }

  //#define PI(i, _) int M_CAT(a, i) = i;
  //#define PR(i, _) { M_RANGE(PI, 0, M_INC(i)) printf("hei %d\n", M_CAT(a, )); }
  //M_EVAL(M_RANGE(PR, 0, 3))

}
