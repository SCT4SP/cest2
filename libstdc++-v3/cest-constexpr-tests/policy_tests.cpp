#include <execution>
#include <vector>
#include <algorithm>
#include <cassert>
#include <functional> // std::identity, std::multiplies, std::plus

constexpr bool policy_test1()
{
  std::vector v = {1,2,3,4,5,6,7,8};
  int x1a = std::reduce( // pstl implements this using transform_reduce
    std::execution::seq, v.begin(), v.end(), 1, std::multiplies{}
  );
  int x1b = std::transform_reduce(
    std::execution::seq, v.begin(), v.end(), 1, std::multiplies{},
                                                  std::identity{}
  );
  int x1c = std::transform_reduce(
    std::execution::seq, v.begin(), v.end() - 1,
                         v.begin() + 1, 1, std::plus{}, std::plus{}
  );
  std::for_each(std::execution::seq, v.begin(), v.end(), [](auto& x) { x++; });
  bool b = v == std::vector{2,3,4,5,6,7,8,9};
  int x2 = std::transform_reduce(
    std::execution::seq,
    v.begin(),     // 2,3,4,5,6,7,8  (7 numbers)
    v.end() - 1,
    v.begin() + 1, // 3,4,5,6,7,8,9  ("")
    0,
    std::plus{},
    [](int x, int y) { return y - x; } // always 1 with the input data
  );

  std::for_each_n(std::execution::seq, v.begin(), 4, [](auto& x) { x=0; });
  bool b2 = v == std::vector{0,0,0,0,6,7,8,9};
  return b && b2 && x1a==40320 && x1b==40320 && x1c==64 && x2==7;
}

int main(int argc, char *argv[])
{
  static_assert(policy_test1());
  assert(policy_test1());
  return 0;
}
