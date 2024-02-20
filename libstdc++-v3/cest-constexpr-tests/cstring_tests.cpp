#include <cstring>
#include <cassert>

constexpr bool cstring_tests()
{
  bool b = 0 == std::strcmp("why?", "why?"); // but not ::strcmp
  return b;
}

int main(int argc, char *argv[])
{
  static_assert(cstring_tests());
  assert(cstring_tests());
  return 0;
}
