// Map iterator invalidation tests

// Copyright (C) 2003-2025 Free Software Foundation, Inc.
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

#include <debug/map>
#include <iterator>
#include <testsuite_hooks.h>

using __gnu_debug::map;
using std::advance;

bool test = true;

// Erase
void test02()
{
  map<int, int> v;
  for (int i = 0; i < 20; ++i)
    v[i] = 20-i;

  // Single element erase (middle)
  map<int, int>::iterator before = v.begin();
  map<int, int>::iterator at = before;
  advance(at, 3);
  map<int, int>::iterator after = at;
  ++after;
  v.erase(at);
  VERIFY(before._M_dereferenceable());
  VERIFY(at._M_singular());
  VERIFY(after._M_dereferenceable());

  // Multiple element erase
  before = v.begin();
  at = before;
  advance(at, 3);
  after = at;
  advance(after, 4);
  v.erase(at, after);
  VERIFY(before._M_dereferenceable());
  VERIFY(at._M_singular());

  // clear()
  before = v.begin();
  map<int, int>::iterator finish = v.end();
  VERIFY(before._M_dereferenceable());
  v.clear();
  VERIFY(before._M_singular());
  VERIFY(!finish._M_singular() && !finish._M_dereferenceable());
}

int main()
{
  test02();
  return 0;
}
