#ifndef IVANP_OPT_PARSER_HH
#define IVANP_OPT_PARSER_HH

#if __has_include(<boost/lexical_cast/try_lexical_convert.hpp>)
#define PROGRAM_OPTIONS_BOOST_LEXICAL_CAST
#include <boost/lexical_cast/try_lexical_convert.hpp>
#else
#include <sstream>
#endif

#include "maybe_valid.hh"

namespace ivanp { namespace po {

template <typename T>
inline void arg_parser(const char* arg, T& var);

namespace detail {

// 0. Assign ========================================================
template <size_t I, typename T> struct arg_parser_switch: std::true_type { };
template <typename T>
struct arg_parser_switch<0,T>: conjunction<
  is_assignable<T,const char*>,
  negation<value_type_trait<std::is_same,T,bool>>
> { };

template <typename T>
inline enable_case<arg_parser_switch,0,T>
arg_parser_impl(const char* arg, T& var) { var = arg;
  std::cout << __PRETTY_FUNCTION__ << std::endl;
}

// Emplace 1. directly; 2. value_type ===============================
#ifdef EMPLACE_TEST
#error macro named 'EMPLACE_TEST' already defined
#endif
#define EMPLACE_TEST(EXPR) SFINAE_EXPR(EXPR, auto& var, auto&& x)

auto maybe_emplace = first_valid(
  EMPLACE_TEST( var.emplace_back (std::move(x)) ),
  EMPLACE_TEST( var.push_back    (std::move(x)) ),
  EMPLACE_TEST( var.emplace      (std::move(x)) ),
  EMPLACE_TEST( var.push         (std::move(x)) ),
  EMPLACE_TEST( var.emplace_front(std::move(x)) ),
  EMPLACE_TEST( var.push_front   (std::move(x)) )
);

#undef EMPLACE_TEST

template <typename Var, typename X>
constexpr bool can_emplace = !is_nothing<decltype(maybe_emplace(
  std::declval<Var&>(), std::declval<X>() ))>::value;

// ------------------------------------------------------------------

template <typename T>
struct arg_parser_switch<1,T>: conjunction<
  value_type_trait<is_constructible,T,const char*>,
  negation<value_type_trait<std::is_same,T,bool>>
> { };

template <typename T>
inline enable_case<arg_parser_switch,1,T>
arg_parser_impl(const char* arg, T& var) { maybe_emplace(var,arg); }

// ------------------------------------------------------------------

template <typename T, typename = void>
struct can_emplace_value_type: std::false_type { };
template <typename T>
struct can_emplace_value_type< T, void_t<value_type<T>> >: bool_constant<
  can_emplace<T,value_type<T>&&> > { };

template <typename T>
struct arg_parser_switch<2,T>: can_emplace_value_type<T> { };

template <typename T>
inline enable_case<arg_parser_switch,2,T>
arg_parser_impl(const char* arg, T& var) {
  value_type<T> x;
  arg_parser(arg,x);
  maybe_emplace(var,std::move(x));
}

// 3. lexical_cast or stream ========================================
template <typename T>
inline enable_case<arg_parser_switch,3,T>
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
  throw po::error(cat('\"',arg,"\" cannot be interpreted as ",type_str<T>()));
#else
  std::istringstream(arg) >> var;
#endif
}

// Explicit =========================================================
template <> void arg_parser_impl<bool>(const char* arg, bool& var);

} // end detail

template <typename T>
inline void arg_parser(const char* arg, T& var) {
  ivanp::po::detail::arg_parser_impl(arg,var);
}

}}

#endif
