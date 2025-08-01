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

struct Object;

#ifdef ELYRIUM_64_BIT

using signed_type = int64;
using floating_type = float64;

using value_marker_type = uintptr;
using value_state_type = uintptr;

#else

using signed_int = int32;
using floating = float32;

struct alignas(8) ValueState {
public:
	size_type marker;
	uintptr value;
};

using value_marker_type = size_type;
using value_state_type = ValueState;

#endif

using object_type = Object*;
using pointer_type = void*;

using value_state_internal = uintptr;

} // namespace elyrium
