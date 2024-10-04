#include <set>
#include <cassert>

constexpr bool multiset_test1()
{
  std::multiset<int> m1{};
  std::multiset<int> m2{std::less<int>{}};
  bool b = m1.empty() && m1.size() == 0;
  m1.insert(42);
  b = b && m1.size()==1;
  b = b && m1.max_size() == m2.max_size() && m1.max_size() > 0;
  m1.clear();
  b = b && m1.empty();
  int i = 4;
  m1.emplace(i); m1.emplace(3); m1.emplace(2); m1.emplace_hint(m1.find(3), 1);
  std::multiset<int> m3(m1.begin(), m1.find(3));
  std::multiset<int> m4(m3);
  std::multiset<int> m5(std::move(m4));
  std::multiset<int> m6{3, 2, 1, 2, 4, 4, 6};
  b = b && m3 == m5 && m5.size() == 2;
  std::multiset<int> m7{m6.cbegin(),  m6.cend()};
  std::multiset<int> m8{m6.rbegin(),  m6.rend()};
  std::multiset<int> m9{m6.crbegin(), m6.crend()};
  b = b && m6.size() == 7 && m7.size() == 7 && m8.size() == 7;
  m9.erase(m9.find(2));
  b = b && m9.size() == 6 && m9.count(4) == 2;
  m8.swap(m9);
  b = b && m8.size() == 6;
  //auto nh = m8.extract(4);
  //nh.value() = 5;
  return b;
}

void multiset_tests()
{
  static_assert(multiset_test1());
  assert(multiset_test1());
}

int main(int argc, char *argv[])
{
  multiset_tests();
  return 0;
}
