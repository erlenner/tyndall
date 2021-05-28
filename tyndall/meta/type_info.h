#include <type_traits>

template<typename Type>
using type_info_get_base = typename std::remove_cv<typename std::remove_reference<Type>::type>::type;
