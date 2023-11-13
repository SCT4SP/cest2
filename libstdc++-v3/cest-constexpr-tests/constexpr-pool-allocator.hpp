#ifndef _CONSTEXPR_POOL_ALLOCATOR_
#define _CONSTEXPR_POOL_ALLOCATOR_

// Copyright (c) 2023 Paul Keir, University of the West of Scotland.
//   https://github.com/SCT4SP/constexpr-pool-allocator

#include <memory>

static_assert(__cpp_constexpr >= 202306L,
              "Compiler support for constexpr cast from void P2738 is missing");

template <class T>
struct pool_alloc
  // : public std::allocator<T> // No: clients will find allocate_at_least etc.
{
  using value_type = T;

  constexpr pool_alloc(void* p) : p_{ p } { }

  template<class U>
  constexpr pool_alloc(const pool_alloc <U>& u) noexcept : p_{ u.p_ } { }

  constexpr T* allocate(std::size_t n)
  {
    T* ret = static_cast<T*>(p_);
    p_ = ret + n;
    return ret;
  }

  constexpr void deallocate(T* p, std::size_t n) noexcept { }

  void* p_{};
};

template<class T, class U>
bool operator==(const pool_alloc<T>&, const pool_alloc<U>&) { return true; }

template<class T, class U>
bool operator!=(const pool_alloc<T>&, const pool_alloc<U>&) { return false; }

#endif // _CONSTEXPR_POOL_ALLOCATOR_

