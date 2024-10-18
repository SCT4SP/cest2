#ifndef _CEST_TESTS_UTIL_HPP_
#define _CEST_TESTS_UTIL_HPP_

#include <algorithm>
#include <cassert>
#include <cmath>

namespace tests_util {

template <typename T>
constexpr bool comparable(const T &x, const T &y, const T tolerance = 0.01) {
  return std::abs(x - y) < x * tolerance;
}

template <bool TestAssert = true> struct Bar {
  constexpr Bar() : p_(new int(42)) {}
  constexpr Bar(int x) : p_(new int(x)) {}
  constexpr Bar(const Bar &f) noexcept : p_(new int(*f.p_)) {}
  constexpr Bar(Bar &&other) : p_(other.p_) {
    assert(TestAssert);
    other.p_ = nullptr;
  }
  constexpr ~Bar() { delete p_; }
  constexpr Bar &operator=(const Bar &other) {
    delete p_;
    p_ = new int(*other.p_);
    return *this;
  }
  constexpr Bar &operator=(Bar &&other) {
    std::swap(p_, other.p_);
    return *this;
  }
  constexpr friend bool operator<(const Bar& x, const Bar& y) {
    return *x.p_ < *y.p_;
  }
  int *p_;
};

template <typename C> constexpr bool push_back_dtor_test() {
  C c;
  typename C::value_type f(42);
  c.push_back(f);
  c.push_back(f); // ~Bar() (Bar destructor) called here (by reserve w' vector)
  bool b1 = 42 == *c.begin()->p_ && 2 == std::distance(c.begin(), c.end());
  c.pop_back(); // ~Bar() (Bar destructor) called here
  bool b2 = 1 == std::distance(c.begin(), c.end());
  return b1 && b2;
}

template <typename C> constexpr bool push_front_dtor_test() {
  C c;
  typename C::value_type f(42);
  c.push_front(f);
  c.push_front(f); // ~Bar() (Bar destructor) called here
  bool b1 = 42 == *c.begin()->p_ && 2 == std::distance(c.begin(), c.end());
  c.pop_front(); // ~Bar() (Bar destructor) called here
  bool b2 = 1 == std::distance(c.begin(), c.end());
  return b1 && b2;
}

template <typename C>
constexpr bool push_dtor_test() {
  C c;
  typename C::value_type f(42);
  c.push(f);
  c.push(f); // ~Bar() (Bar destructor) called here
  bool b1 = 42 == *c.front().p_ && 2 == c.size();
  c.pop(); // ~Bar() (Bar destructor) called here
  bool b2 = 1 == c.size();
  return b1 && b2;
}

template <typename C>
constexpr bool push_dtor_test_pq() {
  C c;
  typename C::value_type f(42);
  c.push(f);
  c.push(f); // ~Bar() (Bar destructor) called here
  bool b1 = 42 == *c.top().p_ && 2 == c.size();
  c.pop(); // ~Bar() (Bar destructor) called here
  bool b2 = 1 == c.size();
  return b1 && b2;
}

} // namespace tests_util

#endif // _CEST_TESTS_UTIL_HPP_
