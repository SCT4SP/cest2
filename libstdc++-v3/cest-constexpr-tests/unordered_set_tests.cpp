#include <algorithm>
#include <cassert>
#include <unordered_set>
#include <tuple>
#include <limits> // std::numeric_limits
#include <string>

constexpr bool common_static_uset_tests()
{
  using namespace std;
  using sf_t = unordered_set<float>;
  using sd_t = unordered_set<double>;
  using si_t = unordered_set<double>;

  static_assert(sizeof(sf_t) == sizeof(sd_t));
  static_assert(weakly_incrementable<typename si_t::iterator>);
  using       iter_t = typename si_t::iterator;
  using const_iter_t = typename si_t::const_iterator;
  static_assert(!is_same_v<iter_t, const_iter_t>);
  static_assert( is_same_v<typename iter_t::iterator_category,
                             forward_iterator_tag>);
  return true;
}

constexpr bool uset_test1()
{
  std::unordered_set<int> s;

  auto r1 = s.insert(3); // set size increased: insert done
  auto r2 = s.insert(3); // set size unchanged: insert not done
  return r1.first != s.end() && 3 == *r1.first && r1.second &&
         r2.first == r1.first && 3 == *r2.first && !r2.second;
}

constexpr bool uset_test2()
{
  std::unordered_set<int> s;

  auto r1 = s.insert(1);
  auto r2 = s.insert(2);
  return 2 == s.size() && 1 == *r1.first && r1.second && 2 == *r2.first &&
         r2.second;
}

constexpr auto uset_test3(int x, int y, int z)
{
  std::unordered_set<int> s;

  auto r1 = s.insert(x);
  auto r2 = s.insert(y);

  auto r3 = s.insert(z);

  return std::tuple{s.size(), *r1.first, *r2.first, *r3.first};
}

constexpr bool uset_test4(int x1, int x2, int x3, int x4, int x5)
{
  std::unordered_set<int> s1, s2, s3;
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

constexpr bool uset_test5(const int x)
{
  std::unordered_set<int> s;

  auto ib0 = s.begin();
  auto ie0 = s.end();
  bool b0 = ib0 == ie0;

  s.insert(x);
  auto ib1 = s.begin();
  bool b1 = ib1 == ie0;
  int a = *ib1;
  s.insert(x-1);
  auto ib2 = s.begin();
  bool b2 = ib2 == ie0;
  int b = *ib2;
  s.insert(x+1);
  auto ib3 = s.begin();
  bool b3 = ib3 == ie0;
  int c = *ib3;

  return b0 && !b1 && !b2 && !b3 && x==a && (x-1)==b && (x+1)==c && 3==s.size();
}

// test pre-increment (and insert, begin and end)
template <typename T, typename... Ts>
constexpr bool uset_test6(T x, Ts... xs)
{
  std::unordered_set<int> s;

  s.insert(x);
  auto it = s.begin();
  ++it;
  bool r0 = it == s.end();

  s.insert({x, xs...});

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

  bool b = 6 == a1 && 8 == a12 && 15 == a2 && 1 == a3 && 11 == a4 &&
           27 == a5 && 145 == sum && 10 == s.size();
  return r0 && r && b;
}

// tests post-increment
constexpr bool uset_test7()
{
  std::unordered_set<int> s;
  s.insert({1, 5, 4, 2, 3});
  auto it0 = s.begin();
  auto it1 = it0++;
  return *it0 != *it1;
}

// tests find
constexpr bool uset_test8()
{
  std::unordered_set<int> s;
  s.insert({1, 2, 3, 4});
  auto it = s.find(2);
  bool ok = it != s.end();
  return ok;
}

constexpr bool uset_test9()
{
  using namespace std;
  using namespace std::literals;

  struct string_hash // C++20's transparent hashing
  {
    using hash_type = hash<string_view>;
    using is_transparent = void;

    constexpr size_t operator()(const char* str) const
    {
      return hash_type{}(str);
    }
    constexpr size_t operator()(string_view str) const
    {
      return hash_type{}(str);
    }
    constexpr size_t operator()(string const& str) const
    {
      return hash_type{}(str);
    }
  };

  unordered_set<int> example{1, 2, -10};

  int i = -1;
  if (auto search = example.find(2); search != example.end())
    i = *search;

  using uset_t = unordered_set<string, string_hash, equal_to<>>;
#if __clang__
  // So avoiding https://github.com/llvm/llvm-project/issues/112894
  uset_t set{string{"one"}, string{"two"}};
  bool b1 = set.find(string{"one"}) != set.end();
  bool b2 = set.find(string_view{"one"}) != set.end();
#else
  uset_t set{"one"s, "two"s};
  bool b1 = set.find("one"s) != set.end();
  bool b2 = set.find("one"sv) != set.end();
#endif
  bool b3 = set.find("one") != set.end();

  return i==2 && b1 && b2 && b3;
}

constexpr bool uset_test10()
{
  std::unordered_set<int> s1;
  s1.insert({1, 2, 3});
  const std::unordered_set<int> s2 = s1;
  bool b = 3 == s1.size() && 3 == s2.size();

  std::unordered_set<int> s3, s4;
  s3.insert({1, 2, 3, 4, 5});
  s4 = s3;
  s3 = s1;
  std::unordered_set<int> s5 = s1;
  s5.clear();
  b = b && 3 == s3.size() && 5 == s4.size() && s5.empty();

  int arr[] = {1, 2, 3};
  std::unordered_set<int> s6{std::begin(arr), std::end(arr)};   // {1,2,3}
  auto nh = s6.extract(1);                //   {2,3}
  std::unordered_set<int> s7{std::begin(arr)+1, std::end(arr)}; //   {2,3}
  b = b && 1==nh.value() && 2==s6.size() && s6==s7;
  nh.value() = 4;
  nh.get_allocator();
  decltype(nh) nh2{};
  b = b && nh && !nh.empty() && !nh2 && nh2.empty();
  nh2 = std::move(nh);
//  s6.insert(std::move(nh2)); // this shouldn't be needed
  return b;
}

constexpr bool doit()
{
  constexpr const std::tuple<int,int,int,int> t[] =
    {{3, 3, 2, 1}, {3, 1, 2, 3}, {3, 1, 3, 2}, {2, 1, 2, 2}};

  bool b = uset_test1();
  b = b && uset_test2();
  b = b && uset_test3(3, 2, 1) == t[0];
  b = b && uset_test3(1, 2, 3) == t[1];
  b = b && uset_test3(1, 3, 2) == t[2];
  b = b && uset_test3(1, 2, 2) == t[3];
  b = b && uset_test4(1, 2, 3, 4, 5);
  b = b && uset_test5(42);
  b = b && uset_test6(1, 6, 8, 11, 13, 15, 17, 22, 25, 27);
  b = b && uset_test6(27, 25, 22, 17, 15, 13, 11, 8, 6, 1);
  b = b && uset_test6(1, 27, 6, 25, 8, 22, 11, 17, 13, 15);
  b = b && uset_test7();
  b = b && uset_test8();
  b = b && uset_test9();
  b = b && uset_test10();

  return b;
}

void uset_tests() {
  assert(doit());
  static_assert(doit());
  static_assert(common_static_uset_tests());
}

int main(int argc, char *argv[])
{
  uset_tests();
  return 0;
}
