//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _CLANGOZ_LIBCPP___UTILITY_CONVERT_TO_INTEGRAL_H
#define _CLANGOZ_LIBCPP___UTILITY_CONVERT_TO_INTEGRAL_H

#include <type_traits>

namespace {

constexpr
int __convert_to_integral(int __val) { return __val; }

constexpr
unsigned __convert_to_integral(unsigned __val) { return __val; }

constexpr
long __convert_to_integral(long __val) { return __val; }

constexpr
unsigned long __convert_to_integral(unsigned long __val) { return __val; }

constexpr
long long __convert_to_integral(long long __val) { return __val; }

constexpr
unsigned long long __convert_to_integral(unsigned long long __val) {return __val; }

template<typename _Fp, std::enable_if_t<std::is_floating_point<_Fp>::value, int> = 0>
constexpr
long long
 __convert_to_integral(_Fp __val) { return __val; }

constexpr
__int128_t __convert_to_integral(__int128_t __val) { return __val; }

constexpr
__uint128_t __convert_to_integral(__uint128_t __val) { return __val; }

template <class _Tp, bool = std::is_enum<_Tp>::value>
struct __sfinae_underlying_type
{
    typedef typename std::underlying_type<_Tp>::type type;
    typedef decltype(((type)1) + 0) __promoted_type;
};

template <class _Tp>
struct __sfinae_underlying_type<_Tp, false> {};

template <class _Tp>
constexpr
typename __sfinae_underlying_type<_Tp>::__promoted_type
__convert_to_integral(_Tp __val) { return __val; }

} // namespace {

#endif // _CLANGOZ_LIBCPP___UTILITY_CONVERT_TO_INTEGRAL_H
