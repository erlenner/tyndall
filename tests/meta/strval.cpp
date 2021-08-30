#include <tyndall/meta/strval.h>


int main()
{
  constexpr auto hei = "hei"_strval;
  decltype(hei)::c_str();
  static_assert(hei.get<0>() == 'h');
  static_assert(hei.get<1>() == 'e');
  static_assert(hei.get<2>() == 'i');
  static_assert(hei.length() == 3);
  static_assert(hei + "du"_strval == "heidu"_strval);
  static_assert(("hei din sei"_strval).occurrences('i') == 3);
  static_assert(("a_b_c"_strval).replace<'_', '-'>() == "a-b-c"_strval);
  static_assert(("///hei"_strval).remove_leading<'/'>() == hei);
  static_assert(to_strval<42>{} == "42"_strval);

}
