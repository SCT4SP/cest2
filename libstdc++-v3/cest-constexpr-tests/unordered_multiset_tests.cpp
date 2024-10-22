#include <unordered_set>
#include <cassert>

constexpr bool unordered_multiset_test1()
{
  std::unordered_multiset<int> m1{};
  std::unordered_multiset<int> m2{100, std::hash<int>{}};
  bool b = m1.empty() && m1.size() == 0;
  m1.insert(42);
  b = b && m1.size()==1;
  b = b && m1.max_size() == m2.max_size() && m1.max_size() > 0;
  m1.clear();
  b = b && m1.empty();
  int i = 4;
  m1.emplace(i); m1.emplace(3); m1.emplace(2); m1.emplace_hint(m1.find(3), 1);
  std::unordered_multiset<int> m3(m1.begin(), m1.find(3));
  std::unordered_multiset<int> m4(m3);
  std::unordered_multiset<int> m5(std::move(m4));
  std::unordered_multiset<int> m6{3, 2, 1, 2, 4, 4, 6};
  b = b && m3 == m5 && m5.size() == 2;
  std::unordered_multiset<int> m7{m6.cbegin(),  m6.cend()}, m8;
  b = b && m6.size() == 7 && m7.size() == 7;

  m7.erase(m7.find(2)); // {3, 2, 1, 2, 4, 4, 6} -> {3, 1, 2, 4, 4, 6}
  b = b && m7.size() == 6;
  m7.swap(m8);
  b = b && m8.size() == 6;
  auto nh = m8.extract(4); // {3, 1, 2, 4, 4, 6} -> {3, 1, 2, 4, 6}
  b = b && 4==nh.value() && 1==m8.count(4) && 5==m8.size();
  nh.value() = 5;
  // m8.insert(std::move(nh)); // this shouldn't be needed
  b = b && 5==nh.value();
  return b;
}

void unordered_multiset_tests()
{
  static_assert(unordered_multiset_test1());
  assert(unordered_multiset_test1());
}

int main(int argc, char *argv[])
{
  unordered_multiset_tests();
  return 0;
}
