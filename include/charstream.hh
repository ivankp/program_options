#ifndef IVANP_CHARSTREAM_HH
#define IVANP_CHARSTREAM_HH

#include <istream>
#include <streambuf>
#include <cstring>

namespace ivanp {

struct membuf : std::streambuf {
  membuf(const char* str, size_t n) {
    char *p = const_cast<char*>(str);
    this->setg(p, p, p+n);
  }
};

class charstream : public std::istream {
  membuf buf;
  charstream(const char* str, size_t n): std::istream(&buf), buf(str,n) { }
  charstream(const char* str): std::istream(&buf), buf(str,strlen(str)) { }
}

} // end namespace ivanp

#endif
