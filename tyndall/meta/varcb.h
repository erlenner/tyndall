#pragma once

/* Variant callback
Like fold, except it doesn't feed the output from one function to the next.
Instead it just calls the callback function if the parameter(s) fit(s) and skips it if not.
*/

#include <utility>

template<typename First, typename... Rest>
class varcb : public First, varcb<Rest...>
{
  template<typename... Args>
  constexpr auto maybe_invoke(Args&&... args) noexcept -> decltype(First::operator()(args...))
  {
    First::operator()(args...);
  }

  constexpr void maybe_invoke(...) noexcept
  {
  }

  public:
  explicit constexpr varcb(First first, Rest... rest) noexcept
    : First(first), varcb<Rest...>(rest...)
  {}

  template<typename... Args>
  constexpr void operator() (Args&&... args) noexcept
  {
    maybe_invoke(std::forward<Args>(args)...);
    varcb<Rest...>::operator()(std::forward<Args>(args)...);
  }
};

template<typename T>
class varcb<T> : public T
{
  template<typename... Args>
  constexpr auto maybe_invoke(Args&&... args) noexcept -> decltype(T::operator()(args...))
  {
    T::operator()(args...);
  }

  constexpr void maybe_invoke(...) noexcept
  {
  }

  public:
  explicit constexpr varcb(T t) noexcept
  : T(t) {}

  template<typename... Args>
  constexpr void operator() (Args&&... args) noexcept
  {
    maybe_invoke(std::forward<Args>(args)...);
  }
};

template<typename... Cbs>
constexpr auto create_varcb(Cbs... cbs) noexcept
{
  return varcb(cbs...);
}

constexpr auto create_varcb() noexcept
{
  constexpr auto empty_lambda = [](){};

  return varcb(empty_lambda);
}


namespace // helpers for assert_param_type_in
{
  template<typename Func, typename Ret, typename Par, typename... Rest>
  Par first_arg_helper(Ret (Func::*)(Par, Rest...));

  template<typename Func, typename Ret, typename Par, typename... Rest>
  Par first_arg_helper(Ret (Func::*)(Par, Rest...) const);

  template<typename Func>
  struct first_arg
  {
    typedef decltype( first_arg_helper(&Func::operator()) ) type;
  };

  template<typename Type>
  constexpr int type_in()
  {
    return false;
  }

  template<typename Type, typename First, typename... Rest>
  constexpr int type_in()
  {
    return std::is_same<Type, First>::value || type_in<Type, Rest...>();
  }

  template<typename... Types>
  constexpr int assert_param_type_in()
  {
    return true;
  }

  template<typename... Types, typename CbFirst, typename... CbRest>
  constexpr int assert_param_type_in(CbFirst cb_first, CbRest... cb_rest)
  {
    constexpr int check = type_in<typename first_arg<CbFirst>::type, Types...>();
    static_assert(check, "param_type_in: The parameter type of CbFirst is not in the given Types (remember to match any given qualifiers, like const and &)\n");
    return check && assert_param_type_in<Types...>(cb_rest...);
  }

} // anonymous namespace

// Factory function which also checks that the first parameter type of each callback (cbs) matches one of the supplied types (AvailableTypes).
// Matching is done using std::is_same, so qualifiers like const and & etc. must also match.
template<typename... AvailableTypes, typename... Cbs>
constexpr auto create_valid_varcb(Cbs... cbs)
{
  assert_param_type_in<AvailableTypes...>(cbs...);
  return varcb(cbs...);
}


//#include <tuple> // for capture_variadic
//
//// Capture variadic args and add them as additional arguments
//// Note: This is a workaround which is not needed in c++20: https://stackoverflow.com/questions/47496358/c-lambdas-how-to-capture-variadic-parameter-pack-from-the-upper-scope
//template <typename Lambda, typename ... Args>
//auto capture_variadic(Lambda&& lambda, Args&& ... args)
//{
//  return [
//    lambda = std::forward<Lambda>(lambda),
//    capture_args = std::make_tuple(std::forward<Args>(args) ...)
//  ](auto&& ... original_args)mutable{
//    return std::apply([&lambda](auto&& ... args){
//      lambda(std::forward<decltype(args)>(args) ...);
//    }, std::tuple_cat(
//      std::forward_as_tuple(original_args ...),
//      std::apply([](auto&& ... args){
//        return std::forward_as_tuple< Args ... >(
//          std::move(args) ...);
//      }, std::move(capture_args))
//    ));
//  };
//}
