#include <cmath>
#include <numbers>
#include <concepts>
#include <cassert>
#include <climits> // INT_MAX
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
constexpr bool basic_cmath_tests()
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
constexpr bool infinite_math_tests()
{
  auto inf = std::numeric_limits<T>::infinity();
  auto qnan = std::numeric_limits<T>::quiet_NaN();
  auto snan = std::numeric_limits<T>::signaling_NaN();
  T zero = 0;
  bool b = std::isinf( inf) && std::isinf(-inf) && !std::isinf(zero);
  b = b && std::isnan(qnan) && std::isnan(snan) && !std::isnan(zero);

  b = b && std::fpclassify( inf)==FP_INFINITE;
  b = b && std::fpclassify(-inf)==FP_INFINITE;
  b = b && std::fpclassify(qnan)==FP_NAN;
  b = b && std::fpclassify(snan)==FP_NAN;
  b = b && std::fpclassify(zero)==FP_ZERO;
  b = b && std::fpclassify(zero)==FP_ZERO;

  return b;
}

template <std::floating_point T>
constexpr bool floor_ceil_logb_tests()
{
  auto inf = std::numeric_limits<T>::infinity();
  auto qnan = std::numeric_limits<T>::quiet_NaN();
  auto snan = std::numeric_limits<T>::signaling_NaN();
  T zero = 0, one = 1, two = 2, three = 3, pi = std::numbers::pi_v<T>;

  bool b = true;

  b = b && -1 == std::ilogb(static_cast<T>(0.8));
  b = b &&  0 == std::ilogb(one);
  b = b && FP_ILOGB0==std::ilogb(zero);
  b = b && INT_MAX==std::ilogb(inf);
  b = b && FP_ILOGBNAN==std::ilogb(qnan) && FP_ILOGBNAN==std::ilogb(snan);
  b = b && std::ceil(pi) > three;

  return b;
}

template <std::floating_point T>
constexpr void run_cmath_tests()
{
  assert(basic_cmath_tests<T>());
#ifndef __clang__
  static_assert(basic_cmath_tests<T>());
#endif

  assert(infinite_math_tests<T>());
  static_assert(infinite_math_tests<T>());

  assert(floor_ceil_logb_tests<T>());
}

int main(int argc, char *argv[])
{
#ifndef __clang__
  static_assert(!std::is_same_v<std::float128_t, long double>); // just saying
#endif
  run_cmath_tests<float>();
  run_cmath_tests<double>();
  run_cmath_tests<long double>();
#ifndef __clang__
  run_cmath_tests<std::float16_t>();
  run_cmath_tests<std::bfloat16_t>();
  run_cmath_tests<std::float128_t>();
#endif

  return 0;
}
