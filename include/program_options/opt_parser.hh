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
namespace detail {

template <typename T, typename CharT=char> using detect_istream = decltype(
  std::declval<std::basic_istream<CharT>&>() >> std::declval<T&>() );
template <typename T>
constexpr bool can_istream =
  is_detected<detect_istream,T,char>::value ||
  is_detected<detect_istream,T,wchar_t>::value;

// ==================================================================
template <typename T>
inline std::enable_if_t<is_assignable_v<T,const char*>>
arg_parser(const char* arg, T& var) noexcept(noexcept(var=arg)) { var = arg; }

// NOTE: try_lexical_convert fails on static_assert rather than sfinae
// can_istream<T> may not be a perfect test for it's validity
template <typename T>
inline std::enable_if_t<!is_assignable_v<T,const char*> && can_istream<T>>
arg_parser(const char* arg, T& var) {
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
      if (d-double(var)!=0.) {
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

// ==================================================================
template <typename T> using parser_void_t = decltype(
  arg_parser(std::declval<const char*>(),std::declval<T&>()) );
template <typename T>
constexpr bool can_parse = is_detected<parser_void_t,T>::value;

// ==================================================================
template <typename T>
inline std::enable_if_t<has_value_type<T>>
arg_emplacer(const char* arg, T& var); // FWD

template <typename T>
inline parser_void_t<T>
arg_applicator(const char* arg, T& var) { arg_parser(arg,var); }

template <typename T>
inline std::enable_if_t<!can_parse<T> && has_value_type<T>>
arg_applicator(const char* arg, T& var) { arg_emplacer(arg,var); };

#ifdef EMPLACE_TEST
#error macro named 'EMPLACE_TEST' already defined
#endif
#define EMPLACE_TEST(EXPR) SFINAE_EXPR(EXPR, auto& var, auto&& x)

template <typename T>
inline std::enable_if_t<has_value_type<T>>
arg_emplacer(const char* arg, T& var) {
  using value_type = value_type<T>;
  value_type x;

  static auto maybe_emplace = first_valid(
    EMPLACE_TEST( var.emplace_back (std::move(x)) ),
    EMPLACE_TEST( var.push_back    (std::move(x)) ),
    EMPLACE_TEST( var.emplace      (std::move(x)) ),
    EMPLACE_TEST( var.push         (std::move(x)) ),
    EMPLACE_TEST( var.emplace_front(std::move(x)) ),
    EMPLACE_TEST( var.push_front   (std::move(x)) )
  );

  arg_applicator(arg,x);
  const auto ret = maybe_emplace(var,std::move(x));

  static_assert( !is_nothing<std::decay_t<decltype(ret)>>::value,
    "at least one expression in maybe_emplace must be valid" );
}

#undef EMPLACE_TEST

// ==================================================================

} // end detail
}}

#endif
