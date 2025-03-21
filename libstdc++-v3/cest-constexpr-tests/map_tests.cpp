#include <cassert>
#include <map>
#include <unordered_map>
#include <type_traits>

template <template <class...> class M>
constexpr bool static_tests()
{
  using namespace std;
  bool b = sizeof(M<float, int>) == sizeof(M<double, int>);
  b = b && sizeof(M<int, float>) == sizeof(M<int, double>);
  using       iter_t = typename M<char, int>::iterator;
  using const_iter_t = typename M<char, int>::const_iterator;
  b = b && weakly_incrementable<iter_t>;
  b = b && weakly_incrementable<const_iter_t>;
  using tag_t = conditional_t<is_same_v<M<char,int>, map<char,int>>,
                  bidirectional_iterator_tag, forward_iterator_tag>;
  b = b &&  is_same_v<typename iter_t::iterator_category, tag_t>;
  b = b && !is_same_v<iter_t, const_iter_t>;

  return b;
}

template <template <class...> class M, class T, class U>
constexpr bool map_test1() {
  M<T, U> m;

  using value_type = typename M<T, U>::value_type;
  static_assert(std::is_same_v<value_type, std::pair<const T, U>>);

  const value_type v1 = std::make_pair('a', 1);
  auto p1 = m.insert(v1);

  auto p2 = m.insert(v1);

  const value_type v2 = std::make_pair('a', 2);
  auto p3 = m.insert(v2);

  const value_type v3 = std::make_pair('b', 1);
  auto p4 = m.insert(v3);

  return p1.second && !p2.second && !p3.second && p4.second;
}

template <template <class...> class M, class T, class U>
constexpr bool map_test2() {
  M<T, U> m;
  m.clear();

  using value_type = typename M<T, U>::value_type;

  const value_type v1 = std::make_pair('a', 1);
  auto p1 = m.insert(v1);
  bool b0 = v1 == *p1.first && p1.second;

  auto i1 = m.find('a');
  bool b1 = i1 != m.end();

  auto i2 = m.find('q');
  bool b2 = i2 != m.end();

  bool b3 = 'a' == i1->first && 1 == i1->second;

  return b0 && b1 && !b2 && b3;
}

template <template <class...> class M, class T, class U>
constexpr bool map_test3() {
  M<T, U> m1;
  m1.insert({'a', 1});
  m1.insert({'b', 2});
  m1.insert({'c', 3});
  M<T, U> m2 = m1;
  bool b1 = 3 == m1.size() && 3 == m2.size();

  auto nh = m1.extract('b');
  bool b2 = 2 == m1.size() && 'b' == nh.key() && 2 == nh.mapped();
  nh.mapped() = 42;
  std::allocator<std::pair<const char, int>> alloc{};
  b2 = b2 && 42 == nh.mapped() && nh.get_allocator()==alloc;

  M<T, U> m3;
  m3.insert({'d', 4});
  m1 = m3;
  m3.clear();
  bool b3 = 1 == m1.size() && 'd' == m1.begin()->first && m3.empty();

  return b1 && b2 && b3;
}

void map_tests()
{
  static_assert(static_tests<std::map>());
  static_assert(map_test1<std::map, char, int>());
  static_assert(map_test2<std::map, char, int>());
  static_assert(map_test3<std::map, char, int>());

  assert(static_tests<std::map>());
  assert((map_test1<std::map, char, int>()));
  assert((map_test2<std::map, char, int>()));
  assert((map_test3<std::map, char, int>()));
}

void unordered_map_tests()
{
  static_assert(static_tests<std::unordered_map>());
  static_assert(map_test1<std::unordered_map, char, int>());
  static_assert(map_test2<std::unordered_map, char, int>());
  static_assert(map_test3<std::unordered_map, char, int>());

  assert(static_tests<std::unordered_map>());
  assert((map_test1<std::unordered_map, char, int>()));
  assert((map_test2<std::unordered_map, char, int>()));
  assert((map_test3<std::unordered_map, char, int>()));
}

int main(int argc, char *argv[])
{
  map_tests();
  unordered_map_tests();
  return 0;
}
