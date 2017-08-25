#include <iostream>
#include <cstring>
#include <vector>

#include <boost/optional.hpp>

// #define PROGRAM_OPTIONS_STD_REGEX
#define PROGRAM_OPTIONS_ALLOW_INT_AS_FLOAT
#include "program_options.hh"

using std::cout;
using std::cerr;
using std::endl;
using namespace std::string_literals;

// TODO: make sure all tuples are forwarded so get returns the right ref type

void double_parser(const char* str, double& x) {
  x = std::atof(str) * 2;
}

template <typename F> auto as_value(F f) {
  return ( struct { F f; operator decltype(f())() { return f(); } } ){ f };
}

namespace ivanp { namespace po {
template <>
inline void arg_parser<std::string>(const char* arg, std::string& var) {
  var = arg;
  for (unsigned i=0; var[i]!='\0'; ++i)
    if (var[i]=='s') var[i] = '*';
}
}}

int main(int argc, char* argv[]) {
  cout << std::boolalpha;
  double d = 0, d2;
  std::vector<int> i;
  std::string s;
  // boost::optional<std::string> s;
  // const char* s;
  boost::optional<bool> b;

  try {
    using namespace ivanp::po;
    program_options()
      (&d,'d',"Double",switch_init(0.1),double_parser)
      (&d2,"--d2","1-d",
       // switch_init(std::tie(d))
       default_init(as_value([&d]{ return 1.-d; }))
      )
      (&b,'b',"bool switch",name("bool"))
      (&i,{"-i","--int"},"Int",multi())
      (&i,"--count","Count",pos(),
        [](const char* str, decltype(i)& x){ x.push_back(strlen(str)); })
      (&s,std::forward_as_tuple(
            's', [](const char* arg){ return arg[0]=='s'; }),
          "starts with \'s\'"/*, req()*/)
      // (&s,".*\\.txt","ends with .txt")
      .parse(argc,argv);
  } catch (const std::exception& e) {
    cerr <<"\033[31m"<< e.what() <<"\033[0m"<< endl;
    return 1;
  }

  TEST( d )
  TEST( d2 )
  cout << "i:";
  for (int i : i) cout << ' ' << i;
  cout << endl;
  // TEST( i )
  // if (s) cout << "s = " << *s << endl;
  // else cout << "s undefined" << endl;
  TEST( s )
  if (b) cout << "b = " << *b << endl;
  else cout << "b undefined" << endl;
  // TEST( b )

  return 0;
}
