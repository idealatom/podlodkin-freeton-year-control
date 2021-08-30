///////////////////////////////////////////////////////////////
//  Copyright 2013 John Maddock. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt
//
// Comparison operators for cpp_int_backend:
//
#ifndef BOOST_MP_DETAIL_BITSCAN_HPP
#define BOOST_MP_DETAIL_BITSCAN_HPP

#include <boost/predef/other/endian.h>
#include <boost/cstdint.hpp>

#if (defined(BOOST_MSVC) || (defined(__clang__) && defined(__c2__)) || (defined(BOOST_INTEL) && defined(_MSC_VER))) && \
    (defined(_M_IX86) || defined(_M_X64))
#include <intrin.h>
#endif

namespace nil {
    namespace crypto3 {
        namespace multiprecision {
            namespace detail {

                template<class Unsigned>
                inline BOOST_MP_CXX14_CONSTEXPR unsigned find_lsb_default(Unsigned mask) {
                    unsigned result = 0;
                    while (!(mask & 1u)) {
                        mask >>= 1;
                        ++result;
                    }
                    return result;
                }

                template<class Unsigned>
                inline BOOST_MP_CXX14_CONSTEXPR unsigned find_msb_default(Unsigned mask) {
                    unsigned index = 0;
                    while (mask) {
                        ++index;
                        mask >>= 1;
                    }
                    return --index;
                }

                template<class Unsigned>
                inline BOOST_MP_CXX14_CONSTEXPR unsigned find_lsb(Unsigned mask, const boost::mpl::int_<0>&) {
                    return find_lsb_default(mask);
                }

                template<class Unsigned>
                inline BOOST_MP_CXX14_CONSTEXPR unsigned find_msb(Unsigned mask, const boost::mpl::int_<0>&) {
                    return find_msb_default(mask);
                }

#if (defined(BOOST_MSVC) || (defined(__clang__) && defined(__c2__)) || (defined(BOOST_INTEL) && defined(_MSC_VER))) && \
    (defined(_M_IX86) || defined(_M_X64))

#pragma intrinsic(_BitScanForward, _BitScanReverse)

                BOOST_FORCEINLINE unsigned find_lsb(unsigned long mask, const boost::mpl::int_<1>&) {
                    unsigned long result;
                    _BitScanForward(&result, mask);
                    return result;
                }

                BOOST_FORCEINLINE unsigned find_msb(unsigned long mask, const boost::mpl::int_<1>&) {
                    unsigned long result;
                    _BitScanReverse(&result, mask);
                    return result;
                }
#ifdef _M_X64

#pragma intrinsic(_BitScanForward64, _BitScanReverse64)

                BOOST_FORCEINLINE unsigned find_lsb(unsigned __int64 mask, const boost::mpl::int_<2>&) {
                    unsigned long result;
                    _BitScanForward64(&result, mask);
                    return result;
                }
                template<class Unsigned>
                BOOST_FORCEINLINE unsigned find_msb(Unsigned mask, const boost::mpl::int_<2>&) {
                    unsigned long result;
                    _BitScanReverse64(&result, mask);
                    return result;
                }
#endif

                template<class Unsigned>
                BOOST_FORCEINLINE BOOST_MP_CXX14_CONSTEXPR unsigned find_lsb(Unsigned mask) {
                    typedef typename boost::make_unsigned<Unsigned>::type ui_type;
                    typedef typename boost::mpl::if_c<
                        sizeof(Unsigned) <= sizeof(unsigned long), boost::mpl::int_<1>,
#ifdef _M_X64
                        typename boost::mpl::if_c<sizeof(Unsigned) <= sizeof(__int64), boost::mpl::int_<2>, boost::mpl::int_<0>>::type
#else
                        boost::mpl::int_<0>
#endif
                        >::type tag_type;
#ifndef BOOST_MP_NO_CONSTEXPR_DETECTION
                    if (BOOST_MP_IS_CONST_EVALUATED(mask)) {
                        return find_lsb_default(mask);
                    } else
#endif
                        return find_lsb(static_cast<ui_type>(mask), tag_type());
                }

                template<class Unsigned>
                BOOST_FORCEINLINE BOOST_MP_CXX14_CONSTEXPR unsigned find_msb(Unsigned mask) {
                    typedef typename boost::make_unsigned<Unsigned>::type ui_type;
                    typedef typename boost::mpl::if_c<
                        sizeof(Unsigned) <= sizeof(unsigned long), boost::mpl::int_<1>,
#ifdef _M_X64
                        typename boost::mpl::if_c<sizeof(Unsigned) <= sizeof(__int64), boost::mpl::int_<2>, boost::mpl::int_<0>>::type
#else
                        boost::mpl::int_<0>
#endif
                        >::type tag_type;
#ifndef BOOST_MP_NO_CONSTEXPR_DETECTION
                    if (BOOST_MP_IS_CONST_EVALUATED(mask)) {
                        return find_msb_default(mask);
                    } else
#endif
                        return find_msb(static_cast<ui_type>(mask), tag_type());
                }

#elif defined(BOOST_GCC) || defined(__clang__) || (defined(BOOST_INTEL) && defined(__GNUC__))

                BOOST_FORCEINLINE unsigned find_lsb(unsigned mask, boost::mpl::int_<1> const&) {
                    return __builtin_ctz(mask);
                }
                BOOST_FORCEINLINE unsigned find_lsb(unsigned long mask, boost::mpl::int_<2> const&) {
                    return __builtin_ctzl(mask);
                }
                BOOST_FORCEINLINE unsigned find_lsb(boost::ulong_long_type mask, boost::mpl::int_<3> const&) {
                    return __builtin_ctzll(mask);
                }
                BOOST_FORCEINLINE unsigned find_msb(unsigned mask, boost::mpl::int_<1> const&) {
                    return sizeof(unsigned) * CHAR_BIT - 1 - __builtin_clz(mask);
                }
                BOOST_FORCEINLINE unsigned find_msb(unsigned long mask, boost::mpl::int_<2> const&) {
                    return sizeof(unsigned long) * CHAR_BIT - 1 - __builtin_clzl(mask);
                }
                BOOST_FORCEINLINE unsigned find_msb(boost::ulong_long_type mask, boost::mpl::int_<3> const&) {
                    return sizeof(boost::ulong_long_type) * CHAR_BIT - 1 - __builtin_clzll(mask);
                }
#ifdef BOOST_HAS_INT128

                __extension__ typedef unsigned __int128 uint128_type;

                BOOST_FORCEINLINE unsigned find_msb(uint128_type mask, boost::mpl::int_<0> const&) {
                    union {
                        uint128_type v;
                        boost::uint64_t sv[2];
                    } val;
                    val.v = mask;
#if BOOST_ENDIAN_LITTLE_BYTE
                    if (val.sv[1])
                        return find_msb(val.sv[1], boost::mpl::int_<3>()) + 64;
                    return find_msb(val.sv[0], boost::mpl::int_<3>());
#else
                    if (val.sv[0])
                        return find_msb(val.sv[0], boost::mpl::int_<3>()) + 64;
                    return find_msb(val.sv[1], boost::mpl::int_<3>());
#endif
                }
                BOOST_FORCEINLINE unsigned find_lsb(uint128_type mask, boost::mpl::int_<0> const&) {
                    union {
                        uint128_type v;
                        boost::uint64_t sv[2];
                    } val;
                    val.v = mask;
#if BOOST_ENDIAN_LITTLE_BYTE
                    if (val.sv[0] == 0)
                        return find_lsb(val.sv[1], boost::mpl::int_<3>()) + 64;
                    return find_lsb(val.sv[0], boost::mpl::int_<3>());
#else
                    if (val.sv[1] == 0)
                        return find_lsb(val.sv[0], boost::mpl::int_<3>()) + 64;
                    return find_lsb(val.sv[1], boost::mpl::int_<3>());
#endif
                }
#endif

                template<class Unsigned>
                BOOST_FORCEINLINE BOOST_MP_CXX14_CONSTEXPR unsigned find_lsb(Unsigned mask) {
                    typedef typename boost::make_unsigned<Unsigned>::type ui_type;
                    typedef typename boost::mpl::if_c<
                        sizeof(Unsigned) <= sizeof(unsigned), boost::mpl::int_<1>,
                        typename boost::mpl::if_c<sizeof(Unsigned) <= sizeof(unsigned long), boost::mpl::int_<2>,
                                           typename boost::mpl::if_c<sizeof(Unsigned) <= sizeof(boost::ulong_long_type),
                                                              boost::mpl::int_<3>, boost::mpl::int_<0>>::type>::type>::type tag_type;
#ifndef BOOST_MP_NO_CONSTEXPR_DETECTION
                    if (BOOST_MP_IS_CONST_EVALUATED(mask)) {
                        return find_lsb_default(mask);
                    } else
#endif
                        return find_lsb(static_cast<ui_type>(mask), tag_type());
                }
                template<class Unsigned>
                BOOST_FORCEINLINE BOOST_MP_CXX14_CONSTEXPR unsigned find_msb(Unsigned mask) {
                    typedef typename boost::make_unsigned<Unsigned>::type ui_type;
                    typedef typename boost::mpl::if_c<
                        sizeof(Unsigned) <= sizeof(unsigned), boost::mpl::int_<1>,
                        typename boost::mpl::if_c<sizeof(Unsigned) <= sizeof(unsigned long), boost::mpl::int_<2>,
                                           typename boost::mpl::if_c<sizeof(Unsigned) <= sizeof(boost::ulong_long_type),
                                                              boost::mpl::int_<3>, boost::mpl::int_<0>>::type>::type>::type tag_type;
#ifndef BOOST_MP_NO_CONSTEXPR_DETECTION
                    if (BOOST_MP_IS_CONST_EVALUATED(mask)) {
                        return find_msb_default(mask);
                    } else
#endif
                        return find_msb(static_cast<ui_type>(mask), tag_type());
                }
#elif defined(BOOST_INTEL)
                BOOST_FORCEINLINE unsigned find_lsb(unsigned mask, boost::mpl::int_<1> const&) {
                    return _bit_scan_forward(mask);
                }
                BOOST_FORCEINLINE unsigned find_msb(unsigned mask, boost::mpl::int_<1> const&) {
                    return _bit_scan_reverse(mask);
                }
                template<class Unsigned>
                BOOST_FORCEINLINE BOOST_MP_CXX14_CONSTEXPR unsigned find_lsb(Unsigned mask) {
                    typedef typename boost::make_unsigned<Unsigned>::type ui_type;
                    typedef typename boost::mpl::if_c<sizeof(Unsigned) <= sizeof(unsigned), boost::mpl::int_<1>, boost::mpl::int_<0>>::type
                        tag_type;
#ifndef BOOST_MP_NO_CONSTEXPR_DETECTION
                    if (BOOST_MP_IS_CONST_EVALUATED(mask)) {
                        return find_lsb_default(mask);
                    } else
#endif
                        return find_lsb(static_cast<ui_type>(mask), tag_type());
                }
                template<class Unsigned>
                BOOST_FORCEINLINE BOOST_MP_CXX14_CONSTEXPR unsigned find_msb(Unsigned mask) {
                    typedef typename boost::make_unsigned<Unsigned>::type ui_type;
                    typedef typename boost::mpl::if_c<sizeof(Unsigned) <= sizeof(unsigned), boost::mpl::int_<1>, boost::mpl::int_<0>>::type
                        tag_type;
#ifndef BOOST_MP_NO_CONSTEXPR_DETECTION
                    if (BOOST_MP_IS_CONST_EVALUATED(mask)) {
                        return find_msb_default(mask);
                    } else
#endif
                        return find_msb(static_cast<ui_type>(mask), tag_type());
                }
#else
                template<class Unsigned>
                BOOST_FORCEINLINE BOOST_MP_CXX14_CONSTEXPR unsigned find_lsb(Unsigned mask) {
                    return find_lsb(mask, boost::mpl::int_<0>());
                }
                template<class Unsigned>
                BOOST_FORCEINLINE BOOST_MP_CXX14_CONSTEXPR unsigned find_msb(Unsigned mask) {
                    return find_msb(mask, boost::mpl::int_<0>());
                }
#endif

            }    // namespace detail
        }        // namespace multiprecision
    }            // namespace crypto3
}    // namespace nil

#endif
