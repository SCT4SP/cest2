#include <execution>
#include <vector>
#include <algorithm>
#include <cassert>

constexpr bool policy_test1()
{
  std::vector v = {1,2,3,4,5,6,7,8};
  std::for_each(std::execution::seq, v.begin(), v.end(), [](auto& x) { x++; });
  bool b = v == std::vector{2,3,4,5,6,7,8,9};
  return b;
}

int main(int argc, char *argv[])
{
  static_assert(policy_test1());
  assert(policy_test1());
  return 0;
}
