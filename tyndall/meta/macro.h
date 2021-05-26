// macro tutorial: http://saadahmad.ca/cc-preprocessor-metaprogramming-basic-pattern-matching-macros-and-conditionals/

#define M_EMPTY() 
#define M_DEFER(...) __VA_ARGS__ M_EMPTY()
#define M_DEFER2(...) __VA_ARGS__ M_DEFER(EMPTY) ()
#define M_DEFER3(...) __VA_ARGS__ M_DEFER2(EMPTY) ()
#define M_DEFER4(...) __VA_ARGS__ M_DEFER3(EMPTY) ()
#define M_DEFER5(...) __VA_ARGS__ M_DEFER4(EMPTY) ()

#define M_EVAL_1(...) __VA_ARGS__
#define M_EVAL_2(...) M_EVAL_1(M_EVAL_1(__VA_ARGS__))
#define M_EVAL_3(...) M_EVAL_2(M_EVAL_2(__VA_ARGS__))
#define M_EVAL_4(...) M_EVAL_3(M_EVAL_3(__VA_ARGS__))
#define M_EVAL_5(...) M_EVAL_4(M_EVAL_4(__VA_ARGS__))
#define M_EVAL_6(...) M_EVAL_5(M_EVAL_5(__VA_ARGS__))
#define M_EVAL_7(...) M_EVAL_6(M_EVAL_6(__VA_ARGS__))
#define M_EVAL_8(...) M_EVAL_7(M_EVAL_7(__VA_ARGS__))
#define M_EVAL(...) M_EVAL_2(__VA_ARGS__)

#define M_EAT(...)
 
#define M_ENCLOSE_EXPAND(...) EXPANDED, ENCLOSED, (__VA_ARGS__) ) M_EAT (
#define M_GET_CAT_EXP(a, b) (a, ENCLOSE_EXPAND b, DEFAULT, b )

#define M_CAT_WITH_ENCLOSED(a, b) a b
#define M_CAT_WITH_DEFAULT(a, b) a ## b
#define M_CAT_WITH(a, _, f, b) M_CAT_WITH_ ## f (a, b)

#define M_EVAL_CAT_WITH(...) M_CAT_WITH __VA_ARGS__
#define M_CAT(a, b) M_EVAL_CAT_WITH ( M_GET_CAT_EXP(a, b) )

#define M_HEAD(x, ...) x
#define M_HEAD1(x, y, ...) y
#define M_HEAD2(x, y, z, ...) z

#define M_TAIL(x, ...) __VA_ARGS__
#define M_TAIL1(x, y, ...) __VA_ARGS__
#define M_TAIL2(x, y, z, ...) __VA_ARGS__
 
#define _M_STRINGIFY(x) #x
#define M_STRINGIFY(x) _M_STRINGIFY(x)

#define M_COMMA ,

#define M_LEN(array) (sizeof(array) / sizeof(array[0]))
