#ifndef IVANP_OPT_PARSER_HH
#define IVANP_OPT_PARSER_HH

#if __has_include(<boost/lexical_cast/try_lexical_convert.hpp>)
#define PROGRAM_OPTIONS_BOOST_LEXICAL_CAST
#include <boost/lexical_cast/try_lexical_convert.hpp>
#else
#include <sstream>
#endif

#include "maybe_valid.hh"

template <typename...> struct bad_type;

namespace ivanp { namespace po {

template <typename T>
inline void arg_parser(const char* arg, T& var);

namespace detail {

// 0. Assignable ====================================================
template <size_t I, typename T> struct arg_parser_sfinae: std::true_type { };
template <typename T>
struct arg_parser_sfinae<0,T>: is_assignable<T,const char*> { };

template <typename T>
inline enable_ver<arg_parser_sfinae,0,T>
arg_parser_impl(const char* arg, T& var) noexcept(noexcept(var=arg)) { var = arg; }

// 1. Container =====================================================
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

// bad_type< bool_constant<arg_parser_sfinae<0,const char*>::value> > bad0;
// bad_type< bool_constant<arg_parser_sfinae<0,double>::value> > bad0;
// bad_type< bool_constant<only_last<arg_parser_sfinae<0,const char*>>::value> > bad0;
// bad_type< enable_ver_seq<arg_parser_sfinae,0,const char*> > bad0;
// bad_type< enable_ver<arg_parser_sfinae,0,const char*> > bad0;
// bad_type< enable_ver<arg_parser_sfinae,1,std::vector<double>> > bad1;
// bad_type< enable_ver<arg_parser_sfinae,2,double> > bad2;

// 2. Lexical cast or stream ========================================
// template <typename T, typename CharT=char> using detect_istream = decltype(
//   std::declval<std::basic_istream<CharT>&>() >> std::declval<T&>() );
// template <typename T>
// constexpr bool can_istream =
//   is_detected<detect_istream,T,char>::value ||
//   is_detected<detect_istream,T,wchar_t>::value;

// template <typename T>
// struct arg_parser_sfinae<2,T>: std::true_type { };
// struct arg_parser_sfinae<2,T>: bool_constant< can_istream<T> > { };

// NOTE: try_lexical_convert fails on static_assert rather than sfinae
// can_istream<T> may not be a perfect test for it's validity
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
