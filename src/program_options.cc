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
    case  1: return   short_opt;
    case  2: return    long_opt;
    default: return context_opt;
  }
}

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

  opt_def *waiting = nullptr;
  const char* val = nullptr;
  std::string tmp; // use view here

  for (int i=1; i<argc; ++i) {
    const char* arg = argv[i];

    const auto opt_type = get_opt_type(arg);
    cout << arg << ' ' << opt_type << endl;

    // ==============================================================

    if (opt_type!=context_opt) {
      if (waiting) waiting->as_switch(), waiting = nullptr;
      if (opt_type==long_opt) { // long: split by '='
        if ((val = strchr(arg,'='))) arg = tmp.assign(arg,val).c_str(), ++val;
      } else { // short: allow spaceless
        if (arg[2]!='\0') val = arg+2;
      }
    }

    // ==============================================================

    if (!waiting) {
      for (auto& m : matchers[opt_type]) {
        auto *opt = m.second;
        const auto name = opt->name(); // TEST
        cout << arg << " trying: " << name << endl; // TEST
        if ((*m.first)(arg)) { // match
          cout << arg << " matched: " << name << endl; // TEST
          if (!opt->is_multi() && opt->count)
            throw error("too many options " + name);
          if (val) opt->parse(val), val = nullptr; // call parser & reset
          else waiting = opt;
          goto cont;
        }
      }
      throw po::error("unexpected option "s + arg);
      cont: ;
    } else {
      waiting->parse(arg);
      waiting = nullptr;
    }

    // TODO: if switch-only
  }
  if (waiting) {
    waiting->as_switch();
    waiting = nullptr;
  }
}

// FIXME
// void program_options::help() {
//   cout << "help" << endl;
// }

}} // end namespace ivanp
