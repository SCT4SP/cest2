#include <cassert>
#include <sstream>
#include <string>
#include <ios>

// This is derived from istringstream_test1(), which fails at runtime with GCC
// The basic_istream ctor is at line 99 of include/std/istream

//  _GLIBCXX_CEST_CONSTEXPR
//  explicit
//  basic_istream(__streambuf_type* __sb)
//  : __ostream_type(__sb), _M_gcount(streamsize(0))
//  { this->init(__sb); }
//
// __ostream_type is roughly basic_ostream<char>

// good is a member of the virtually inherited std::basic_ios, which is
// not virtual in C'est 2:
//   class basic_istream :         public basic_ostream<_CharT, _Traits>
// instead of
//   class basic_istream : virtual public     basic_ios<CharT, Traits>
//
// In C'est 2, basic_ios is inherited via basic_istream's non-virtual
// inheritance from basic_ostream, which has it as a non-virtual base class.

int main(int argc, char *argv[])
{
  std::stringbuf sb("ok");
  std::basic_istream<char> is(&sb);
  std::basic_ostream<char> os(&sb);
  assert(is.good());
  assert(os.good());
  return 0;
}
