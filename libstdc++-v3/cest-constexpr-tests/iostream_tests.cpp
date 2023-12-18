#include <cassert>
#include <fstream>
#include <iostream>

constexpr bool iostream_test1()
{
  std::cout << "Hello ";
  std::cout << "World!" << std::endl;
  std::cerr << "Fire and ";
  std::cerr << "Brimstone!" << std::endl;
  std::cout << std::hex << 66 << ' ' << 66U << std::endl; // 42
  std::cerr << std::hex << 66 << ' ' << 66U << std::endl;
  return true;
}

constexpr bool iostream_test2()
{
  bool b = true;
  std::string filename(__FILE__);

  if (!std::is_constant_evaluated())
  {
    std::ifstream ifs(filename.c_str());
    if (!ifs)
      return false;
    std::istream &is = ifs;
    char c;
    is.get(c);
    b = b && '#' == c;
  }

  return b;
}

void iostream_tests()
{
  using std_char_type = decltype(std::cout)::char_type;
  using std_traits_type = decltype(std::cout)::traits_type;
  auto &std_endl = std::endl<std_char_type, std_traits_type>;

  static_assert(iostream_test1());
  // static_assert(iostream_test2<std::string,std::ifstream,std::istream>());

  assert(iostream_test1());
  assert(iostream_test2());
}

int main(int argc, char *argv[])
{
  iostream_tests();
  return 0;
}
