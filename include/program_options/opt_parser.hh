#ifndef IVANP_OPT_PARSER_HH
#define IVANP_OPT_PARSER_HH

#if __has_include(<boost/lexical_cast.hpp>)
#define PROGRAM_OPTIONS_BOOST_LEXICAL_CAST
#include <boost/lexical_cast.hpp>
#else
#include <sstream>
#endif

namespace ivanp { namespace po {
namespace detail {

template <typename T>
inline std::enable_if_t<!is_assignable_v<T,const char*>>
arg_parser(const char* arg, T& x) {
#ifdef PROGRAM_OPTIONS_BOOST_LEXICAL_CAST
  try {
    x = boost::lexical_cast<T>(arg);
  } catch (const boost::bad_lexical_cast&) {
    throw po::error(cat(
      '\"',arg,"\" cannot be interpreted as ",type_str<T>()));
  }
#else
  std::istringstream(arg) >> x;
#endif
}

template <typename T>
inline std::enable_if_t<is_assignable_v<T,const char*>>
arg_parser(const char* arg, T& x) noexcept(noexcept(x = arg)) { x = arg; }

}
}}

#endif
