#include <iostream>
#include <cstring>

// #define ARGS_PARSER_STD_REGEX
#include "program_options.hh"

using std::cout;
using std::cerr;
using std::endl;
using namespace std::string_literals;

// TODO: req
// TODO: default_init
// TODO: make sure all tuples are forwarded so get returns the right ref type

void double_parser(const char* str, double& x) {
  x = std::atof(str) * 2;
}

template <typename F>
struct as_value {
  F f;
  operator decltype(f())() { return f(); }
};
template <typename F>
auto make_as_value(F f) { return as_value<F>{ f }; }

int main(int argc, char* argv[]) {
  double d, d2;
  int i;
  std::string s;
  bool b = false;

  std::string prog = argv[0];

  try {
    using namespace ivanp::po;
    program_options()
      (&d,'d',"Double",switch_init(0.1),double_parser)
      (&d2,"--d2","Double 2",
       // switch_init(std::tie(d))
       switch_init(make_as_value([&d]{ return 1.-d; }))
      )
      (&b,'b',"bool switch",name("bool"))
      (&i,{"-i","--int"},"Int",multi())
      (&i,"--count","Count",pos(),
        [](const char* str, int& x){ x = strlen(str); })
      (&s,std::forward_as_tuple(
            's', [](const char* arg){ return arg[0]=='s'; }),
          "starts with \'s\'",
       switch_init(prog.begin(),prog.end())
       // switch_init()
          )
      // (&c,".*\\.txt","ends with .txt",name{"regex"})
      .parse(argc,argv);
  } catch (const std::exception& e) {
    cerr <<"\033[31m"<< e.what() <<"\033[0m"<< endl;
    return 1;
  }

  TEST( d )
  TEST( d2 )
  TEST( i )
  TEST( s )
  TEST( b )

  return 0;
}
