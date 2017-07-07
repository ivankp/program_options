#include <iostream>
#include <cstring>
#include <stdexcept>

#include "program_options.hh"
// TODO: forward declarations instead of full header?
// TODO: allow negative integers

using std::cout;
using std::cerr;
using std::endl;
using namespace std::string_literals;

bool is_number(const char* str) noexcept {
// https://stackoverflow.com/q/4654636/2640636
#ifdef PROGRAM_OPTIONS_BOOST_LEXICAL_CAST
  try {
    boost::lexical_cast<double>(str);
    return true;
  } catch (const boost::bad_lexical_cast&) { return false; }
#else
  char* p;
  std::strtod(str,&p);
  return !*p;
#endif
}

namespace ivanp { namespace po {

namespace detail {

template <>
bool opt_match<const char*>::operator()(const char* arg) const noexcept {
  int i = 0;
  for (; m[i]!='\0' && arg[i]!='\0'; ++i)
    if ( arg[i]!=m[i] ) return false;
  return m[i]=='\0' && arg[i]=='\0';
}

opt_type get_opt_type(const char* arg) noexcept {
  unsigned char n = 0;
  for (char c=arg[n]; c=='-'; c=arg[++n]) ;
  switch (n) {
    case  1:
      if (is_number(arg)) return context_opt;
      return   short_opt;
    case  2: return    long_opt;
    default: return context_opt;
  }
}

}

void check_count(detail::opt_def* opt) {
  if (!opt->is_multi() && opt->count)
    throw error("too many options " + opt->name());
}

void program_options::parse(int argc, char const * const * argv) {
  using namespace ::ivanp::po::detail;
  // for (int i=1; i<argc; ++i) {
  //   for (const auto& m : help_matchers) {
  //     if ((*m)(argv[i])) {
  //       help();
  //       return;
  //     }
  //   }
  // }

  opt_def *opt = nullptr;
  const char* val = nullptr;
  std::string tmp; // use view here

  for (int i=1; i<argc; ++i) {
    const char* arg = argv[i];

    const auto opt_type = get_opt_type(arg);
    cout << arg << ' ' << opt_type << endl;

    // ==============================================================

    if (opt_type!=context_opt) {
      if (opt) {
        if (!opt->count) opt->as_switch();
        opt = nullptr;
      }
      if (opt_type==long_opt) { // long: split by '='
        if ((val = strchr(arg,'='))) arg = tmp.assign(arg,val).c_str(), ++val;
      } else { // short: allow spaceless
        if (arg[2]!='\0') val = arg+2;
      }
    }

    // ==============================================================

    if (opt) {
      cout << arg << " arg of: " << opt->name() << endl; // TEST
      opt->parse(arg);
      if (!opt->is_multi()) opt = nullptr;
      continue;
    }

    for (auto& m : matchers[opt_type]) {
      if ((*m.first)(arg)) { // match
        opt = m.second;
        cout << arg << " matched: " << opt->name() << endl; // TEST
        check_count(opt);
        if (opt_type==context_opt) val = arg;
        if (opt->is_switch()) {
          if (val) throw error(
            "switch " + opt->name() + " does not take arguments");
          opt->as_switch(), opt = nullptr;
        } else if (val) {
          opt->parse(val), val = nullptr;
          if (!opt->is_multi()) opt = nullptr;
        }
        goto next_arg;
      }
    }

    // handle positional options
    if (opt_type==context_opt && !opt && pos.size()) {
      auto *pos_opt = pos.front();
      check_count(pos_opt);
      cout << arg << " pos: " << pos_opt->name() << endl; // TEST
      pos_opt->parse(arg);
      if (!pos_opt->is_pos_end()) pos.pop();
      continue;
    }

    throw po::error("unexpected option "s + arg);
    next_arg: ;
  } // end arg loop
  if (opt) {
    if (!opt->count) opt->as_switch();
    opt = nullptr;
  }
}

// FIXME
// void program_options::help() {
//   cout << "help" << endl;
// }

}} // end namespace ivanp
