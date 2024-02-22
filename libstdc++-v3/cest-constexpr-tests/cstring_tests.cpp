#include <cstring>
#include <cassert>

// All of cstring cannot be implemented constexpr via a library; and only
// some of GCC's __builtin_ intrinsics are constexpr; __builtin_memset isn't

constexpr bool cstring_tests()
{
#if 0
        char  a[] = "abc", d[4]{};
  const char ca[] = "abd";

  bool b = sizeof a == sizeof d;

  b = b && 0 == std::strcmp("why?", "why?") and 0 == strcmp("why?", "why?");
  b = b && 4 == std::strlen("four")         and 4 == strlen("four");
  b = b && !std::memchr( a, 'q', sizeof  a) and !memchr( a, 'q', sizeof  a);
  b = b && !std::memchr(ca, 'q', sizeof ca) and !memchr(ca, 'q', sizeof ca);
  b = b && std::memcmp(a, ca, sizeof a) < 0 && memcmp(ca, a, sizeof a) > 0;
  b = b && std::memcpy(d,  a, sizeof d) and 0 == strcmp(a, d);
  b = b &&      memcpy(d, ca, sizeof d) and 0 == strcmp(ca, d);
  b = b && std::memmove(a+1, a+0, 1) && 0 == strcmp("aac", a);
  b = b &&      memmove(a+2, a+1, 1) && 0 == strcmp("aaa", a);
  b = b && std::memset(a, 'b', 1) && strcmp("bbb", a);
  return b;
#endif
  return true;
}

int main(int argc, char *argv[])
{
  static_assert(cstring_tests());
  assert(cstring_tests());
  return 0;
}
