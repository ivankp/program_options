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

template <typename T> constexpr bool is_optional = false;
#ifdef BOOST_OPTIONAL_FLC_19NOV2002_HPP
template <typename T> constexpr bool is_optional<boost::optional<T>> = true;
#endif
#ifdef _GLIBCXX_EXPERIMENTAL_OPTIONAL
template <typename T> constexpr bool is_optional<
  std::experimental::optional<T>> = true;
#endif
#ifdef _GLIBCXX_OPTIONAL
template <typename T> constexpr bool is_optional<std::optional<T>> = true;
#endif

// 0. Assign ========================================================
template <size_t I, typename T> struct arg_parser_sfinae: std::true_type { };
template <typename T>
struct arg_parser_sfinae<0,T>: bool_constant<
  is_assignable_v<T,const char*> && !is_optional<T>
> { };

template <typename T>
inline enable_ver<arg_parser_sfinae,0,T>
arg_parser_impl(const char* arg, T& var) noexcept(noexcept(var=arg)) { var = arg; }

// 1. Emplace =======================================================
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

template <typename T, typename = void>
struct maybe_emplacable: std::false_type { };
template <typename T>
struct maybe_emplacable< T, void_t<value_type<T>> > {
  static constexpr bool value = !is_nothing<decltype(maybe_emplace(
    std::declval<T&>(), std::declval<value_type<T>&&>() ))>::value;
};

template <typename T>
struct arg_parser_sfinae<1,T>: maybe_emplacable<T> { };

template <typename T>
inline enable_ver<arg_parser_sfinae,1,T>
arg_parser_impl(const char* arg, T& var) {
  value_type<T> x;
  arg_parser(arg,x);
  maybe_emplace(var,std::move(x));
}

// 2. lexical_cast or stream ========================================
template <typename T>
inline enable_ver<arg_parser_sfinae,2,T>
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

} // end detail

template <typename T>
inline void arg_parser(const char* arg, T& var) {
  ivanp::po::detail::arg_parser_impl(arg,var);
}

}}

#endif
