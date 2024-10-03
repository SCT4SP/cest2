#include <stack>
// #include <ranges>
#include <cassert>

constexpr bool stack_test1()
{
  std::stack<int> s1{}, s2{};

  bool b = s1.empty();
  int i = 43;
  s1.push(42);
  s1.push(i);
  b = b && s1.size() == 2 && s1.top() == 43;
  s1.emplace(44);
  b = b && s1.size() == 3;
  s1.pop();
  b = b && s1.size() == 2 && s1.top() == 43;

  s2.emplace(42);
  s2.emplace(43);
  b = b && s1 == s2 && s1 >= s2 && s1 <= s2;
  s2.top() -= 3;
  b = b && s1 != s2 && s2.top() == 40;
  b = b && s1 > s2 && s1 >= s2 && s2 < s1 && s2 <= s1;
  s1.swap(s2);
  b = b && s1 < s2;

  const int arr[] = {43, 42};
  // s2.push_range(arr); // not in libstdc++

  b = b && !std::uses_allocator<std::stack<int>, void>::value &&
            std::uses_allocator<std::stack<int>, std::allocator<int>>::value;
  std::stack<int> s3(s2);
  std::stack<int> s4(std::begin(arr), std::end(arr));
  // todo: other constructors

  return b;
}

void stack_tests()
{
  static_assert(stack_test1());
  assert(stack_test1());
}

int main(int argc, char *argv[])
{
  stack_tests();
  return 0;
}
