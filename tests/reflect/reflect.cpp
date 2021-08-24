#include <tyndall/reflect/reflect.h>

#include <type_traits>
#include <string>

int main()
{

  {
    struct S
    {
      int floating_point;
      float integer;
    };

    constexpr S s
    {
      .floating_point = 42,
      .integer = 3.14f,
    };

    constexpr auto floating_point = reflect(s).get<0>();
    static_assert(floating_point == s.floating_point);

    constexpr auto integer = reflect(s).get<1>();
    static_assert(integer == s.integer);

    constexpr int size = reflect(s).size();
    static_assert(size == 2);
  }

  //{
  //  struct S
  //  {
  //    int floating_point;
  //    float integer;
  //    char c_string[5];
  //    std::string std_string;
  //    unsigned int unsigned_integer;
  //    unsigned char unsigned_char;
  //    char signed_char;
  //  };

  //  constexpr S s
  //  {
  //    .floating_point = -3111696,
  //    .integer = 3.14,
  //    .c_string = {'h', 'e', 'i', '\0'},
  //    .std_string = "hei",
  //    .unsigned_integer = 3111696,
  //    .unsigned_char = 42,
  //    .signed_char = -42,
  //  };
  //}

}
