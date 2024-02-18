#include <chrono>
#include <iostream>
#include <cassert>

//constexpr
bool chrono_test1()
{
  using namespace std::chrono;
  high_resolution_clock::time_point t1, t2;
  t1 = high_resolution_clock::now();
  t2 = high_resolution_clock::now();
#if 0
  if (!std::is_constant_evaluated())
  {
    double dur1 = duration_cast<duration<double>>(t2-t1).count();
    std::cout << "dur1: " << dur1 << std::endl;
  }

  steady_clock::time_point t3, t4;
  t3 = steady_clock::now();
  t4 = steady_clock::now();
  double dur2 = duration<double>(t4-t3).count();
#endif
  return true;
}

int main(int argc, char *argv[])
{
//  static_assert(chrono_test1());
  assert((chrono_test1()));

  return 0;
}

