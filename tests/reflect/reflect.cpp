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

  //{
  //  struct S
  //  {
  //    float floating_point;
  //    int integer;
  //    char c_string[5];
  //    std::string std_string;
  //    unsigned int unsigned_integer;
  //    unsigned char unsigned_char;
  //    char signed_char;
  //  };

  //  constexpr S s
  //  {
  //    .floating_point = 3.14,
  //    .integer = -3111696,
  //    .c_string = {'h', 'e', 'i', '\0'},
  //    .std_string = "hei",
  //    .unsigned_integer = 3111696,
  //    .unsigned_char = 42,
  //    .signed_char = -42,
  //  };
  //}

}
