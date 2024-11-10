#include <map>
#include <utility> // std::pair
#include <cassert>

constexpr bool multimap_test1()
{
  using map_t = std::multimap<int,char>;
  using value_t = typename map_t::value_type; // std::pair<const int, char>
  bool b = std::is_same_v<value_t, std::pair<const int, char>>;
  const std::initializer_list<value_t> il = {value_t{1,'a'}, {2,'b'}, {2,'c'}};

  map_t m1{};
  map_t m2{{1,'a'}, {2,'b'}, {2,'c'}};
  std::multimap<int, char> m3{il.begin(), il.end()};

  b = b && m1.empty() && m1.size() == 0;
  b = b && m2.size() == 3 && m3.size() == 3;
  return b;
}

void multimap_tests()
{
  static_assert(multimap_test1());
  assert(multimap_test1());
}

int main(int argc, char *argv[])
{
  multimap_tests();
  return 0;
}
