// Internal policy header for unordered_set and unordered_map -*- C++ -*-

// Copyright (C) 2010-2025 Free Software Foundation, Inc.
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

// Under Section 7 of GPL version 3, you are granted additional
// permissions described in the GCC Runtime Library Exception, version
// 3.1, as published by the Free Software Foundation.

// You should have received a copy of the GNU General Public License and
// a copy of the GCC Runtime Library Exception along with this program;
// see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
// <http://www.gnu.org/licenses/>.

/** @file bits/hashtable_policy.h
 *  This is an internal header file, included by other library headers.
 *  Do not attempt to use it directly.
 *  @headername{unordered_map,unordered_set}
 */

#ifndef _HASHTABLE_POLICY_H
#define _HASHTABLE_POLICY_H 1

#include <tuple>		// for std::tuple, std::forward_as_tuple
#include <bits/functional_hash.h> // for __is_fast_hash
#include <bits/stl_algobase.h>	// for std::min
#include <bits/stl_pair.h>	// for std::pair
#include <ext/aligned_buffer.h>	// for __gnu_cxx::__aligned_buffer
#include <ext/alloc_traits.h>	// for std::__alloc_rebind
#include <ext/numeric_traits.h>	// for __gnu_cxx::__int_traits

namespace std _GLIBCXX_VISIBILITY(default)
{
_GLIBCXX_BEGIN_NAMESPACE_VERSION
/// @cond undocumented

  template<typename _Key, typename _Value, typename _Alloc,
	   typename _ExtractKey, typename _Equal,
	   typename _Hash, typename _RangeHash, typename _Unused,
	   typename _RehashPolicy, typename _Traits>
    class _Hashtable;

namespace __detail
{
  /**
   *  @defgroup hashtable-detail Base and Implementation Classes
   *  @ingroup unordered_associative_containers
   *  @{
   */
  template<typename _Key, typename _Value, typename _ExtractKey,
	   typename _Equal, typename _Hash, typename _RangeHash,
	   typename _Unused, typename _Traits>
    struct _Hashtable_base;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wc++17-extensions" // if constexpr
  // Helper function: return distance(first, last) for forward
  // iterators, or 0/1 for input iterators.
  template<typename _Iterator>
    _GLIBCXX_CEST_CONSTEXPR
    inline typename std::iterator_traits<_Iterator>::difference_type
    __distance_fw(_Iterator __first, _Iterator __last)
    {
      using _Cat = typename std::iterator_traits<_Iterator>::iterator_category;
      if constexpr (is_convertible<_Cat, forward_iterator_tag>::value)
	return std::distance(__first, __last);
      else
	return __first != __last ? 1 : 0;
    }
#pragma GCC diagnostic pop

  struct _Identity
  {
    template<typename _Tp>
      _GLIBCXX_CEST_CONSTEXPR
      _Tp&&
      operator()(_Tp&& __x) const noexcept
      { return std::forward<_Tp>(__x); }
  };

  struct _Select1st
  {
    template<typename _Pair>
      struct __1st_type;

    template<typename _Tp, typename _Up>
      struct __1st_type<pair<_Tp, _Up>>
      { using type = _Tp; };

    template<typename _Tp, typename _Up>
      struct __1st_type<const pair<_Tp, _Up>>
      { using type = const _Tp; };

    template<typename _Pair>
      struct __1st_type<_Pair&>
      { using type = typename __1st_type<_Pair>::type&; };

    template<typename _Tp>
      _GLIBCXX_CEST_CONSTEXPR
      typename __1st_type<_Tp>::type&&
      operator()(_Tp&& __x) const noexcept
      { return std::forward<_Tp>(__x).first; }
  };

  template<typename _ExKey>
    struct _NodeBuilder;

  template<>
    struct _NodeBuilder<_Select1st>
    {
      template<typename _Kt, typename _Arg, typename _NodeGenerator>
	_GLIBCXX_CEST_CONSTEXPR
	static auto
	_S_build(_Kt&& __k, _Arg&& __arg, _NodeGenerator& __node_gen)
	-> typename _NodeGenerator::__node_ptr
	{
	  return __node_gen(std::forward<_Kt>(__k),
			    std::forward<_Arg>(__arg).second);
	}
    };

  template<>
    struct _NodeBuilder<_Identity>
    {
      template<typename _Kt, typename _Arg, typename _NodeGenerator>
	_GLIBCXX_CEST_CONSTEXPR
	static auto
	_S_build(_Kt&& __k, _Arg&&, _NodeGenerator& __node_gen)
	-> typename _NodeGenerator::__node_ptr
	{ return __node_gen(std::forward<_Kt>(__k)); }
    };

  template<typename _HashtableAlloc, typename _NodePtr>
    struct _NodePtrGuard
    {
      _HashtableAlloc& _M_h;
      _NodePtr _M_ptr;

      _GLIBCXX_CEST_CONSTEXPR
      ~_NodePtrGuard()
      {
	if (_M_ptr)
	  _M_h._M_deallocate_node_ptr(_M_ptr);
      }
    };

  template<typename _NodeAlloc>
    struct _Hashtable_alloc;

  // Functor recycling a pool of nodes and using allocation once the pool is
  // empty.
  template<typename _NodeAlloc>
    struct _ReuseOrAllocNode
    {
    private:
      using __node_alloc_type = _NodeAlloc;
      using __hashtable_alloc = _Hashtable_alloc<__node_alloc_type>;
      using __node_alloc_traits =
	typename __hashtable_alloc::__node_alloc_traits;

    public:
      using __node_ptr = typename __hashtable_alloc::__node_ptr;

      _GLIBCXX_CEST_CONSTEXPR
      _ReuseOrAllocNode(__node_ptr __nodes, __hashtable_alloc& __h)
      : _M_nodes(__nodes), _M_h(__h) { }
      _ReuseOrAllocNode(const _ReuseOrAllocNode&) = delete;

      _GLIBCXX_CEST_CONSTEXPR
      ~_ReuseOrAllocNode()
      { _M_h._M_deallocate_nodes(_M_nodes); }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wc++17-extensions" // if constexpr
      template<typename _Arg>
	_GLIBCXX_CEST_CONSTEXPR
	__node_ptr
	operator()(_Arg&& __arg)
	{
	  if (!_M_nodes)
	    return _M_h._M_allocate_node(std::forward<_Arg>(__arg));

	  using value_type = typename _NodeAlloc::value_type::value_type;

	  __node_ptr __node = _M_nodes;
	  if constexpr (is_assignable<value_type&, _Arg>::value)
	    {
	      __node->_M_v() = std::forward<_Arg>(__arg);
	      _M_nodes = _M_nodes->_M_next();
	      __node->_M_nxt = nullptr;
	    }
	  else
	    {
	      _M_nodes = _M_nodes->_M_next();
	      __node->_M_nxt = nullptr;
	      auto& __a = _M_h._M_node_allocator();
	      __node_alloc_traits::destroy(__a, __node->_M_valptr());
	      _NodePtrGuard<__hashtable_alloc, __node_ptr>
		__guard{ _M_h, __node };
	      __node_alloc_traits::construct(__a, __node->_M_valptr(),
					     std::forward<_Arg>(__arg));
	      __guard._M_ptr = nullptr;
	    }
	  return __node;
	}
#pragma GCC diagnostic pop

    private:
      __node_ptr _M_nodes;
      __hashtable_alloc& _M_h;
    };

  // Functor similar to the previous one but without any pool of nodes to
  // recycle.
  template<typename _NodeAlloc>
    struct _AllocNode
    {
    private:
      using __hashtable_alloc = _Hashtable_alloc<_NodeAlloc>;

    public:
      using __node_ptr = typename __hashtable_alloc::__node_ptr;

      _GLIBCXX_CEST_CONSTEXPR
      _AllocNode(__hashtable_alloc& __h)
      : _M_h(__h) { }

      template<typename... _Args>
	_GLIBCXX_CEST_CONSTEXPR
	__node_ptr
	operator()(_Args&&... __args) const
	{ return _M_h._M_allocate_node(std::forward<_Args>(__args)...); }

    private:
      __hashtable_alloc& _M_h;
    };

  // Auxiliary types used for all instantiations of _Hashtable nodes
  // and iterators.

  /**
   *  struct _Hashtable_traits
   *
   *  Important traits for hash tables.
   *
   *  @tparam _Cache_hash_code  Boolean value. True if the value of
   *  the hash function is stored along with the value. This is a
   *  time-space tradeoff.  Storing it may improve lookup speed by
   *  reducing the number of times we need to call the _Hash or _Equal
   *  functors.
   *
   *  @tparam _Constant_iterators  Boolean value. True if iterator and
   *  const_iterator are both constant iterator types. This is true
   *  for unordered_set and unordered_multiset, false for
   *  unordered_map and unordered_multimap.
   *
   *  @tparam _Unique_keys  Boolean value. True if the return value
   *  of _Hashtable::count(k) is always at most one, false if it may
   *  be an arbitrary number. This is true for unordered_set and
   *  unordered_map, false for unordered_multiset and
   *  unordered_multimap.
   */
  template<bool _Cache_hash_code, bool _Constant_iterators, bool _Unique_keys>
    struct _Hashtable_traits
    {
      using __hash_cached = __bool_constant<_Cache_hash_code>;
      using __constant_iterators = __bool_constant<_Constant_iterators>;
      using __unique_keys = __bool_constant<_Unique_keys>;
    };

  /**
   *  struct _Hashtable_hash_traits
   *
   *  Important traits for hash tables depending on associated hasher.
   *
   */
  template<typename _Hash>
    struct _Hashtable_hash_traits
    {
      static constexpr size_t
      __small_size_threshold() noexcept
      { return std::__is_fast_hash<_Hash>::value ? 0 : 20; }
    };

  /**
   *  struct _Hash_node_base
   *
   *  Nodes, used to wrap elements stored in the hash table.  A policy
   *  template parameter of class template _Hashtable controls whether
   *  nodes also store a hash code. In some cases (e.g. strings) this
   *  may be a performance win.
   */
  struct _Hash_node_base
  {
    _Hash_node_base* _M_nxt;

    _GLIBCXX_CEST_CONSTEXPR
    _Hash_node_base() noexcept : _M_nxt() { }

    _GLIBCXX_CEST_CONSTEXPR
    _Hash_node_base(_Hash_node_base* __next) noexcept : _M_nxt(__next) { }
  };

  /**
   *  struct _Hash_node_value_base
   *
   *  Node type with the value to store.
   */
  template<typename _Value>
    struct _Hash_node_value_base
    {
      using value_type = _Value;

      __gnu_cxx::__aligned_buffer<_Value> _M_storage;

      // These member functions must be always_inline, see PR 111050

      [[__gnu__::__always_inline__]]
      _GLIBCXX_CEST_CONSTEXPR
      _Value*
      _M_valptr() noexcept
      { return _M_storage._M_ptr(); }

      [[__gnu__::__always_inline__]]
      _GLIBCXX_CEST_CONSTEXPR
      const _Value*
      _M_valptr() const noexcept
      { return _M_storage._M_ptr(); }

      [[__gnu__::__always_inline__]]
      _GLIBCXX_CEST_CONSTEXPR
      _Value&
      _M_v() noexcept
      { return *_M_valptr(); }

      [[__gnu__::__always_inline__]]
      _GLIBCXX_CEST_CONSTEXPR
      const _Value&
      _M_v() const noexcept
      { return *_M_valptr(); }
    };

  /**
   *  Primary template struct _Hash_node_code_cache.
   */
  template<bool _Cache_hash_code>
    struct _Hash_node_code_cache
    { };

  /**
   *  Specialization for node with cache, struct _Hash_node_code_cache.
   */
  template<>
    struct _Hash_node_code_cache<true>
    { size_t  _M_hash_code; };

  template<typename _Value, bool _Cache_hash_code>
    struct _Hash_node_value
    : _Hash_node_value_base<_Value>
    , _Hash_node_code_cache<_Cache_hash_code>
    { };

  /**
   *  Primary template struct _Hash_node.
   */
  template<typename _Value, bool _Cache_hash_code>
    struct _Hash_node
    : _Hash_node_base
    , _Hash_node_value<_Value, _Cache_hash_code>
    {
      _GLIBCXX_CEST_CONSTEXPR
      _Hash_node*
      _M_next() const noexcept
      { return static_cast<_Hash_node*>(this->_M_nxt); }
    };

  /// Base class for node iterators.
  template<typename _Value, bool _Cache_hash_code>
    struct _Node_iterator_base
    {
      using __node_type = _Hash_node<_Value, _Cache_hash_code>;

      __node_type* _M_cur;

      _Node_iterator_base() : _M_cur(nullptr) { }
      _GLIBCXX_CEST_CONSTEXPR
      _Node_iterator_base(__node_type* __p) noexcept
      : _M_cur(__p) { }

      _GLIBCXX_CEST_CONSTEXPR
      void
      _M_incr() noexcept
      { _M_cur = _M_cur->_M_next(); }

      _GLIBCXX_CEST_CONSTEXPR
      friend bool
      operator==(const _Node_iterator_base& __x, const _Node_iterator_base& __y)
      noexcept
      { return __x._M_cur == __y._M_cur; }

#if __cpp_impl_three_way_comparison < 201907L
      friend bool
      operator!=(const _Node_iterator_base& __x, const _Node_iterator_base& __y)
      noexcept
      { return __x._M_cur != __y._M_cur; }
#endif
    };

  /// Node iterators, used to iterate through all the hashtable.
  template<typename _Value, bool __constant_iterators, bool __cache>
    struct _Node_iterator
    : public _Node_iterator_base<_Value, __cache>
    {
    private:
      using __base_type = _Node_iterator_base<_Value, __cache>;
      using __node_type = typename __base_type::__node_type;

    public:
      using value_type        = _Value;
      using difference_type   = ptrdiff_t;
      using iterator_category = forward_iterator_tag;

      using pointer = __conditional_t<__constant_iterators,
				      const value_type*, value_type*>;

      using reference = __conditional_t<__constant_iterators,
					const value_type&, value_type&>;

      _Node_iterator() = default;

      _GLIBCXX_CEST_CONSTEXPR
      explicit
      _Node_iterator(__node_type* __p) noexcept
      : __base_type(__p) { }

      _GLIBCXX_CEST_CONSTEXPR
      reference
      operator*() const noexcept
      { return this->_M_cur->_M_v(); }

      _GLIBCXX_CEST_CONSTEXPR
      pointer
      operator->() const noexcept
      { return this->_M_cur->_M_valptr(); }

      _GLIBCXX_CEST_CONSTEXPR
      _Node_iterator&
      operator++() noexcept
      {
	this->_M_incr();
	return *this;
      }

      _GLIBCXX_CEST_CONSTEXPR
      _Node_iterator
      operator++(int) noexcept
      {
	_Node_iterator __tmp(*this);
	this->_M_incr();
	return __tmp;
      }

#if __cpp_impl_three_way_comparison >= 201907L
      _GLIBCXX_CEST_CONSTEXPR
      friend bool
      operator==(const _Node_iterator&, const _Node_iterator&) = default;
#else
      friend bool
      operator==(const _Node_iterator& __x, const _Node_iterator& __y) noexcept
      {
	const __base_type& __bx = __x;
	const __base_type& __by = __y;
	return __bx == __by;
      }

      friend bool
      operator!=(const _Node_iterator& __x, const _Node_iterator& __y) noexcept
      { return !(__x == __y); }
#endif
    };

  /// Node const_iterators, used to iterate through all the hashtable.
  template<typename _Value, bool __constant_iterators, bool __cache>
    struct _Node_const_iterator
    : public _Node_iterator_base<_Value, __cache>
    {
    private:
      using __base_type = _Node_iterator_base<_Value, __cache>;
      using __node_type = typename __base_type::__node_type;

      // The corresponding non-const iterator.
      using __iterator
	= _Node_iterator<_Value, __constant_iterators, __cache>;

    public:
      using value_type        = _Value;
      using difference_type   = ptrdiff_t;
      using iterator_category = forward_iterator_tag;

      using pointer = const value_type*;
      using reference = const value_type&;

      _Node_const_iterator() = default;

      _GLIBCXX_CEST_CONSTEXPR
      explicit
      _Node_const_iterator(__node_type* __p) noexcept
      : __base_type(__p) { }

      _GLIBCXX_CEST_CONSTEXPR
      _Node_const_iterator(const __iterator& __x) noexcept
      : __base_type(__x._M_cur) { }

      _GLIBCXX_CEST_CONSTEXPR
      reference
      operator*() const noexcept
      { return this->_M_cur->_M_v(); }

      pointer
      operator->() const noexcept
      { return this->_M_cur->_M_valptr(); }

      _GLIBCXX_CEST_CONSTEXPR
      _Node_const_iterator&
      operator++() noexcept
      {
	this->_M_incr();
	return *this;
      }

      _GLIBCXX_CEST_CONSTEXPR
      _Node_const_iterator
      operator++(int) noexcept
      {
	_Node_const_iterator __tmp(*this);
	this->_M_incr();
	return __tmp;
      }

#if __cpp_impl_three_way_comparison >= 201907L
      friend bool
      operator==(const _Node_const_iterator&,
		 const _Node_const_iterator&) = default;

      _GLIBCXX_CEST_CONSTEXPR
      friend bool
      operator==(const _Node_const_iterator& __x, const __iterator& __y)
      {
	const __base_type& __bx = __x;
	const __base_type& __by = __y;
	return __bx == __by;
      }
#else
      friend bool
      operator==(const _Node_const_iterator& __x,
		 const _Node_const_iterator& __y) noexcept
      {
	const __base_type& __bx = __x;
	const __base_type& __by = __y;
	return __bx == __by;
      }

      friend bool
      operator!=(const _Node_const_iterator& __x,
		 const _Node_const_iterator& __y) noexcept
      { return !(__x == __y); }

      friend bool
      operator==(const _Node_const_iterator& __x,
		 const __iterator& __y) noexcept
      {
	const __base_type& __bx = __x;
	const __base_type& __by = __y;
	return __bx == __by;
      }

      friend bool
      operator!=(const _Node_const_iterator& __x,
		 const __iterator& __y) noexcept
      { return !(__x == __y); }

      friend bool
      operator==(const __iterator& __x,
		 const _Node_const_iterator& __y) noexcept
      {
	const __base_type& __bx = __x;
	const __base_type& __by = __y;
	return __bx == __by;
      }

      friend bool
      operator!=(const __iterator& __x,
		 const _Node_const_iterator& __y) noexcept
      { return !(__x == __y); }
#endif
    };

  // Many of class template _Hashtable's template parameters are policy
  // classes.  These are defaults for the policies.

  /// Default range hashing function: use division to fold a large number
  /// into the range [0, N).
  struct _Mod_range_hashing
  {
    _GLIBCXX_CEST_CONSTEXPR
    size_t
    operator()(size_t __num, size_t __den) const noexcept
    { return __num % __den; }
  };

  /// Default ranged hash function H.  In principle it should be a
  /// function object composed from objects of type H1 and H2 such that
  /// h(k, N) = h2(h1(k), N), but that would mean making extra copies of
  /// h1 and h2.  So instead we'll just use a tag to tell class template
  /// hashtable to do that composition.
  struct _Default_ranged_hash { };

  /// Default value for rehash policy.  Bucket size is (usually) the
  /// smallest prime that keeps the load factor small enough.
  struct _Prime_rehash_policy
  {
    using __has_load_factor = true_type;

    _GLIBCXX_CEST_CONSTEXPR
    _Prime_rehash_policy(float __z = 1.0) noexcept
    : _M_max_load_factor(__z), _M_next_resize(0) { }

    float
    max_load_factor() const noexcept
    { return _M_max_load_factor; }

    // Return a bucket size no smaller than n.
    // TODO: 'const' qualifier is kept for abi compatibility reason.
    _GLIBCXX_CEST_CONSTEXPR
    size_t
    _M_next_bkt(size_t __n) const;

    // Return a bucket count appropriate for n elements
    _GLIBCXX_CEST_CONSTEXPR
    size_t
    _M_bkt_for_elements(size_t __n) const
#if _GLIBCXX_CEST_VERSION && defined(__clang__)
    {
      auto __builtin_ceil = [](std::floating_point auto f) -> decltype(f) {
          return static_cast<int>(f <= 0 ? f : f + 1);
      };
    }
#else
    { return __builtin_ceil(__n / (double)_M_max_load_factor); }
#endif

    // __n_bkt is current bucket count, __n_elt is current element count,
    // and __n_ins is number of elements to be inserted.  Do we need to
    // increase bucket count?  If so, return make_pair(true, n), where n
    // is the new bucket count.  If not, return make_pair(false, 0).
    // TODO: 'const' qualifier is kept for abi compatibility reason.
    _GLIBCXX_CEST_CONSTEXPR
    std::pair<bool, size_t>
    _M_need_rehash(size_t __n_bkt, size_t __n_elt,
		   size_t __n_ins) const;

    using _State = size_t;

    _GLIBCXX_CEST_CONSTEXPR
    _State
    _M_state() const
    { return _M_next_resize; }

    _GLIBCXX_CEST_CONSTEXPR
    void
    _M_reset() noexcept
    { _M_next_resize = 0; }

    void
    _M_reset(_State __state)
    { _M_next_resize = __state; }

    static const size_t _S_growth_factor = 2;

    float		_M_max_load_factor;

    // TODO: 'mutable' kept for abi compatibility reason.
    mutable size_t	_M_next_resize;
  };

  /// Range hashing function assuming that second arg is a power of 2.
  struct _Mask_range_hashing
  {
    size_t
    operator()(size_t __num, size_t __den) const noexcept
    { return __num & (__den - 1); }
  };

  /// Compute closest power of 2 not less than __n
  inline size_t
  __clp2(size_t __n) noexcept
  {
    using __gnu_cxx::__int_traits;
    // Equivalent to return __n ? std::bit_ceil(__n) : 0;
    if (__n < 2)
      return __n;
    const unsigned __lz = sizeof(size_t) > sizeof(long)
      ? __builtin_clzll(__n - 1ull)
      : __builtin_clzl(__n - 1ul);
    // Doing two shifts avoids undefined behaviour when __lz == 0.
    return (size_t(1) << (__int_traits<size_t>::__digits - __lz - 1)) << 1;
  }

  /// Rehash policy providing power of 2 bucket numbers. Avoids modulo
  /// operations.
  struct _Power2_rehash_policy
  {
    using __has_load_factor = true_type;

    _Power2_rehash_policy(float __z = 1.0) noexcept
    : _M_max_load_factor(__z), _M_next_resize(0) { }

    float
    max_load_factor() const noexcept
    { return _M_max_load_factor; }

    // Return a bucket size no smaller than n (as long as n is not above the
    // highest power of 2).
    _GLIBCXX_CEST_CONSTEXPR
    size_t
    _M_next_bkt(size_t __n) noexcept
    {
      if (__n == 0)
	// Special case on container 1st initialization with 0 bucket count
	// hint. We keep _M_next_resize to 0 to make sure that next time we
	// want to add an element allocation will take place.
	return 1;

      const auto __max_width = std::min<size_t>(sizeof(size_t), 8);
      const auto __max_bkt = size_t(1) << (__max_width * __CHAR_BIT__ - 1);
      size_t __res = __clp2(__n);

      if (__res == 0)
	__res = __max_bkt;
      else if (__res == 1)
	// If __res is 1 we force it to 2 to make sure there will be an
	// allocation so that nothing need to be stored in the initial
	// single bucket
	__res = 2;

      if (__res == __max_bkt)
	// Set next resize to the max value so that we never try to rehash again
	// as we already reach the biggest possible bucket number.
	// Note that it might result in max_load_factor not being respected.
	_M_next_resize = size_t(-1);
      else
	_M_next_resize
	  = __builtin_floor(__res * (double)_M_max_load_factor);

      return __res;
    }

    // Return a bucket count appropriate for n elements
    size_t
    _M_bkt_for_elements(size_t __n) const noexcept
    { return __builtin_ceil(__n / (double)_M_max_load_factor); }

    // __n_bkt is current bucket count, __n_elt is current element count,
    // and __n_ins is number of elements to be inserted.  Do we need to
    // increase bucket count?  If so, return make_pair(true, n), where n
    // is the new bucket count.  If not, return make_pair(false, 0).
    _GLIBCXX_CEST_CONSTEXPR
    std::pair<bool, size_t>
    _M_need_rehash(size_t __n_bkt, size_t __n_elt, size_t __n_ins) noexcept
    {
      if (__n_elt + __n_ins > _M_next_resize)
	{
	  // If _M_next_resize is 0 it means that we have nothing allocated so
	  // far and that we start inserting elements. In this case we start
	  // with an initial bucket size of 11.
	  double __min_bkts
	    = std::max<size_t>(__n_elt + __n_ins, _M_next_resize ? 0 : 11)
	      / (double)_M_max_load_factor;
	  if (__min_bkts >= __n_bkt)
	    return { true,
	      _M_next_bkt(std::max<size_t>(__builtin_floor(__min_bkts) + 1,
					   __n_bkt * _S_growth_factor)) };

	  _M_next_resize
	    = __builtin_floor(__n_bkt * (double)_M_max_load_factor);
	  return { false, 0 };
	}
      else
	return { false, 0 };
    }

    using _State = size_t;

    _State
    _M_state() const noexcept
    { return _M_next_resize; }

    void
    _M_reset() noexcept
    { _M_next_resize = 0; }

    void
    _M_reset(_State __state) noexcept
    { _M_next_resize = __state; }

    static const size_t _S_growth_factor = 2;

    float	_M_max_load_factor;
    size_t	_M_next_resize;
  };

  template<typename _RehashPolicy>
    struct _RehashStateGuard
    {
      _RehashPolicy* _M_guarded_obj;
      typename _RehashPolicy::_State _M_prev_state;

      _GLIBCXX_CEST_CONSTEXPR
      _RehashStateGuard(_RehashPolicy& __policy)
      : _M_guarded_obj(std::__addressof(__policy))
      , _M_prev_state(__policy._M_state())
      { }
      _RehashStateGuard(const _RehashStateGuard&) = delete;

      _GLIBCXX_CEST_CONSTEXPR
      ~_RehashStateGuard()
      {
	if (_M_guarded_obj)
	  _M_guarded_obj->_M_reset(_M_prev_state);
      }
    };

  // Base classes for std::_Hashtable.  We define these base classes
  // because in some cases we want to do different things depending on
  // the value of a policy class.  In some cases the policy class
  // affects which member functions and nested typedefs are defined;
  // we handle that by specializing base class templates.  Several of
  // the base class templates need to access other members of class
  // template _Hashtable, so we use a variant of the "Curiously
  // Recurring Template Pattern" (CRTP) technique.

  /**
   *  Primary class template _Map_base.
   *
   *  If the hashtable has a value type of the form pair<const T1, T2> and
   *  a key extraction policy (_ExtractKey) that returns the first part
   *  of the pair, the hashtable gets a mapped_type typedef.  If it
   *  satisfies those criteria and also has unique keys, then it also
   *  gets an operator[].
   */
  template<typename _Key, typename _Value, typename _Alloc,
	   typename _ExtractKey, typename _Equal,
	   typename _Hash, typename _RangeHash, typename _Unused,
	   typename _RehashPolicy, typename _Traits,
	   bool _Unique_keys = _Traits::__unique_keys::value>
    struct _Map_base { };

  /// Partial specialization, __unique_keys set to false, std::pair value type.
  template<typename _Key, typename _Val, typename _Alloc, typename _Equal,
	   typename _Hash, typename _RangeHash, typename _Unused,
	   typename _RehashPolicy, typename _Traits>
    struct _Map_base<_Key, pair<const _Key, _Val>, _Alloc, _Select1st, _Equal,
		     _Hash, _RangeHash, _Unused, _RehashPolicy, _Traits, false>
    {
      using mapped_type = _Val;
    };

  /// Partial specialization, __unique_keys set to true.
  template<typename _Key, typename _Val, typename _Alloc, typename _Equal,
	   typename _Hash, typename _RangeHash, typename _Unused,
	   typename _RehashPolicy, typename _Traits>
    struct _Map_base<_Key, pair<const _Key, _Val>, _Alloc, _Select1st, _Equal,
		     _Hash, _RangeHash, _Unused, _RehashPolicy, _Traits, true>
    {
    private:
      using __hashtable_base = _Hashtable_base<_Key, pair<const _Key, _Val>,
					       _Select1st, _Equal, _Hash,
					       _RangeHash, _Unused,
					       _Traits>;

      using __hashtable = _Hashtable<_Key, pair<const _Key, _Val>, _Alloc,
				     _Select1st, _Equal, _Hash, _RangeHash,
				     _Unused, _RehashPolicy, _Traits>;

      using __hash_code = typename __hashtable_base::__hash_code;

    public:
      using key_type = typename __hashtable_base::key_type;
      using mapped_type = _Val;

      mapped_type&
      operator[](const key_type& __k);

      mapped_type&
      operator[](key_type&& __k);

      // _GLIBCXX_RESOLVE_LIB_DEFECTS
      // DR 761. unordered_map needs an at() member function.
      mapped_type&
      at(const key_type& __k)
      {
	auto __ite = static_cast<__hashtable*>(this)->find(__k);
	if (!__ite._M_cur)
	  __throw_out_of_range(__N("unordered_map::at"));
	return __ite->second;
      }

      const mapped_type&
      at(const key_type& __k) const
      {
	auto __ite = static_cast<const __hashtable*>(this)->find(__k);
	if (!__ite._M_cur)
	  __throw_out_of_range(__N("unordered_map::at"));
	return __ite->second;
      }
    };

  template<typename _Key, typename _Val, typename _Alloc, typename _Equal,
	   typename _Hash, typename _RangeHash, typename _Unused,
	   typename _RehashPolicy, typename _Traits>
    auto
    _Map_base<_Key, pair<const _Key, _Val>, _Alloc, _Select1st, _Equal,
	      _Hash, _RangeHash, _Unused, _RehashPolicy, _Traits, true>::
    operator[](const key_type& __k)
    -> mapped_type&
    {
      __hashtable* __h = static_cast<__hashtable*>(this);
      __hash_code __code = __h->_M_hash_code(__k);
      size_t __bkt = __h->_M_bucket_index(__code);
      if (auto __node = __h->_M_find_node(__bkt, __k, __code))
	return __node->_M_v().second;

      typename __hashtable::_Scoped_node __node {
	__h,
	std::piecewise_construct,
	std::tuple<const key_type&>(__k),
	std::tuple<>()
      };
      auto __pos
	= __h->_M_insert_unique_node(__bkt, __code, __node._M_node);
      __node._M_node = nullptr;
      return __pos->second;
    }

  template<typename _Key, typename _Val, typename _Alloc, typename _Equal,
	   typename _Hash, typename _RangeHash, typename _Unused,
	   typename _RehashPolicy, typename _Traits>
    auto
    _Map_base<_Key, pair<const _Key, _Val>, _Alloc, _Select1st, _Equal,
	      _Hash, _RangeHash, _Unused, _RehashPolicy, _Traits, true>::
    operator[](key_type&& __k)
    -> mapped_type&
    {
      __hashtable* __h = static_cast<__hashtable*>(this);
      __hash_code __code = __h->_M_hash_code(__k);
      size_t __bkt = __h->_M_bucket_index(__code);
      if (auto __node = __h->_M_find_node(__bkt, __k, __code))
	return __node->_M_v().second;

      typename __hashtable::_Scoped_node __node {
	__h,
	std::piecewise_construct,
	std::forward_as_tuple(std::move(__k)),
	std::tuple<>()
      };
      auto __pos
	= __h->_M_insert_unique_node(__bkt, __code, __node._M_node);
      __node._M_node = nullptr;
      return __pos->second;
    }

  // Partial specialization for unordered_map<const T, U>, see PR 104174.
  template<typename _Key, typename _Val, typename _Alloc, typename _Equal,
	   typename _Hash, typename _RangeHash, typename _Unused,
	   typename _RehashPolicy, typename _Traits, bool __uniq>
    struct _Map_base<const _Key, pair<const _Key, _Val>,
		     _Alloc, _Select1st, _Equal, _Hash,
		     _RangeHash, _Unused, _RehashPolicy, _Traits, __uniq>
    : _Map_base<_Key, pair<const _Key, _Val>, _Alloc, _Select1st, _Equal, _Hash,
		_RangeHash, _Unused, _RehashPolicy, _Traits, __uniq>
    { };

  template<typename _Policy>
    using __has_load_factor = typename _Policy::__has_load_factor;

  /**
   *  Primary class template  _Rehash_base.
   *
   *  Give hashtable the max_load_factor functions and reserve iff the
   *  rehash policy supports it.
  */
  template<typename _Key, typename _Value, typename _Alloc,
	   typename _ExtractKey, typename _Equal,
	   typename _Hash, typename _RangeHash, typename _Unused,
	   typename _RehashPolicy, typename _Traits,
	   typename =
	     __detected_or_t<false_type, __has_load_factor, _RehashPolicy>>
    struct _Rehash_base;

  /// Specialization when rehash policy doesn't provide load factor management.
  template<typename _Key, typename _Value, typename _Alloc,
	   typename _ExtractKey, typename _Equal,
	   typename _Hash, typename _RangeHash, typename _Unused,
	   typename _RehashPolicy, typename _Traits>
    struct _Rehash_base<_Key, _Value, _Alloc, _ExtractKey, _Equal,
			_Hash, _RangeHash, _Unused, _RehashPolicy, _Traits,
			false_type /* Has load factor */>
    {
    };

  /// Specialization when rehash policy provide load factor management.
  template<typename _Key, typename _Value, typename _Alloc,
	   typename _ExtractKey, typename _Equal,
	   typename _Hash, typename _RangeHash, typename _Unused,
	   typename _RehashPolicy, typename _Traits>
    struct _Rehash_base<_Key, _Value, _Alloc, _ExtractKey, _Equal,
			_Hash, _RangeHash, _Unused, _RehashPolicy, _Traits,
			true_type /* Has load factor */>
    {
    private:
      using __hashtable = _Hashtable<_Key, _Value, _Alloc, _ExtractKey,
				     _Equal, _Hash, _RangeHash, _Unused,
				     _RehashPolicy, _Traits>;

    public:
      float
      max_load_factor() const noexcept
      {
	const __hashtable* __this = static_cast<const __hashtable*>(this);
	return __this->__rehash_policy().max_load_factor();
      }

      void
      max_load_factor(float __z)
      {
	__hashtable* __this = static_cast<__hashtable*>(this);
	__this->__rehash_policy(_RehashPolicy(__z));
      }

      _GLIBCXX_CEST_CONSTEXPR
      void
      reserve(size_t __n)
      {
	__hashtable* __this = static_cast<__hashtable*>(this);
	__this->rehash(__this->__rehash_policy()._M_bkt_for_elements(__n));
      }
    };

  /**
   *  Primary class template _Hashtable_ebo_helper.
   *
   *  Helper class using [[no_unique_address]] to reduce object size.
   */
  template<typename _Tp,
	   bool __use_ebo = !__is_final(_Tp) && __is_empty(_Tp)>
    struct _Hashtable_ebo_helper
    {
      [[__no_unique_address__]] _Tp _M_obj;
    };

#if ! _GLIBCXX_INLINE_VERSION
  // For ABI compatibility reasons, [[no_unique_address]] is only used
  // for empty non-final types.
  template<typename _Tp>
    struct _Hashtable_ebo_helper<_Tp, false>
    {
      _Tp _M_obj;
    };
#endif

  /**
   *  Primary class template _Local_iterator_base.
   *
   *  Base class for local iterators, used to iterate within a bucket
   *  but not between buckets.
   */
  template<typename _Key, typename _Value, typename _ExtractKey,
	   typename _Hash, typename _RangeHash, typename _Unused,
	   bool __cache_hash_code>
    struct _Local_iterator_base;

  // Wraps the _Hash object and provides some utility functions for using it.
  template<typename _Key, typename _Value, typename _ExtractKey,
	   typename _Hash, typename _RangeHash, typename _Unused,
	   bool /* __cache_hash_code */>
    struct _Hash_code_base
    {
      // Gives the local iterator implementation access to _M_bucket_index().
      friend struct _Local_iterator_base<_Key, _Value, _ExtractKey,
					 _Hash, _RangeHash, _Unused, false>;
    public:
      using hasher = _Hash;

      _GLIBCXX_CEST_CONSTEXPR
      hasher
      hash_function() const
      { return _M_hash._M_obj; }

    protected:
      [[__no_unique_address__]] _Hashtable_ebo_helper<_Hash> _M_hash{};

      using __hash_code = size_t;

      // We need the default constructor for the local iterators and _Hashtable
      // default constructor.
      _Hash_code_base() = default;

      _GLIBCXX_CEST_CONSTEXPR
      _Hash_code_base(const _Hash& __hash) : _M_hash{__hash} { }

      _GLIBCXX_CEST_CONSTEXPR
      __hash_code
      _M_hash_code(const _Key& __k) const
      {
	static_assert(__is_invocable<const _Hash&, const _Key&>{},
	    "hash function must be invocable with an argument of key type");
	return _M_hash._M_obj(__k);
      }

      template<typename _Kt>
	_GLIBCXX_CEST_CONSTEXPR
	__hash_code
	_M_hash_code_tr(const _Kt& __k) const
	{
	  static_assert(__is_invocable<const _Hash&, const _Kt&>{},
	    "hash function must be invocable with an argument of key type");
	  return _M_hash._M_obj(__k);
	}

      _GLIBCXX_CEST_CONSTEXPR
      __hash_code
      _M_hash_code(const _Hash_node_value<_Value, false>& __n) const
      { return _M_hash_code(_ExtractKey{}(__n._M_v())); }

      __hash_code
      _M_hash_code(const _Hash_node_value<_Value, true>& __n) const
      { return __n._M_hash_code; }

      _GLIBCXX_CEST_CONSTEXPR
      size_t
      _M_bucket_index(__hash_code __c, size_t __bkt_count) const
      { return _RangeHash{}(__c, __bkt_count); }

      _GLIBCXX_CEST_CONSTEXPR
      size_t
      _M_bucket_index(const _Hash_node_value<_Value, false>& __n,
		      size_t __bkt_count) const
      noexcept( noexcept(declval<const _Hash&>()(declval<const _Key&>())) )
      {
	return _RangeHash{}(_M_hash_code(_ExtractKey{}(__n._M_v())),
			    __bkt_count);
      }

      _GLIBCXX_CEST_CONSTEXPR
      size_t
      _M_bucket_index(const _Hash_node_value<_Value, true>& __n,
		      size_t __bkt_count) const noexcept
      { return _RangeHash{}(__n._M_hash_code, __bkt_count); }

      _GLIBCXX_CEST_CONSTEXPR
      void
      _M_store_code(_Hash_node_code_cache<false>&, __hash_code) const
      { }

      _GLIBCXX_CEST_CONSTEXPR
      void
      _M_copy_code(_Hash_node_code_cache<false>&,
		   const _Hash_node_code_cache<false>&) const
      { }

      _GLIBCXX_CEST_CONSTEXPR
      void
      _M_store_code(_Hash_node_code_cache<true>& __n, __hash_code __c) const
      { __n._M_hash_code = __c; }

      void
      _M_copy_code(_Hash_node_code_cache<true>& __to,
		   const _Hash_node_code_cache<true>& __from) const
      { __to._M_hash_code = __from._M_hash_code; }

      _GLIBCXX_CEST_CONSTEXPR
      void
      _M_swap(_Hash_code_base& __x)
      { std::swap(__ebo_hash::_M_get(), __x.__ebo_hash::_M_get()); }

      _GLIBCXX_CEST_CONSTEXPR
      const _Hash&
      _M_hash() const { return __ebo_hash::_M_cget(); }
=======
>>>>>>> master
    };

  /// Partial specialization used when nodes contain a cached hash code.
  template<typename _Key, typename _Value, typename _ExtractKey,
	   typename _Hash, typename _RangeHash, typename _Unused>
    struct _Local_iterator_base<_Key, _Value, _ExtractKey,
				_Hash, _RangeHash, _Unused, true>
    : public _Node_iterator_base<_Value, true>
    {
    protected:
      using __base_node_iter = _Node_iterator_base<_Value, true>;
      using __hash_code_base = _Hash_code_base<_Key, _Value, _ExtractKey,
					      _Hash, _RangeHash, _Unused, true>;

      _Local_iterator_base() = default;

      _Local_iterator_base(const __hash_code_base&,
			   _Hash_node<_Value, true>* __p,
			   size_t __bkt, size_t __bkt_count)
      : __base_node_iter(__p), _M_bucket(__bkt), _M_bucket_count(__bkt_count)
      { }

      void
      _M_incr()
      {
	__base_node_iter::_M_incr();
	if (this->_M_cur)
	  {
	    size_t __bkt
	      = _RangeHash{}(this->_M_cur->_M_hash_code, _M_bucket_count);
	    if (__bkt != _M_bucket)
	      this->_M_cur = nullptr;
	  }
      }

      size_t _M_bucket = 0;
      size_t _M_bucket_count = 0;

    public:
      size_t
      _M_get_bucket() const { return _M_bucket; }  // for debug mode
    };

  // Uninitialized storage for a _Hash object in a local iterator.
  // This type is DefaultConstructible even if the _Hash type isn't,
  // so that _Local_iterator_base<..., false> can be DefaultConstructible.
  template<typename _Hash>
    struct _Hash_obj_storage
    {
      union _Uninit_storage
      {
	_Uninit_storage() noexcept { }
	~_Uninit_storage() { }

	[[__no_unique_address__]] _Hash _M_h;
      };

      [[__no_unique_address__]] _Uninit_storage _M_u;
    };

  // Partial specialization used when hash codes are not cached
  template<typename _Key, typename _Value, typename _ExtractKey,
	   typename _Hash, typename _RangeHash, typename _Unused>
    struct _Local_iterator_base<_Key, _Value, _ExtractKey,
				_Hash, _RangeHash, _Unused, false>
    : _Hash_obj_storage<_Hash>, _Node_iterator_base<_Value, false>
    {
    protected:
      using __hash_code_base = _Hash_code_base<_Key, _Value, _ExtractKey,
					     _Hash, _RangeHash, _Unused, false>;
      using __hash_obj_storage = _Hash_obj_storage<_Hash>;
      using __node_iter_base = _Node_iterator_base<_Value, false>;

      _Local_iterator_base() = default;

      _Local_iterator_base(const __hash_code_base& __base,
			   _Hash_node<_Value, false>* __p,
			   size_t __bkt, size_t __bkt_count)
      : __node_iter_base(__p), _M_bucket(__bkt), _M_bucket_count(__bkt_count)
      { _M_init(__base._M_hash._M_obj); }

      ~_Local_iterator_base()
      {
	if (_M_bucket_count != size_t(-1))
	  _M_destroy();
      }

      _Local_iterator_base(const _Local_iterator_base& __iter)
      : __node_iter_base(__iter._M_cur), _M_bucket(__iter._M_bucket)
      , _M_bucket_count(__iter._M_bucket_count)
      {
	if (_M_bucket_count != size_t(-1))
	  _M_init(__iter._M_h());
      }

      _Local_iterator_base&
      operator=(const _Local_iterator_base& __iter)
      {
	if (_M_bucket_count != size_t(-1))
	  _M_destroy();
	this->_M_cur = __iter._M_cur;
	_M_bucket = __iter._M_bucket;
	_M_bucket_count = __iter._M_bucket_count;
	if (_M_bucket_count != size_t(-1))
	  _M_init(__iter._M_h());
	return *this;
      }

      void
      _M_incr()
      {
	__node_iter_base::_M_incr();
	if (this->_M_cur)
	  {
	    const auto __code = _M_h()(_ExtractKey{}(this->_M_cur->_M_v()));
	    size_t __bkt = _RangeHash{}(__code, _M_bucket_count);
	    if (__bkt != _M_bucket)
	      this->_M_cur = nullptr;
	  }
      }

      size_t _M_bucket = 0;
      size_t _M_bucket_count = -1;

      void
      _M_init(const _Hash& __h)
      { std::_Construct(std::__addressof(__hash_obj_storage::_M_u._M_h), __h); }

      void
      _M_destroy() { __hash_obj_storage::_M_u._M_h.~_Hash(); }

      const _Hash&
      _M_h() const { return __hash_obj_storage::_M_u._M_h; }

    public:
      size_t
      _M_get_bucket() const { return _M_bucket; }  // for debug mode
    };

  /// local iterators
  template<typename _Key, typename _Value, typename _ExtractKey,
	   typename _Hash, typename _RangeHash, typename _Unused,
	   bool __constant_iterators, bool __cache>
    struct _Local_iterator
    : public _Local_iterator_base<_Key, _Value, _ExtractKey,
				  _Hash, _RangeHash, _Unused, __cache>
    {
    private:
      using __base_type = _Local_iterator_base<_Key, _Value, _ExtractKey,
					   _Hash, _RangeHash, _Unused, __cache>;
      using __hash_code_base = typename __base_type::__hash_code_base;

    public:
      using value_type = _Value;
      using pointer = __conditional_t<__constant_iterators,
				      const value_type*, value_type*>;
      using reference = __conditional_t<__constant_iterators,
					const value_type&, value_type&>;
      using difference_type = ptrdiff_t;
      using iterator_category = forward_iterator_tag;

      _Local_iterator() = default;

      _Local_iterator(const __hash_code_base& __base,
		      _Hash_node<_Value, __cache>* __n,
		      size_t __bkt, size_t __bkt_count)
      : __base_type(__base, __n, __bkt, __bkt_count)
      { }

      reference
      operator*() const
      { return this->_M_cur->_M_v(); }

      pointer
      operator->() const
      { return this->_M_cur->_M_valptr(); }

      _Local_iterator&
      operator++()
      {
	this->_M_incr();
	return *this;
      }

      _Local_iterator
      operator++(int)
      {
	_Local_iterator __tmp(*this);
	this->_M_incr();
	return __tmp;
      }
    };

  /// local const_iterators
  template<typename _Key, typename _Value, typename _ExtractKey,
	   typename _Hash, typename _RangeHash, typename _Unused,
	   bool __constant_iterators, bool __cache>
    struct _Local_const_iterator
    : public _Local_iterator_base<_Key, _Value, _ExtractKey,
				  _Hash, _RangeHash, _Unused, __cache>
    {
    private:
      using __base_type = _Local_iterator_base<_Key, _Value, _ExtractKey,
					   _Hash, _RangeHash, _Unused, __cache>;
      using __hash_code_base = typename __base_type::__hash_code_base;

    public:
      using value_type        = _Value;
      using pointer           = const value_type*;
      using reference         = const value_type&;
      using difference_type   = ptrdiff_t;
      using iterator_category = forward_iterator_tag;

      _Local_const_iterator() = default;

      _Local_const_iterator(const __hash_code_base& __base,
			    _Hash_node<_Value, __cache>* __n,
			    size_t __bkt, size_t __bkt_count)
      : __base_type(__base, __n, __bkt, __bkt_count)
      { }

      _Local_const_iterator(const _Local_iterator<_Key, _Value, _ExtractKey,
						  _Hash, _RangeHash, _Unused,
						  __constant_iterators,
						  __cache>& __x)
      : __base_type(__x)
      { }

      reference
      operator*() const
      { return this->_M_cur->_M_v(); }

      pointer
      operator->() const
      { return this->_M_cur->_M_valptr(); }

      _Local_const_iterator&
      operator++()
      {
	this->_M_incr();
	return *this;
      }

      _Local_const_iterator
      operator++(int)
      {
	_Local_const_iterator __tmp(*this);
	this->_M_incr();
	return __tmp;
      }
    };

  /**
   *  Primary class template _Hashtable_base.
   *
   *  Helper class adding management of _Equal functor to
   *  _Hash_code_base type.
   *
   *  Base class templates are:
   *    - __detail::_Hash_code_base
   */
  template<typename _Key, typename _Value, typename _ExtractKey,
	   typename _Equal, typename _Hash, typename _RangeHash,
	   typename _Unused, typename _Traits>
    struct _Hashtable_base
    : public _Hash_code_base<_Key, _Value, _ExtractKey, _Hash, _RangeHash,
			     _Unused, _Traits::__hash_cached::value>
    {
    public:
      using key_type        = _Key;
      using value_type      = _Value;
      using key_equal       = _Equal;
      using size_type       = size_t;
      using difference_type = ptrdiff_t;

      using __traits_type = _Traits;
      using __hash_cached = typename __traits_type::__hash_cached;

      using __hash_code_base = _Hash_code_base<_Key, _Value, _ExtractKey,
					       _Hash, _RangeHash, _Unused,
					       __hash_cached::value>;

      using __hash_code = typename __hash_code_base::__hash_code;

    protected:
      [[__no_unique_address__]] _Hashtable_ebo_helper<_Equal> _M_equal{};

      _Hashtable_base() = default;

      _GLIBCXX_CEST_CONSTEXPR
      _Hashtable_base(const _Hash& __hash, const _Equal& __eq)
      : __hash_code_base(__hash), _M_equal{__eq}
      { }

      _GLIBCXX_CEST_CONSTEXPR
      bool
      _M_key_equals(const _Key& __k,
		    const _Hash_node_value<_Value,
					   __hash_cached::value>& __n) const
      {
	static_assert(__is_invocable<const _Equal&, const _Key&, const _Key&>{},
	  "key equality predicate must be invocable with two arguments of "
	  "key type");
	return _M_eq()(__k, _ExtractKey{}(__n._M_v()));
      }

      template<typename _Kt>
	_GLIBCXX_CEST_CONSTEXPR
	bool
	_M_key_equals_tr(const _Kt& __k,
			 const _Hash_node_value<_Value,
					     __hash_cached::value>& __n) const
	{
	  static_assert(
	    __is_invocable<const _Equal&, const _Kt&, const _Key&>{},
	    "key equality predicate must be invocable with the argument type "
	    "and the key type");
	  return _M_eq()(__k, _ExtractKey{}(__n._M_v()));
	}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wc++17-extensions" // if constexpr
      _GLIBCXX_CEST_CONSTEXPR
      bool
      _M_equals(const _Key& __k, __hash_code __c,
		const _Hash_node_value<_Value, __hash_cached::value>& __n) const
      {
	if constexpr (__hash_cached::value)
	  if (__c != __n._M_hash_code)
	    return false;

	return _M_key_equals(__k, __n);
      }

      template<typename _Kt>
	_GLIBCXX_CEST_CONSTEXPR
	bool
	_M_equals_tr(const _Kt& __k, __hash_code __c,
		     const _Hash_node_value<_Value,
					    __hash_cached::value>& __n) const
	{
	  if constexpr (__hash_cached::value)
	    if (__c != __n._M_hash_code)
	      return false;

	  return _M_key_equals_tr(__k, __n);
	}

      _GLIBCXX_CEST_CONSTEXPR
      bool
      _M_node_equals(
	const _Hash_node_value<_Value, __hash_cached::value>& __lhn,
	const _Hash_node_value<_Value, __hash_cached::value>& __rhn) const
      {
	if constexpr (__hash_cached::value)
	  if (__lhn._M_hash_code != __rhn._M_hash_code)
	    return false;

	return _M_key_equals(_ExtractKey{}(__lhn._M_v()), __rhn);
      }
#pragma GCC diagnostic pop

      _GLIBCXX_CEST_CONSTEXPR
      const _Equal&
      _M_eq() const noexcept { return _M_equal._M_obj; }
    };

  /**
   * This type deals with all allocation and keeps an allocator instance.
   */
  template<typename _NodeAlloc>
    struct _Hashtable_alloc
    {
    private:
      [[__no_unique_address__]] _Hashtable_ebo_helper<_NodeAlloc> _M_alloc{};

      template<typename>
	struct __get_value_type;
      template<typename _Val, bool _Cache_hash_code>
	struct __get_value_type<_Hash_node<_Val, _Cache_hash_code>>
	{ using type = _Val; };

    public:
      using __node_type = typename _NodeAlloc::value_type;
      using __node_alloc_type = _NodeAlloc;
      // Use __gnu_cxx to benefit from _S_always_equal and al.
      using __node_alloc_traits = __gnu_cxx::__alloc_traits<__node_alloc_type>;

      using __value_alloc_traits = typename __node_alloc_traits::template
	rebind_traits<typename __get_value_type<__node_type>::type>;

      using __node_ptr = __node_type*;
      using __node_base = _Hash_node_base;
      using __node_base_ptr = __node_base*;
      using __buckets_alloc_type =
	__alloc_rebind<__node_alloc_type, __node_base_ptr>;
      using __buckets_alloc_traits = std::allocator_traits<__buckets_alloc_type>;
      using __buckets_ptr = __node_base_ptr*;

      _Hashtable_alloc() = default;
      _Hashtable_alloc(const _Hashtable_alloc&) = default;
      _Hashtable_alloc(_Hashtable_alloc&&) = default;

      template<typename _Alloc>
	_GLIBCXX_CEST_CONSTEXPR
	_Hashtable_alloc(_Alloc&& __a)
	: _M_alloc{std::forward<_Alloc>(__a)}
	{ }

      _GLIBCXX_CEST_CONSTEXPR
      __node_alloc_type&
      _M_node_allocator()
      { return _M_alloc._M_obj; }

      _GLIBCXX_CEST_CONSTEXPR
      const __node_alloc_type&
      _M_node_allocator() const
      { return _M_alloc._M_obj; }

      // Allocate a node and construct an element within it.
      template<typename... _Args>
	_GLIBCXX_CEST_CONSTEXPR
	__node_ptr
	_M_allocate_node(_Args&&... __args);

      // Destroy the element within a node and deallocate the node.
      _GLIBCXX_CEST_CONSTEXPR
      void
      _M_deallocate_node(__node_ptr __n);

      // Deallocate a node.
      _GLIBCXX_CEST_CONSTEXPR
      void
      _M_deallocate_node_ptr(__node_ptr __n);

      // Deallocate the linked list of nodes pointed to by __n.
      // The elements within the nodes are destroyed.
      _GLIBCXX_CEST_CONSTEXPR
      void
      _M_deallocate_nodes(__node_ptr __n);

      _GLIBCXX_CEST_CONSTEXPR
      __buckets_ptr
      _M_allocate_buckets(size_t __bkt_count);

      _GLIBCXX_CEST_CONSTEXPR
      void
      _M_deallocate_buckets(__buckets_ptr, size_t __bkt_count);
    };

  // Definitions of class template _Hashtable_alloc's out-of-line member
  // functions.
  template<typename _NodeAlloc>
    template<typename... _Args>
      _GLIBCXX_CEST_CONSTEXPR
      auto
      _Hashtable_alloc<_NodeAlloc>::_M_allocate_node(_Args&&... __args)
      -> __node_ptr
      {
	auto& __alloc = _M_node_allocator();
	auto __nptr = __node_alloc_traits::allocate(__alloc, 1);
	__node_ptr __n = std::__to_address(__nptr);
	__try
	  {
	    ::new ((void*)__n) __node_type;
	    __node_alloc_traits::construct(__alloc, __n->_M_valptr(),
					   std::forward<_Args>(__args)...);
	    return __n;
	  }
	__catch(...)
	  {
	    __n->~__node_type();
	    __node_alloc_traits::deallocate(__alloc, __nptr, 1);
	    __throw_exception_again;
	  }
      }

  template<typename _NodeAlloc>
    _GLIBCXX_CEST_CONSTEXPR
    void
    _Hashtable_alloc<_NodeAlloc>::_M_deallocate_node(__node_ptr __n)
    {
      __node_alloc_traits::destroy(_M_node_allocator(), __n->_M_valptr());
      _M_deallocate_node_ptr(__n);
    }

  template<typename _NodeAlloc>
    _GLIBCXX_CEST_CONSTEXPR
    void
    _Hashtable_alloc<_NodeAlloc>::_M_deallocate_node_ptr(__node_ptr __n)
    {
      using _Ptr = typename __node_alloc_traits::pointer;
      auto __ptr = std::pointer_traits<_Ptr>::pointer_to(*__n);
      __n->~__node_type();
      __node_alloc_traits::deallocate(_M_node_allocator(), __ptr, 1);
    }

  template<typename _NodeAlloc>
    _GLIBCXX_CEST_CONSTEXPR
    void
    _Hashtable_alloc<_NodeAlloc>::_M_deallocate_nodes(__node_ptr __n)
    {
      while (__n)
	{
	  __node_ptr __tmp = __n;
	  __n = __n->_M_next();
	  _M_deallocate_node(__tmp);
	}
    }

  template<typename _NodeAlloc>
    _GLIBCXX_CEST_CONSTEXPR
    auto
    _Hashtable_alloc<_NodeAlloc>::_M_allocate_buckets(size_t __bkt_count)
    -> __buckets_ptr
    {
      __buckets_alloc_type __alloc(_M_node_allocator());

      auto __ptr = __buckets_alloc_traits::allocate(__alloc, __bkt_count);
      __buckets_ptr __p = std::__to_address(__ptr);
#if _GLIBCXX_CEST_VERSION
      if (__builtin_is_constant_evaluated())
      {
        for (int i = 0; i < __bkt_count; ++i)
        {
          std::construct_at(&__p[i]);
          __p[i] = 0;
        }
      }
      else
#endif
      __builtin_memset(__p, 0, __bkt_count * sizeof(__node_base_ptr));
      return __p;
    }

  template<typename _NodeAlloc>
    _GLIBCXX_CEST_CONSTEXPR
    void
    _Hashtable_alloc<_NodeAlloc>::
    _M_deallocate_buckets(__buckets_ptr __bkts, size_t __bkt_count)
    {
      using _Ptr = typename __buckets_alloc_traits::pointer;
      auto __ptr = std::pointer_traits<_Ptr>::pointer_to(*__bkts);
      __buckets_alloc_type __alloc(_M_node_allocator());
      __buckets_alloc_traits::deallocate(__alloc, __ptr, __bkt_count);
    }

 ///@} hashtable-detail
} // namespace __detail
/// @endcond
_GLIBCXX_END_NAMESPACE_VERSION
} // namespace std

namespace std _GLIBCXX_VISIBILITY(default)
{
_GLIBCXX_BEGIN_NAMESPACE_VERSION

// From src/c++11/hashtable_c++0x.c and src/shared/hashtable-aux.cc

namespace __detail
{
  // The sentinel value is kept only for abi backward compatibility.
#if _GLIBCXX_CEST_VERSION
  extern constexpr unsigned long __prime_list[] = // 256 + 1 or 256 + 48 + 1
#else
  extern const unsigned long __prime_list[] = // 256 + 1 or 256 + 48 + 1
#endif
  {
    2ul, 3ul, 5ul, 7ul, 11ul, 13ul, 17ul, 19ul, 23ul, 29ul, 31ul,
    37ul, 41ul, 43ul, 47ul, 53ul, 59ul, 61ul, 67ul, 71ul, 73ul, 79ul,
    83ul, 89ul, 97ul, 103ul, 109ul, 113ul, 127ul, 137ul, 139ul, 149ul,
    157ul, 167ul, 179ul, 193ul, 199ul, 211ul, 227ul, 241ul, 257ul,
    277ul, 293ul, 313ul, 337ul, 359ul, 383ul, 409ul, 439ul, 467ul,
    503ul, 541ul, 577ul, 619ul, 661ul, 709ul, 761ul, 823ul, 887ul,
    953ul, 1031ul, 1109ul, 1193ul, 1289ul, 1381ul, 1493ul, 1613ul,
    1741ul, 1879ul, 2029ul, 2179ul, 2357ul, 2549ul, 2753ul, 2971ul,
    3209ul, 3469ul, 3739ul, 4027ul, 4349ul, 4703ul, 5087ul, 5503ul,
    5953ul, 6427ul, 6949ul, 7517ul, 8123ul, 8783ul, 9497ul, 10273ul,
    11113ul, 12011ul, 12983ul, 14033ul, 15173ul, 16411ul, 17749ul,
    19183ul, 20753ul, 22447ul, 24281ul, 26267ul, 28411ul, 30727ul,
    33223ul, 35933ul, 38873ul, 42043ul, 45481ul, 49201ul, 53201ul,
    57557ul, 62233ul, 67307ul, 72817ul, 78779ul, 85229ul, 92203ul,
    99733ul, 107897ul, 116731ul, 126271ul, 136607ul, 147793ul,
    159871ul, 172933ul, 187091ul, 202409ul, 218971ul, 236897ul,
    256279ul, 277261ul, 299951ul, 324503ul, 351061ul, 379787ul,
    410857ul, 444487ul, 480881ul, 520241ul, 562841ul, 608903ul,
    658753ul, 712697ul, 771049ul, 834181ul, 902483ul, 976369ul,
    1056323ul, 1142821ul, 1236397ul, 1337629ul, 1447153ul, 1565659ul,
    1693859ul, 1832561ul, 1982627ul, 2144977ul, 2320627ul, 2510653ul,
    2716249ul, 2938679ul, 3179303ul, 3439651ul, 3721303ul, 4026031ul,
    4355707ul, 4712381ul, 5098259ul, 5515729ul, 5967347ul, 6456007ul,
    6984629ul, 7556579ul, 8175383ul, 8844859ul, 9569143ul, 10352717ul,
    11200489ul, 12117689ul, 13109983ul, 14183539ul, 15345007ul,
    16601593ul, 17961079ul, 19431899ul, 21023161ul, 22744717ul,
    24607243ul, 26622317ul, 28802401ul, 31160981ul, 33712729ul,
    36473443ul, 39460231ul, 42691603ul, 46187573ul, 49969847ul,
    54061849ul, 58488943ul, 63278561ul, 68460391ul, 74066549ul,
    80131819ul, 86693767ul, 93793069ul, 101473717ul, 109783337ul,
    118773397ul, 128499677ul, 139022417ul, 150406843ul, 162723577ul,
    176048909ul, 190465427ul, 206062531ul, 222936881ul, 241193053ul,
    260944219ul, 282312799ul, 305431229ul, 330442829ul, 357502601ul,
    386778277ul, 418451333ul, 452718089ul, 489790921ul, 529899637ul,
    573292817ul, 620239453ul, 671030513ul, 725980837ul, 785430967ul,
    849749479ul, 919334987ul, 994618837ul, 1076067617ul, 1164186217ul,
    1259520799ul, 1362662261ul, 1474249943ul, 1594975441ul, 1725587117ul,
    1866894511ul, 2019773507ul, 2185171673ul, 2364114217ul, 2557710269ul,
    2767159799ul, 2993761039ul, 3238918481ul, 3504151727ul, 3791104843ul,
    4101556399ul, 4294967291ul,
    // Sentinel, so we don't have to test the result of lower_bound,
    // or, on 64-bit machines, rest of the table.
#if __SIZEOF_LONG__ != 8
    4294967291ul
#else
    6442450933ul, 8589934583ul, 12884901857ul, 17179869143ul,
    25769803693ul, 34359738337ul, 51539607367ul, 68719476731ul,
    103079215087ul, 137438953447ul, 206158430123ul, 274877906899ul,
    412316860387ul, 549755813881ul, 824633720731ul, 1099511627689ul,
    1649267441579ul, 2199023255531ul, 3298534883309ul, 4398046511093ul,
    6597069766607ul, 8796093022151ul, 13194139533241ul, 17592186044399ul,
    26388279066581ul, 35184372088777ul, 52776558133177ul, 70368744177643ul,
    105553116266399ul, 140737488355213ul, 211106232532861ul, 281474976710597ul,
    562949953421231ul, 1125899906842597ul, 2251799813685119ul,
    4503599627370449ul, 9007199254740881ul, 18014398509481951ul,
    36028797018963913ul, 72057594037927931ul, 144115188075855859ul,
    288230376151711717ul, 576460752303423433ul,
    1152921504606846883ul, 2305843009213693951ul,
    4611686018427387847ul, 9223372036854775783ul,
    18446744073709551557ul, 18446744073709551557ul
#endif
  };
} // namespace __detail

namespace __detail
{
  // Return a prime no smaller than n.
  _GLIBCXX_CEST_CONSTEXPR
  std::size_t
  _Prime_rehash_policy::_M_next_bkt(std::size_t __n) const
  {
    // Optimize lookups involving the first elements of __prime_list.
    // (useful to speed-up, eg, constructors)
#if _GLIBCXX_CEST_VERSION
    static constexpr unsigned char __fast_bkt[]
#else
    static const unsigned char __fast_bkt[]
#endif
      = { 2, 2, 2, 3, 5, 5, 7, 7, 11, 11, 11, 11, 13, 13 };

#if _GLIBCXX_CEST_VERSION && defined(__clang__)
    auto __builtin_floor = [](std::floating_point auto f) -> decltype(f) {
        return static_cast<int>(f >= 0 ? f : f - 1);
    };
#endif

    if (__n < sizeof(__fast_bkt))
      {
	if (__n == 0)
	  // Special case on container 1st initialization with 0 bucket count
	  // hint. We keep _M_next_resize to 0 to make sure that next time we
	  // want to add an element allocation will take place.
	  return 1;

	_M_next_resize =
	  __builtin_floor(__fast_bkt[__n] * (double)_M_max_load_factor);
	return __fast_bkt[__n];
      }

    // Number of primes (without sentinel).
    constexpr auto __n_primes
      = sizeof(__prime_list) / sizeof(unsigned long) - 1;

    // Don't include the last prime in the search, so that anything
    // higher than the second-to-last prime returns a past-the-end
    // iterator that can be dereferenced to get the last prime.
    constexpr auto __last_prime = __prime_list + __n_primes - 1;

    const unsigned long* __next_bkt =
      std::lower_bound(__prime_list + 6, __last_prime, __n);

    if (__next_bkt == __last_prime)
      // Set next resize to the max value so that we never try to rehash again
      // as we already reach the biggest possible bucket number.
      // Note that it might result in max_load_factor not being respected.
      _M_next_resize = size_t(-1);
    else
      _M_next_resize =
	__builtin_floor(*__next_bkt * (double)_M_max_load_factor);

    return *__next_bkt;
  }

  // Finds the smallest prime p such that alpha p > __n_elt + __n_ins.
  // If p > __n_bkt, return make_pair(true, p); otherwise return
  // make_pair(false, 0).  In principle this isn't very different from
  // _M_bkt_for_elements.

  // The only tricky part is that we're caching the element count at
  // which we need to rehash, so we don't have to do a floating-point
  // multiply for every insertion.

  _GLIBCXX_CEST_CONSTEXPR
  std::pair<bool, std::size_t>
  _Prime_rehash_policy::
  _M_need_rehash(std::size_t __n_bkt, std::size_t __n_elt,
		 std::size_t __n_ins) const
  {
    if (__n_elt + __n_ins > _M_next_resize)
      {
#if _GLIBCXX_CEST_VERSION && defined(__clang__)
	auto __builtin_floor = [](std::floating_point auto f) -> decltype(f) {
	  return static_cast<int>(f >= 0 ? f : f - 1);
	};
#endif
	// If _M_next_resize is 0 it means that we have nothing allocated so
	// far and that we start inserting elements. In this case we start
	// with an initial bucket size of 11.
	double __min_bkts
	  = std::max<std::size_t>(__n_elt + __n_ins, _M_next_resize ? 0 : 11)
	  / (double)_M_max_load_factor;
	if (__min_bkts >= __n_bkt)
	  return { true,
	    _M_next_bkt(std::max<std::size_t>(__builtin_floor(__min_bkts) + 1,
					      __n_bkt * _S_growth_factor)) };

	_M_next_resize
	  = __builtin_floor(__n_bkt * (double)_M_max_load_factor);
	return { false, 0 };
      }
    else
      return { false, 0 };
  }
} // namespace __detail

_GLIBCXX_END_NAMESPACE_VERSION
} // namespace std

#endif // _HASHTABLE_POLICY_H
