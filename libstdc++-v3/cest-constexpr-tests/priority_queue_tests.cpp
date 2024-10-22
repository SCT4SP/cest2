#include "tests_util.hpp"
#include <cassert>
#include <queue>
#include <list>
#include <deque>

// tests copy ctor and operator=
template <typename PQ>
constexpr bool pqueue_test1()
{
  PQ q1;
  bool b = q1.empty();
  q1.push(42);
  q1.pop();
  int i = 42;
  q1.push(i);
  PQ q2 = q1;
  PQ q3;
  q3 = q2;

  return b && q1.size() == q3.size();
}

namespace pq_test
{
  struct S
  {
    constexpr S(int id, double, char) : id_{id} { }
    constexpr friend bool operator<(S const& x, S const& y) {
      return x.id_ < y.id_;
    }
    int id_{};
  };
}

template <typename PQ>
constexpr bool pqueue_test2()
{
  PQ q;
  bool b = q.empty();
  b = b && 0 == q.size();
  const int i{42};

  q.push(i); // add 42 to the back
  q.push(43); // add 43 to the back
  b = b && 2 == q.size();
  q.pop(); // remove 42 from the front
  q.pop(); // remove 43 from the front
  b = b && q.empty();

  std::priority_queue<pq_test::S> q2;
  q2.emplace(42, 3.14, 'c');
  b = b && 42 == q2.top().id_;

  return b;
}

constexpr bool doit()
{
  using namespace tests_util;

  bool b = pqueue_test1<std::priority_queue<int>>();
  b = b && pqueue_test1<std::priority_queue<int, std::deque<int>>>();

  b = b && pqueue_test2<std::priority_queue<int>>();
  b = b && pqueue_test2<std::priority_queue<int, std::deque<int>>>();

  b = b && push_dtor_test_pq<std::priority_queue<Bar<>>>();
  b = b && push_dtor_test_pq<std::priority_queue<Bar<>, std::deque<Bar<>>>>();

  return b;
}

void priority_queue_tests()
{
  assert(doit());
  static_assert(doit());
}

int main(int argc, char *argv[])
{
  priority_queue_tests();
  return 0;
}
