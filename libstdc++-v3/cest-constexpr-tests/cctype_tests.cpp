#include <cassert>
#include <cctype>
#include <limits>

void cctype_tests() {

  static_assert(std::isalnum('q') != 0);
  static_assert(std::isalnum('!') == 0);

  static_assert(std::isupper('W') != 0);
  static_assert(std::isupper('a') == 0);
  static_assert(std::isupper('!') == 0);

  static_assert(std::isspace('\n') != 0 and std::isblank('\n') == 0);

  assert(std::isalnum('q') != 0);
  assert(std::isalnum('!') == 0);

  assert(std::isupper('W') != 0);
  assert(std::isupper('a') == 0);
  assert(std::isupper('!') == 0);

  assert(std::isspace('\n') != 0 and std::isblank('\n') == 0);

  for (auto c = std::numeric_limits<char>::lowest();; ++c) {
    assert(std::isalnum(c) == std::isalnum(c));
    if (c == std::numeric_limits<char>::max())
      break;
  }
}

int main(int argc, char *argv[])
{
  cctype_tests();
  return 0;
}
