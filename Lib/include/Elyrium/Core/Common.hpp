/*************************
 * @file Common.hpp
 * @author Zhile Zhu (zhuzhile08@gmail.com)
 *
 * @brief Type aliases and forward declarations
 *
 * @date 2024-06-07
 * @copyright Copyright (c) 2024
 *************************/

#pragma once

#include <Elyrium/Core/Config.hpp>

#include <cstddef>
#include <cstdint>
#include <cstdio>

namespace elyrium {

// Type aliases

// Integral types

using uint8 = std::uint8_t;
using uint16 = std::uint16_t;
using uint32 = std::uint32_t;
using uint64 = std::uint64_t;

using int8 = std::int8_t;
using int16 = std::int16_t;
using int32 = std::int32_t;
using int64 = std::int64_t;

// Floating point types

using float32 = float;
using float64 = double;

// Character types

using char8 = char;
using char16 = char16_t;
using char32 = char32_t;

// Other common types

using filepos = std::fpos_t;
using uintptr = std::uintptr_t;
using nullpointer = std::nullptr_t;
using size_type = std::size_t;


// VM types

#ifdef ELYRIUM_64_BIT

using signed_int = int64;
using unsigned_int = uint64;
using floating = float64;

#else

using signed_int = int32;
using unsigned_int = uint32;
using floating = float32;

#endif

} // namespace elyrium
