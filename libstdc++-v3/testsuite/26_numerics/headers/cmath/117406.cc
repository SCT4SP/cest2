// Copyright (C) 1999-2024 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 3, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along
// with this library; see the file COPYING3.  If not see
// <http://www.gnu.org/licenses/>.

// { dg-do run { target c++23 } }

#include <cmath>
#include <limits>
#include <climits>
#include <stdfloat>
#include <testsuite_hooks.h>

template <typename T>
constexpr
int
test01()
{
  constexpr T pinf = std::numeric_limits<T>::infinity();
  constexpr T ninf = -pinf;

  int i1 = std::ilogb(pinf);
  VERIFY(i1 == INT_MAX);

  int i2 = std::ilogb(ninf);
  VERIFY(i2 == INT_MAX);

  T x = 42;
  int i3a = std::ilogb(x);
  int i3b{std::numeric_limits<int>::max()};
  std::frexp(x, &i3b);
  VERIFY(i3a == (i3b-1));

  return 0;
}

int
main()
{
  test01<std::float16_t>();
  test01<std::bfloat16_t>();
  return 0;
}
