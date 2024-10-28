#include <cmath>
#include <numbers>
#include <concepts>
#include <cassert>
#ifndef __clang__
#include <stdfloat>
#endif

// This test fails with Clang.
// P0533: constexpr for <cmath> and <cstdlib> (C++23)
// P1383: More constexpr for <cmath> and <complex> (C++26)
// Partial support for both in libstdc++

// Based on https://codereview.stackexchange.com/a/283360:
template <std::floating_point T> [[nodiscard]]
constexpr bool is_float_equal(T x, T y) { return y == std::nextafter(x, y); }

template <std::floating_point T>
constexpr bool cmath_tests()
{
  const T one = 1, two = 2, four = 4;
  T pi = std::atan(one) * 4;
  T pi2 = std::numbers::pi_v<T>;
  bool b1 = is_float_equal(pi, pi2);

  T f1 = std::sin(pi2/four);
  T f2 = std::cos(pi2/four);
  bool b2 = is_float_equal(f1, f2);

  T f3 = std::sqrt(four);
  bool b3 = is_float_equal(f3, two);

  return b1 && b2 && b3;
}

template <std::floating_point T>
constexpr void run_cmath_tests()
{
#ifndef __clang__
  assert(cmath_tests<T>());
  static_assert(cmath_tests<T>());
#endif
}

int main(int argc, char *argv[])
{
#ifndef __clang__
  static_assert(!std::is_same_v<std::float128_t, long double>);
#endif
  run_cmath_tests<float>();
  run_cmath_tests<double>();
  run_cmath_tests<long double>();
  // https://sourceware.org/bugzilla/show_bug.cgi?id=32312
  // GCC: undefined reference to `nextafterf16'
  //run_cmath_tests<std::float16_t>();
#ifndef __clang__
  run_cmath_tests<std::float128_t>();
#endif

  return 0;
}
