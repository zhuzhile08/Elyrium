/*************************
 * @file Common.h
 * @author Zhile Zhu (zhuzhile08@gmail.com)
 *
 * @brief forward declare all core members and utility
 *
 * @date 2022-02-05
 *
 * @copyright Copyright (c) 2022
 *************************/

#pragma once

#include <stddef.h>
#include <stdint.h>

namespace elyrium {

// abbreviations for fixed width integers

using uint8 = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;
using uint = unsigned int;

using uchar = unsigned char; // mostly alternative names, borrowed from ogre
using ushort = unsigned short;
using ulong = unsigned long;

using int8 = int8_t;
using int16 = int16_t;
using int32 = int32_t;
using int64 = int64_t;

// floating point types

using float32 = float;
using float64 = double;
using float128 = long double; // not supported everywhere, so don't really use this

// some other common types

using wchar = wchar_t;
using filepos = fpos_t;
using object_id = uint64;
using uintptr = std::uintptr_t;
using nullpointer = decltype(nullptr);
using size_type = std::size_t;

} // namespace elyrium

// utility macros

#define NODISCARD [[nodiscard]]
#define DEPRECATED [[deprecated]]
#define NO_UNIQUE_ADDRESS [[no_unique_address]]
