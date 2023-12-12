#include <functional>
#include <cassert>

constexpr int forty_two(double *, char) { return 42; }

constexpr bool function_test1()
{
  bool b0 = true;
  double d{0};

  std::function<bool()> f1 = [&]{ return b0; };

  std::function<bool(double*,char)> f2 = forty_two;
  bool b2 = typeid(void) != f2.target_type();

  std::function<int(int, int)> myplus = std::plus<int>();
  bool b3 = myplus.target<std::plus<int>>() != nullptr;

  //std::function<int(int, int)> myf = f; // use forty_two
  //int (*const* fptr)(int, int) = myf.target<int(*)(int, int)>();
  //bool b4 = fptr && *fptr == f;

  bool b = f1() && f2(&d,'q') && b2 && b3;
  return b;
}

int main(int argc, char *argv[])
{
  static_assert(function_test1());
  assert((function_test1()));

  return 0;
}
