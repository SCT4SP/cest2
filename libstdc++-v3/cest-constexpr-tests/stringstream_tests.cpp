#include <cassert>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>

namespace ss_tests {

template <typename Iss, typename S> constexpr bool istringstream_test1() {
  S str("def");
  std::stringbuf sb("ok"); // This was not in the C'est tests
  Iss s1("abc");
  Iss s2(str);
  char c1, c2;
  s1.get(c1);
  s2.get(c2);
  bool b1 = c1 == 'a' && c2 == 'd';
  s1.get(c1);
  s2.get(c2);
  bool b2 = c1 == 'b' && c2 == 'e';
  bool good = s1.good() && s2.good();
  s1.unget();
  s2.unget();
  s1.get(c1);
  s2.get(c2);
  bool b3 = c1 == 'b' && c2 == 'e';
  bool beof1 = !s1.eof() && !s1.fail();
  s1.get(c1);
  s1.get(c1);
  bool beof2 = s1.eof() && s1.fail();
  return b1 && b2 && b3 && good && beof1 && beof2;
}

template <typename Iss, typename S> constexpr bool istringstream_test2() {
  Iss s;
  S str("alphaville");
  s.str(std::move(str));
  char c;
  s.get(c);
  char buf[4];
  s.read(buf, 4);
  bool b = 'a' == c && 'l' == buf[0] && 'p' == buf[1] && 'h' == buf[2] &&
           'a' == buf[3];
  s.get(c);
  return b && 'v' == c;
}

template <typename Ifs, typename S, typename Isbi>
constexpr bool istringstream_test3() {
  Ifs file(__FILE__);
  // std::istreambuf_iterator<typename S::value_type> ite(file);
  Isbi it(file);
  bool b1 = *it == '#';
  S str(it, {});
  bool b2 = str[0] == '#';
  return b1 && b2;
}

} // namespace ss_tests

constexpr bool help()
{
  std::string str("def");
  std::istringstream s(str);
  return s.good();
}

void stringstream_tests() {
  using namespace ss_tests;

  using std_isbi = std::istreambuf_iterator<char>;

  std::ifstream file(__FILE__);
  std::istreambuf_iterator<char> it(file);
#if 0
    if (!file) // operator! is used here
    {
        std::cout << "File opening failed\n";
    }
if (file.bad())
        std::cout << "I/O error while reading\n";
    else if (file.eof())
        std::cout << "End of file reached successfully\n";
    else if (file.fail())
        std::cout << "Non-integer data encountered\n";
if (file.rdbuf() == nullptr)
  std::cout << "rdbuf is null\n";
std::string filename = __FILE__;
std::cout << filename << ' ' << file.is_open() << ' ' << file.good() << ' ';
std::cout << *it << std::endl; // *it++ causes a crash
#endif
  bool b1 = *it == '#';
  assert(b1);
  assert((istringstream_test1<std::istringstream, std::string>()));
  assert((istringstream_test2<std::istringstream, std::string>()));
  assert((istringstream_test3<std::ifstream, std::string, std_isbi>()));

  static_assert(istringstream_test1<std::istringstream, std::string>());
  static_assert(istringstream_test2<std::istringstream, std::string>());
}

int main(int argc, char *argv[])
{
  stringstream_tests();
  return 0;
}
