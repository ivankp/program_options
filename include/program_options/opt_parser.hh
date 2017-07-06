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

template <typename T> struct arg_parser {
  inline static void parse(const char* arg, T& x) {
#ifdef PROGRAM_OPTIONS_BOOST_LEXICAL_CAST
    try {
      x = boost::lexical_cast<T>(arg);
    } catch (...) {
      throw po::error(cat(
        '\"',arg,"\" cannot be interpreted as ",type_str<T>()));
    }
#else
    std::istringstream(arg) >> x;
#endif
  }
};

}
}}

#endif
