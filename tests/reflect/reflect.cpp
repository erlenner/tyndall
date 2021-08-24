#include <tyndall/reflect/reflect.h>

#include <string>

struct S
{
  int floating_point;
  float integer;
};


struct L
{
  int floating_point;
  float integer;
  char c_string[5];
  std::string std_string;
  unsigned int unsigned_integer;
  unsigned char unsigned_char;
  char signed_char;
};

int main()
{
  S s
  {
    .floating_point = 42,
    .integer = 3.14f,
  };

  auto floating_point = reflect(s).get<0>();
  auto integer = reflect(s).get<1>();

  static_assert(std::is_same<decltype(floating_point), decltype(s.floating_point)>());
  static_assert(std::is_same<decltype(integer), decltype(s.integer)>());

  //L l
  //{
  //  .floating_point = -3111696,
  //  .integer = 3.14,
  //  .c_string = {'h', 'e', 'i', '\0'},
  //  .std_string = "hei",
  //  .unsigned_integer = 3111696,
  //  .unsigned_char = 42,
  //  .signed_char = -42,
  //};

}
