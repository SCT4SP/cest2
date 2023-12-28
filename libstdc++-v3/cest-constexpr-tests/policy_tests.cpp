#include <execution>
#include <vector>
#include <algorithm>
#include <cassert>

constexpr bool policy_test1()
{
  std::vector v = {1,2,3,4,5,6,7,8};
  std::for_each(std::execution::seq, v.begin(), v.end(), [](auto& x) { x++; });
  bool b = v == std::vector{2,3,4,5,6,7,8,9};
  int x = std::transform_reduce(
    std::execution::seq,
    v.begin(),     // 2,3,4,5,6,7,8  (7 numbers)
    v.end() - 1,
    v.begin() + 1, // 3,4,5,6,7,8,9  ("")
    0,
    std::plus{},
    [](int x, int y) { return y - x; } // always 1 with the input data
  );
  return b && x==7;
}

int main(int argc, char *argv[])
{
  static_assert(policy_test1());
  assert(policy_test1());
  return 0;
}
