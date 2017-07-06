#include <iostream>
#include <cstring>
#include <stdexcept>

#include "program_options.hh"

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

void parser::parse(int argc, char const * const * argv) {
  using namespace ::ivanp::po::detail;
  // for (int i=1; i<argc; ++i) {
  //   for (const auto& m : help_matchers) {
  //     if ((*m)(argv[i])) {
  //       help();
  //       return;
  //     }
  //   }
  // }

  opt_def_base *waiting = nullptr;
  const char* str = nullptr;
  std::string tmp;

  for (int i=1; i<argc; ++i) {
    const char* arg = argv[i];

    const auto opt_type = get_opt_type(arg);
    cout << arg << ' ' << opt_type << endl;

    // ==============================================================
    if (opt_type!=context_opt) {
      if (waiting && waiting->need)
        throw po::error(waiting->name() + " without value");
    }

    switch (opt_type) {
      case long_opt: // ---------------------------------------------

        str = strchr(arg,'='); // split by '=' if long
        if (str) tmp.assign(arg,str), ++str, arg = tmp.c_str();

        break;
      case short_opt: // --------------------------------------------
        if (arg[2]!='\0') str = arg+2;

        break;
      case context_opt: // ------------------------------------------
        if (!waiting) str = arg;

        break;
    }

    // TODO

    // ./test/test --int 55 -d1.754564e3 test
    // "test" cannot be interpreted as int

    // ./test/test --int 55 -d 1.75456
    // "1.75456" cannot be interpreted as int

    // ==============================================================

    if (opt_type!=context_opt || !waiting) {
      for (auto& m : matchers[opt_type]) {
        auto& def = m.second;
        const auto name = def->name();
        cout << "trying: " << name << endl;
        if ((*m.first)(arg)) {
          cout << arg << " matched: " << name << endl;
          if (def->count==0) throw error("excessive option " + name);
          if (str) def->parse(str), str = nullptr; // call parser & reset
          else if (def->is_switch()) { }
          else waiting = def;
          goto cont;
        }
      }
    }

    if (waiting && waiting->count) {
      waiting->parse(arg);
      if (!waiting->count) waiting = nullptr;
      goto cont;
    }

    throw po::error("unexpected option "s + arg);
    cont: ;
  }
}

// FIXME
// void parser::help() {
//   cout << "help" << endl;
// }

}} // end namespace ivanp
