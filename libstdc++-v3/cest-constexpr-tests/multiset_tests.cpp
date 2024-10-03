#include <set>
#include <cassert>

constexpr bool multiset_test1()
{
  std::multiset<int> ms1{};
  std::multiset<int> ms2{std::less<int>{}};
  return true;
}

void multiset_tests()
{
  static_assert(multiset_test1());
  assert(multiset_test1());
}

int main(int argc, char *argv[])
{
  multiset_tests();
  return 0;
}
