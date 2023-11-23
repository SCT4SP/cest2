#include <cmath>
#include <numbers>
#include <cassert>

// This test fails with Clang.
// P0533: constexpr for <cmath> and <cstdlib> (C++23)
// P1383: More constexpr for <cmath> and <complex> (C++26)
// Partial support for both in libstdc++

// Based on https://codereview.stackexchange.com/a/283360:
template <std::floating_point T> [[nodiscard]]
constexpr bool is_float_equal(T x, T y) { return y == std::nextafter(x, y); }

template <typename T = float>
constexpr bool cmath_tests()
{
  T pi = std::atan(1.0f) * 4.0f;
  T pi2 = std::numbers::pi_v<T>;
  bool b1 = is_float_equal(pi,pi2);

  T f1 = std::sin(pi2/4.0f);
  T f2 = std::cos(pi2/4.0f);
  bool b2 = is_float_equal(f1,f2);

  T f3 = std::sqrt(4.0f);
  bool b3 = is_float_equal(f3, 2.0f);

  return b1 && b2 && b3;
}

int main(int argc, char *argv[])
{
  assert(cmath_tests());
  static_assert(cmath_tests());
  return 0;
}
