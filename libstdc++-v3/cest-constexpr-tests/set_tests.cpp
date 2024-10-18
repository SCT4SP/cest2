#include <algorithm>
#include <cassert>
#include <iostream>
#include <set>
#include <tuple>
#include <limits>

constexpr bool common_static_set_tests()
{
  using namespace std;
  using sf_t = set<float>;
  using sd_t = set<double>;
  using si_t = set<double>;

  static_assert(sizeof(sf_t) == sizeof(sd_t));
  static_assert(weakly_incrementable<typename si_t::iterator>);
  using       iter_t = typename si_t::iterator;
  using const_iter_t = typename si_t::const_iterator;
  static_assert(is_same_v<iter_t, const_iter_t>);
  static_assert(is_same_v<typename iter_t::iterator_category,
                            bidirectional_iterator_tag>);
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

template <typename S, typename T> constexpr bool set_test5(const T x) {
  S s;

  auto ib0 = s.begin();
  auto ie0 = s.end();
  bool b0 = ib0 == ie0;

  s.insert(x);
  auto ib1 = s.begin();
  bool b1 = ib1 == ie0;
  T a = *ib1;
  s.insert(x-1);
  auto ib2 = s.begin();
  bool b2 = ib2 == ie0;
  T b = *ib2;
  s.insert(x+1);
  auto ib3 = s.begin();
  bool b3 = ib3 == ie0;
  T c = *ib3;

  return b0 && !b1 && !b2 && !b3 && x == a && (x-1) == b && (x-1) == c &&
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

  auto ip12 = s.insert(8);
  bool r12 = ip12.second;
  auto it12 = ip12.first;
  auto a12 = *it12;
  ++it12; // 8 -> 11

  auto ip2 = s.insert(15);
  bool r2 = ip2.second;
  auto it2 = ip2.first;
  auto a2 = *it2;
  ++it2; // 15 -> 17

  auto ip3 = s.insert(1);
  bool r3 = ip3.second;
  auto it3 = ip3.first;
  auto a3 = *it3;
  ++it3; // 1 -> 6

  auto ip4 = s.insert(11);
  bool r4 = ip4.second;
  auto it4 = ip4.first;
  auto a4 = *it4;
  ++it4; // 11 -> 13

  auto ip5 = s.insert(27);
  bool r5 = ip5.second;
  auto it5 = ip5.first;
  auto a5 = *it5;
  ++it5; // 27 -> end

  bool r = !r1 && !r12 && !r2 && !r3 && !r4 && !r5; // no changes

  T sum{0};
  T prev = std::numeric_limits<int>::min();
  bool inc = true; // increasing
  for (auto it = s.begin(); it != s.end(); ++it) {
    inc = inc && (*it > prev);
    prev = *it;
    sum += prev;
  }

  bool b =  6 == a1 &&   8 == a12 && 15 == a2 && 1 == a3 && 11 == a4 &&
           27 == a5 && 145 == sum && 10 == s.size();
  return r0 && r && b && 8 == *it1 && 11 == *it12 && 17 == *it2 && 6 == *it3 &&
         13 == *it4 && (it5==s.end()) && 145 == sum && inc && 10 == s.size();
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
  S s6{std::begin(arr), std::end(arr)};   // {1,2,3}
  auto nh = s6.extract(1);                //   {2,3}
  S s7{std::begin(arr)+1, std::end(arr)}; //   {2,3}
  b = b && 1==nh.value() && 2==s6.size() && s6==s7;
  nh.value() = 4;
  nh.get_allocator();
  decltype(nh) nh2{};
  b = b && nh && !nh.empty() && !nh2 && nh2.empty();
  nh2 = std::move(nh);
//  s6.insert(std::move(nh2)); // this shouldn't be needed
  return b;
}

constexpr void doit()
{
  using set_t  = std::set<int>;
  using setF_t = std::set<test9::FatKey, std::less<>>;
  constexpr const auto tup1 = std::tuple{3, 3, 2, 1};
  constexpr const auto tup2 = std::tuple{3, 1, 2, 3};
  constexpr const auto tup3 = std::tuple{3, 1, 3, 2};
  constexpr const auto tup4 = std::tuple{2, 1, 2, 2};

  assert(set_test1<set_t>());
  assert(set_test2<set_t>());
  assert(set_test3<set_t>(3, 2, 1) == tup1);
  assert(set_test3<set_t>(1, 2, 3) == tup2);
  assert(set_test3<set_t>(1, 3, 2) == tup3);
  assert(set_test3<set_t>(1, 2, 2) == tup4);
  assert(set_test4<set_t>(1, 2, 3, 4, 5));
  assert(set_test5<set_t>(42));
  assert(set_test6<set_t>(1, 6, 8, 11, 13, 15, 17, 22, 25, 27));
  assert(set_test6<set_t>(27, 25, 22, 17, 15, 13, 11, 8, 6, 1));
  assert(set_test6<set_t>(1, 27, 6, 25, 8, 22, 11, 17, 13, 15));
  assert(set_test7<set_t>());
  assert(set_test8<set_t>());
  assert(set_test9<setF_t>());
  assert(set_test10<set_t>());

  static_assert(set_test1<set_t>());
  static_assert(set_test2<set_t>());
  static_assert(set_test3<set_t>(3, 2, 1) == tup1);
  static_assert(set_test3<set_t>(1, 2, 3) == tup2);
  static_assert(set_test3<set_t>(1, 3, 2) == tup3);
  static_assert(set_test3<set_t>(1, 2, 2) == tup4);
  static_assert(set_test4<set_t>(1, 2, 3, 4, 5));
  static_assert(set_test5<set_t>(42));
  static_assert(set_test6<set_t>(1, 6, 8, 11, 13, 15, 17, 22, 25, 27));
  static_assert(set_test6<set_t>(27, 25, 22, 17, 15, 13, 11, 8, 6, 1));
  static_assert(set_test6<set_t>(1, 27, 6, 25, 8, 22, 11, 17, 13, 15));
  static_assert(set_test7<set_t>());
  static_assert(set_test8<set_t>());
  static_assert(set_test9<setF_t>());
  static_assert(set_test10<set_t>());
}

void set_tests() {
  doit();
  static_assert(common_static_set_tests());
}

int main(int argc, char *argv[])
{
  set_tests();
  return 0;
}
