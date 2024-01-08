#include <string>
#include <cassert>

constexpr bool string_test1()
{
#ifndef __clang__
  // Clang complains of access to `_M_local_buf' of union with
  // active member '_M_allocated_capacity'
  {
    std::string str1;
    std::string str2 = "1234567890123456"; // 16 chars
    str1 = std::move(str2);
  }
#endif
  return true;
}

// Fixing the issue above by adding a call to `_M_init_local_buf()` within
// `basic_string::clear()`, produces another error from the test function
// below; from both compilers.

// The error occurs during the execution of the rvalue's destructor causes a
// complaint of read of member '_M_allocated_capacity' of union with
// active member '_M_local_buf'.
constexpr bool string_test2()
{
  {
    std::string str = "1234567890123456";  // 16 chars
    str = std::string{"1234567890123456"}; // rvalue assignment
  }

  return true;
}

int main(int argc, char *argv[])
{
  static_assert(string_test1());
  assert(string_test1());

  static_assert(string_test2());
  assert(string_test2());

  return 0;
}
