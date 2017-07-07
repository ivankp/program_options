#include <iostream>
#include <cstring>

// #define ARGS_PARSER_STD_REGEX
#include "program_options.hh"

using std::cout;
using std::cerr;
using std::endl;
using namespace std::string_literals;

// TODO: allow function pointers as parsers

void double_parser(const char* str, double& x) {
  x = std::atof(str) * 2;
}

int main(int argc, char* argv[]) {
  double d;
  int i;
  std::string s;
  bool b = false;

  try {
    using namespace ivanp::po;
    program_options()
      (&d,'d',"Double",switch_init(4.2))
      (&b,'b',"bool switch",name("bool"))
      (&i,{"-i","--int"},"Int",multi())
      (&i,"--count","Count",pos(),
        [](const char* str, int& x){ x = strlen(str); })
      (&s,std::forward_as_tuple(
            's', [](const char* arg){ return arg[0]=='s'; }),
          "starts with \'s\'")
      // (&c,".*\\.txt","ends with .txt",name{"regex"})
      .parse(argc,argv);
  } catch (const std::exception& e) {
    cerr <<"\033[31m"<< e.what() <<"\033[0m"<< endl;
    return 1;
  }

  TEST( d )
  TEST( i )
  TEST( s )
  TEST( b )

  return 0;
}
