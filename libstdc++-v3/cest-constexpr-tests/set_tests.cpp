#include <algorithm>
#include <cassert>
#include <iostream>
#include <set>
#include <tuple>
#include <type_traits>
#include <limits>

namespace set_tests_ns {

constexpr bool common_static_set_tests() {
  auto f = []<template <class...> class S>() {
    static_assert(sizeof(S<float>) == sizeof(S<double>));
    static_assert(std::weakly_incrementable<typename S<int>::iterator>);
    using iter_t = typename S<int>::iterator;
    using const_iter_t = typename S<int>::const_iterator;
    static_assert(std::is_same_v<typename iter_t::iterator_category,
                                 std::bidirectional_iterator_tag>);
    static_assert(std::is_same_v<iter_t, const_iter_t>);
  };

  f.operator()<std::set>();

  return true;
}

template <typename S> constexpr bool set_test1() {
  S set;

  auto r1 = set.insert(3); // set size increased: insert done
  auto r2 = set.insert(3); // set size unchanged: insert not done
  return r1.first != set.end() && 3 == *r1.first && r1.second &&
         r2.first == r1.first && 3 == *r2.first && !r2.second;
}

template <typename S> constexpr bool set_test2() {
  S set;

  auto r1 = set.insert(1);
  auto r2 = set.insert(2);
  return 2 == set.size() && 1 == *r1.first && r1.second && 2 == *r2.first &&
         r2.second;
}

template <typename S, typename T> constexpr auto set_test3(T x, T y, T z) {
  S s;

  auto r1 = s.insert(x);
  auto r2 = s.insert(y);

  auto r3 = s.insert(z);

  return std::tuple{s.size(), *r1.first, *r2.first, *r3.first};
}

template <class T, class U> constexpr T &inserts(T &s, U x) {
  s.insert(x);
  return s;
}

template <class T, class U, class... Us>
constexpr T &inserts(T &s, U x, Us... xs) {
  s.insert(x);
  return inserts(s, xs...);
}

template <typename S, typename T>
constexpr bool set_test4(T x1, T x2, T x3, T x4, T x5) {
  S s1, s2, s3;
  s1.insert(x1);
  s1.insert(x2);
  s1.insert(x3);
  s1.insert(x4);
  s1.insert(x5);
  s2.insert(x5);
  s2.insert(x4);
  s2.insert(x3);
  s2.insert(x2);
  s2.insert(x1);
  s3.insert(x5);
  s3.insert(x1);
  s3.insert(x4);
  s3.insert(x2);
  s3.insert(x3);

  return 5 == s1.size() && 5 == s2.size() && 5 == s3.size();
}

template <typename S, typename T> constexpr bool set_test5(T x) {
  S s;

  auto ib0 = s.begin();
  auto ie0 = s.end();
  bool b0 = ib0 == ie0;

  s.insert(x);
  auto ib1 = s.begin();
  bool b1 = ib1 == ie0;
  T a = *ib1;
  s.insert(x - 1);
  auto ib2 = s.begin();
  bool b2 = ib2 == ie0;
  T b = *ib2;
  s.insert(x + 1);
  auto ib3 = s.begin();
  bool b3 = ib3 == ie0;
  T c = *ib3;

  return b0 && !b1 && !b2 && !b3 && 42 == a && 41 == b && 41 == c &&
         3 == s.size();
}

// test pre-increment (and insert, begin and end)
template <typename S, typename T, typename... Ts>
constexpr bool set_test6(T x, Ts... xs) {
  S s;

  s.insert(x);
  auto it = s.begin();
  ++it;
  bool r0 = it == s.end();

  // wikipedia - a different structure, but still fine (a valid rb tree)
  inserts(s, x, xs...);

  auto ip1 = s.insert(6);
  bool r1 = ip1.second;
  auto it1 = ip1.first;
  auto a1 = *it1;
  ++it1; // 6 -> 8
  auto b1 = *it1;

  auto ip12 = s.insert(8);
  bool r12 = ip12.second;
  auto it12 = ip12.first;
  auto a12 = *it12;
  ++it12; // 8 -> 11
  auto b12 = *it12;

  auto ip2 = s.insert(15);
  bool r2 = ip2.second;
  auto it2 = ip2.first;
  auto a2 = *it2;
  ++it2; // 15 -> 17
  auto b2 = *it2;

  auto ip3 = s.insert(1);
  bool r3 = ip3.second;
  auto it3 = ip3.first;
  auto a3 = *it3;
  ++it3; // 1 -> 6
  auto b3 = *it3;

  auto ip4 = s.insert(11);
  bool r4 = ip4.second;
  auto it4 = ip4.first;
  auto a4 = *it4;
  ++it4; // 11 -> 13
  auto b4 = *it4;

  auto ip5 = s.insert(27);
  bool r5 = ip5.second;
  auto it5 = ip5.first;
  auto a5 = *it5;
  ++it5; // 27 -> end
  auto r6 = it5 == s.end();

  bool r = !r1 && !r12 && !r2 && !r3 && !r4 && !r5; // no changes

  T sum{0};
  T prev = std::numeric_limits<int>::min();
  bool inc = true; // increasing
  for (auto it = s.begin(); it != s.end(); ++it) {
    inc = inc && (*it > prev);
    prev = *it;
    sum += prev;
  }

  return r0 && r && 6 == a1 && 8 == b1 && 8 == a12 && 11 == b12 && 15 == a2 &&
         17 == b2 && 1 == a3 && 6 == b3 && 11 == a4 && 13 == b4 && 27 == a5 &&
         r6 && 145 == sum && inc && 10 == s.size();
}

// tests post-increment
template <typename S> constexpr bool set_test7() {
  S s;
  inserts(s, 1, 5, 4, 2, 3);
  auto it0 = s.begin();
  auto it1 = it0++;
  return 2 == *it0 && 1 == *it1;
}

namespace test9 {
struct FatKey {
  int x;
  int data[1000];
};
struct LightKey {
  int x;
};
constexpr bool operator<(const FatKey &fk, const LightKey &lk) {
  return fk.x < lk.x;
}
constexpr bool operator<(const LightKey &lk, const FatKey &fk) {
  return lk.x < fk.x;
}
constexpr bool operator<(const FatKey &fk1, const FatKey &fk2) {
  return fk1.x < fk2.x;
}
} // namespace test9

// tests find
template <typename S> constexpr bool set_test8() {
  S s;
  inserts(s, 1, 2, 3, 4);
  auto it = s.find(2);
  bool ok = it != s.end();
  return ok;
}

template <typename S> constexpr bool set_test9() {
  using namespace test9;

  LightKey lk = {2};

  S s;
  FatKey fk{2, {}};
  inserts(s, FatKey{1, {}}, FatKey{2, {}}, FatKey{3, {}}, FatKey{4, {}});
  auto it = s.find(lk);  // The C++14 template version of find
  auto itf = s.find(fk); // ""
  int x = it->x;
  int xf = itf->x;
  bool ok = it != s.end();

  auto it2 = ++it;
  auto x2 = it2->x;
  auto it3 = it2++;
  auto x3 = it3->x;

  return ok && 2 == x && 2 == xf && 3 == x2 && 3 == x3;
}

template <typename S> constexpr bool set_test10() {
  S s1;
  inserts(s1, 1, 2, 3);
  const S s2 = s1;
  bool b = 3 == s1.size() && 3 == s2.size();

  S s3, s4;
  inserts(s3, 1, 2, 3, 4, 5);
  s4 = s3;
  s3 = s1;
  S s5 = s1;
  s5.clear();
  b = b && 3 == s3.size() && 5 == s4.size() && s5.empty();

  int arr[] = {1, 2, 3};
  S s6{std::begin(arr), std::end(arr)};
  auto nh = s6.extract(1);
  nh.value() = 4;
  nh.get_allocator();
  decltype(nh) nh2{};
  b = b && nh && !nh.empty() && !nh2 && nh2.empty();
  nh2 = std::move(nh);
  s6.insert(std::move(nh2)); // this shouldn't be needed
  return b;
}

template <bool SA, class S1, class S2, class S3, class S4, class S5, class S6,
          class S7, class S8, class S9, class S10>
constexpr void doit() {
  constexpr const auto tup3 = std::tuple{3, 3, 2, 1};
  constexpr const auto tup4 = std::tuple{3, 1, 2, 3};
  constexpr const auto tup5 = std::tuple{3, 1, 3, 2};
  constexpr const auto tup6 = std::tuple{2, 1, 2, 2};

  assert(set_test1<S1>());
  assert(set_test2<S2>());
  assert(set_test3<S3>(3, 2, 1) == tup3);
  assert(set_test3<S3>(1, 2, 3) == tup4);
  assert(set_test3<S3>(1, 3, 2) == tup5);
  assert(set_test3<S3>(1, 2, 2) == tup6);
  assert(set_test4<S4>(1, 2, 3, 4, 5));
  assert(set_test5<S5>(42));
  assert(set_test6<S6>(1, 6, 8, 11, 13, 15, 17, 22, 25, 27));
  assert(set_test6<S6>(27, 25, 22, 17, 15, 13, 11, 8, 6, 1));
  assert(set_test6<S6>(1, 27, 6, 25, 8, 22, 11, 17, 13, 15));
  assert(set_test7<S7>());
  assert(set_test8<S8>());
  assert(set_test9<S9>());
  assert(set_test10<S10>());

  if constexpr (SA) {
    static_assert(set_test1<S1>());
    static_assert(set_test2<S2>());
    static_assert(set_test3<S3>(3, 2, 1) == tup3);
    static_assert(set_test3<S3>(1, 2, 3) == tup4);
    static_assert(set_test3<S3>(1, 3, 2) == tup5);
    static_assert(set_test3<S3>(1, 2, 2) == tup6);
    static_assert(set_test4<S4>(1, 2, 3, 4, 5));
    static_assert(set_test5<S5>(42));
    static_assert(set_test6<S6>(1, 6, 8, 11, 13, 15, 17, 22, 25, 27));
    static_assert(set_test6<S6>(27, 25, 22, 17, 15, 13, 11, 8, 6, 1));
    static_assert(set_test6<S6>(1, 27, 6, 25, 8, 22, 11, 17, 13, 15));
    static_assert(set_test7<S7>());
    static_assert(set_test8<S8>());
    static_assert(set_test9<S9>());
    static_assert(set_test10<S10>());
  }
}

template <bool SA, template <class...> class St,
          template <class> class Alloc = std::allocator>
constexpr void tests_helper() {
  using S1 = St<int, std::less<int>, Alloc<int>>;
  using S2 = St<int, std::less<int>, Alloc<int>>;
  using S3 = St<int, std::less<int>, Alloc<int>>;
  using S4 = St<int, std::less<int>, Alloc<int>>;
  using S5 = St<int, std::less<int>, Alloc<int>>;
  using S6 = St<int, std::less<int>, Alloc<int>>;
  using S7 = St<int, std::less<int>, Alloc<int>>;
  using S8 = St<int, std::less<int>, Alloc<int>>;
  using S9 = St<test9::FatKey, std::less<>>;
  using S10 = St<int, std::less<int>, Alloc<int>>;

  doit<SA, S1, S2, S3, S4, S5, S6, S7, S8, S9, S10>();
}

void new_set_tests() {
  tests_helper<true, std::set>();
}

} // namespace set_tests_ns

void set_tests() {
  set_tests_ns::new_set_tests();
  static_assert(set_tests_ns::common_static_set_tests());
}

int main(int argc, char *argv[])
{
  set_tests();
  return 0;
}
