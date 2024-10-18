#include "tests_util.hpp"
#include <cassert>
#include <queue>
#include <list>
#include <deque>

template <typename Q>
constexpr bool queue_test1()
{
  Q q;
  bool b0 = q.empty();
  auto sz0 = q.size();
  const int i{42};

  q.push(i); // add 42 to the back
  auto ef1 = q.front();
  auto eb1 = q.back();

  q.push(43); // add 43 to the back
  auto ef2 = q.front();
  auto eb2 = q.back();
  auto sz2 = q.size();

  q.pop(); // remove 42 from the front
  auto ef3 = q.front();
  auto eb3 = q.back();

  q.pop(); // remove 43 from the front
  bool b4 = q.empty();

  return b0 && sz0 == 0 && ef1 == 42 && eb1 == 42 && ef2 == 42 && eb2 == 43 &&
         sz2 == 2 && ef3 == 43 && eb3 == 43 && b4;
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
constexpr bool pqueue_test1()
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

// tests copy ctor and operator=
template <typename Q>
constexpr bool queue_test2()
{
  Q q1;
  q1.push(42);
  Q q2 = q1;
  Q q3;
  q3 = q2;
  bool b = q1.size() == q2.size();
  if constexpr (std::is_same_v<Q, std::queue<int>>)
    b = b && 42 == q3.front();

  return b;
}

constexpr bool doit()
{
  using namespace tests_util;

  bool b =  queue_test1<         std::queue<int>>();
  b = b && pqueue_test1<std::priority_queue<int>>();
  b = b &&  queue_test1<         std::queue<int, std::list<int>>>();
  b = b && pqueue_test1<std::priority_queue<int, std::deque<int>>>();

  b = b && queue_test2<         std::queue<int>>();
  b = b && queue_test2<std::priority_queue<int>>();
  b = b && queue_test2<         std::queue<int, std::list<int>>>();
  b = b && queue_test2<std::priority_queue<int, std::deque<int>>>();

  b = b && push_dtor_test<            std::queue<Bar<>>>();
  b = b && push_dtor_test_pq<std::priority_queue<Bar<>>>();
  b = b && push_dtor_test<            std::queue<Bar<>, std::list<Bar<>>>>();
  b = b && push_dtor_test_pq<std::priority_queue<Bar<>, std::deque<Bar<>>>>();

  return b;
}

void queue_tests()
{
  assert(doit());
  static_assert(doit());
}

int main(int argc, char *argv[])
{
  queue_tests();
  return 0;
}
