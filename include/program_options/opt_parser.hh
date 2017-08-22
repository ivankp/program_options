#ifndef IVANP_OPT_PARSER_HH
#define IVANP_OPT_PARSER_HH

#if __has_include(<boost/lexical_cast/try_lexical_convert.hpp>)
#define PROGRAM_OPTIONS_BOOST_LEXICAL_CAST
#include <boost/lexical_cast/try_lexical_convert.hpp>
#else
#include <sstream>
#endif

#include <meta.hh>

namespace ivanp { namespace po {
namespace detail {

// ==================================================================
template <typename T>
inline std::enable_if_t<!is_assignable_v<T,const char*>,void_t<decltype(
#ifdef PROGRAM_OPTIONS_BOOST_LEXICAL_CAST
  boost::conversion::try_lexical_convert(std::declval<const char*>(),std::declval<T&>())
#else
  std::istringstream(std::declval<const char*>()) >> std::declval<T&>()
#endif
)>> arg_parser(const char* arg, T& var) {
#ifdef PROGRAM_OPTIONS_BOOST_LEXICAL_CAST
  if (!boost::conversion::try_lexical_convert(arg,var))
    throw po::error(cat(
      '\"',arg,"\" cannot be interpreted as ",type_str<T>()));
#else
  std::istringstream(arg) >> var;
#endif
}

template <typename T>
inline std::enable_if_t<is_assignable_v<T,const char*>>
arg_parser(const char* arg, T& var) noexcept(noexcept(var = arg)) { var = arg; }

// ==================================================================
template <typename T> using detect_parser = decltype(
  arg_parser(std::declval<const char*>(),std::declval<T&>()) );
template <typename T>
constexpr bool can_parse = is_detected<detect_parser,T>::value;

template <typename T> using value_type = typename T::value_type;
template <typename T>
constexpr bool has_value_type = is_detected<value_type,T>::value;

// ==================================================================
#ifdef SFINAE_EXPR
#error macro named 'SFINAE_EXPR' already defined
#endif
#define SFINAE_EXPR(EXPR) \
    [](T& var, value_type&& x) \
    noexcept(noexcept(EXPR)) -> void_t<decltype(EXPR)> { EXPR; }

template <typename T>
inline std::enable_if_t<has_value_type<T>>
arg_emplacer(const char* arg, T& var) {
  using value_type = value_type<T>;
  value_type x;

  static auto maybe_emplace = first_valid(
    SFINAE_EXPR( var.emplace_back (std::move(x)) ),
    SFINAE_EXPR( var.push_back    (std::move(x)) ),
    SFINAE_EXPR( var.emplace      (std::move(x)) ),
    SFINAE_EXPR( var.push         (std::move(x)) ),
    SFINAE_EXPR( var.emplace_front(std::move(x)) ),
    SFINAE_EXPR( var.push_front   (std::move(x)) ),
    SFINAE_EXPR( var <<            std::move(x)  )
  );

  arg_applicator(arg,x);
  const auto ret = maybe_emplace(var,std::move(x));

  static_assert( !is_nothing<std::decay_t<decltype(ret)>>::value,
    "at least one expression in maybe_emplace must be valid" );
}

#undef SFINAE_EXPR

// ==================================================================
template <typename T>
inline std::enable_if_t<can_parse<T>>
arg_applicator(const char* arg, T& var) { arg_parser(arg,var); }

template <typename T>
inline std::enable_if_t<!can_parse<T> && has_value_type<T>>
arg_applicator(const char* arg, T& var) { arg_emplacer(arg,var); };

} // end detail
}}

#endif
