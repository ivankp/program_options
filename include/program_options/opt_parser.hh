#ifndef IVANP_OPT_PARSER_HH
#define IVANP_OPT_PARSER_HH

#include "program_options/fwd/opt_parser.hh"
#include "maybe_valid.hh"

namespace ivanp { namespace po {

template <typename T>
inline void arg_parser(const char* arg, T& var);

namespace detail {

template <typename T>
using value_type_not_bool = negation< maybe_is<
  bind_tail<std::is_same,bool>::template type,
  m_value_type<T> > >;

// 0. Assign ========================================================
template <size_t I, typename T> struct arg_parser_switch: std::true_type { };
template <typename T>
struct arg_parser_switch<0,T>: conjunction<
  is_assignable<T,const char*>,
  value_type_not_bool<T>
> { };

template <typename T>
inline enable_case<arg_parser_switch,0,T>
arg_parser_impl(const char* arg, T& var) { var = arg; }

// Emplace ==========================================================
#ifdef EMPLACE_EXPR
#error macro named 'EMPLACE_EXPR' already defined
#endif
#define EMPLACE_EXPR(EXPR) SFINAE_EXPR(EXPR, auto& var, auto&& x)

auto maybe_emplace = first_valid(
  EMPLACE_EXPR( var.emplace_back (std::move(x)) ),
  EMPLACE_EXPR( var.push_back    (std::move(x)) ),
  EMPLACE_EXPR( var.emplace      (std::move(x)) ),
  EMPLACE_EXPR( var.push         (std::move(x)) ),
  EMPLACE_EXPR( var.emplace_front(std::move(x)) ),
  EMPLACE_EXPR( var.push_front   (std::move(x)) )
);

#undef EMPLACE_EXPR

template <typename Var, typename X>
using can_emplace = is_just_value<decltype(maybe_emplace(
  std::declval<Var&>(), std::declval<X&&>() ))>;

// 1. Emplace const char* ===========================================
template <typename T>
struct arg_parser_switch<1,T>: conjunction<
  maybe_is<
    bind_tail<is_constructible,const char*>::template type,
    m_value_type<T> >,
  value_type_not_bool<T>
> { };

template <typename T>
inline enable_case<arg_parser_switch,1,T>
arg_parser_impl(const char* arg, T& var) { maybe_emplace(var,arg); }

// 2. Emplace value_type ============================================
template <typename T>
struct arg_parser_switch<2,T>: maybe_is<
  bind_first<can_emplace,T>::template type,
  m_value_type<T> > { };

template <typename T>
inline enable_case<arg_parser_switch,2,T>
arg_parser_impl(const char* arg, T& var) {
  typename T::value_type x; // TODO: deal with const
  arg_parser(arg,x);
  maybe_emplace(var,std::move(x));
}

// 3. pair, array, tuple ============================================
template <typename T>
using tuple_size_t = decltype(std::tuple_size<std::decay_t<T>>::value);

template <typename T>
struct arg_parser_switch<3,T>: is_detected<tuple_size_t,T> { };

template <size_t I, typename T>
inline std::enable_if_t<(I==std::tuple_size<T>::value)>
parse_elem(const char*, const T&) noexcept {
  static_assert(I>0,"use of type with tuple_size==0 in program options");
}

template <size_t I, typename T>
inline std::enable_if_t<(I+1==std::tuple_size<T>::value)>
parse_elem(const char* arg, T& tup) { arg_parser(arg,std::get<I>(tup)); }

template <size_t I, typename T>
inline std::enable_if_t<(I+1<std::tuple_size<T>::value)>
parse_elem(const char* arg, T& tup) {
  int n = 0;
  while (arg[n]!=':' && arg[n]!='\0') ++n;
  arg_parser(std::string(arg,n).c_str(),std::get<I>(tup));
  if (arg[n]!='\0') parse_elem<I+1>(arg+n+1,tup);
}

template <typename T>
inline enable_case<arg_parser_switch,3,T>
arg_parser_impl(const char* arg, T& var) { parse_elem<0>(arg,var); }

// 4. lexical_cast or stream ========================================
template <typename T>
inline enable_case<arg_parser_switch,4,T>
arg_parser_impl(const char* arg, T& var) {
#ifdef PROGRAM_OPTIONS_BOOST_LEXICAL_CAST
  if (boost::conversion::try_lexical_convert(arg,var)) return;
#ifdef PROGRAM_OPTIONS_ALLOW_INT_AS_FLOAT
  else if
#ifdef __cpp_if_constexpr
    constexpr
#endif
  (std::is_integral<T>::value) {
    double d;
    if (boost::conversion::try_lexical_convert(arg,d)) {
      var = d;
      if (d != double(var)) {
        std::cerr << "\033[33mWarning\033[0m: "
          "lossy conversion from double to " << type_str<T>()
          << " in program option argument ("
          << d << " to " << var << ')'
          << std::endl;
      }
      return;
    }
  }
#endif
  throw po::error('\"',arg,"\" cannot be interpreted as ",type_str<T>());
#else
  std::istringstream(arg) >> var;
#endif
}

// Explicit =========================================================
template <>
inline void arg_parser_impl<bool>(const char* arg, bool& var) {
  arg_parser_impl_bool(arg,var);
}

} // end detail

template <typename T>
inline void arg_parser(const char* arg, T& var) {
  ivanp::po::detail::arg_parser_impl(arg,var);
}

}}

#endif
