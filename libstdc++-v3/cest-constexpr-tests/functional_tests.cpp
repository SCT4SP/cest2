#include <functional>
#include <cassert>

constexpr int forty_two(double *, char) { return 42; }

constexpr bool function_test1()
{
  bool b0 = true;
  double d{0};

  std::function<bool()> f0a, f0b = nullptr; // no target is contained

  std::function<bool()> f1 = [&]{ return b0; };
  std::function<bool()> f1_copy = f1;                 // copy ctor
  bool b1 = !f0a && !f0b && f1() && f1_copy();
  std::function<bool()> f1_move = std::move(f1_copy); // move ctor
  f1_move = f1;            // assignment (uses function::swap)
  f1 = std::move(f1_move); // move assignment
  b1 = b1 && f1();
  f1 = nullptr;
  b1 = b1 && !f1;
  std::function<bool()> f1_ref;
  f1_ref = std::ref(f1);
  f1_ref = std::function<int()>(); // "assignment to a new target"
  b1 = b1 && nullptr == f1_ref;    // null ptr comparisons

  std::function<int(double*,char)> f2 = forty_two;
#ifdef __clang__
  bool b2 = 42 == f2(&d, 'q') && typeid(void) != f2.target_type();
#else
  // GCC bug 113306 https://gcc.gnu.org/bugzilla/show_bug.cgi?id=113306
  bool b2 = 42 == f2(&d, 'q');
#endif

  std::function<int(int, int)> myplus = std::plus<int>();
  std::function<int(int, int)> myminus = std::minus<int>();
  bool b3 =  myplus.target<std::plus<int>>()  != nullptr;
  std::swap(myplus, myminus); // careful now!
  b3 = b3 && myplus.target<std::minus<int>>() != nullptr
          && myplus.target<void()>()          == nullptr;

  int (*const* fp)(double*, char) = f2.target<int(*)(double*, char)>();
  bool b4 = fp && *fp == forty_two;

  bool b = b0 && b1 && b2 && b3 && b4;
  return b;
}

int main(int argc, char *argv[])
{
  static_assert(function_test1());
  assert((function_test1()));

  return 0;
}
