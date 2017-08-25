#include <iostream>
#include <vector>
#include <set>

#include "program_options.hh"

using ivanp::po::detail::maybe_emplace;

int main(int argc, char* argv[]) {

  std::multiset<int> x;

  // maybe_emplace(x,"hello");
  maybe_emplace(x,0);
  x.emplace(0);
  x.emplace("");

  // TEST( x[0] )
  // TEST( x.count("hello") )
  TEST( x.count(0) )

}
