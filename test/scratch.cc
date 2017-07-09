#include <iostream>
#include <type_traits>

#include "meta.hh"
#include "type.hh"

int main(int argc, char* argv[]) {
  
  prt_type<ivanp::find_first_t<std::is_integral,double,float,int,char>>();
  prt_type<ivanp::find_first_t<std::is_integral,double,float>>();

}
