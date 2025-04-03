/*************************
 * @file Config.hpp
 * @author zhuzhile08 (zhuzhile08@gmail.com)
 * 
 * @brief Library configs and settings
 * 
 * @date 2024-06-07
 * @copyright Copyright (c) 2024
 *************************/

#pragma once

// Config macros

#if UINTPTR_MAX == UINT64_MAX && !ELYRIUM_FORCE_32_BIT_ON_64_BIT
#define ELYRIUM_64_BIT
#else
#define ELYRIUM_32_BIT
#endif

#if defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))
#define ELYRIUM_POSIX
#elif defined(_WIN32) || defined(_WIN64)
#define ELYRIUM_WINDOWS
#else
#define ELYRIUM_ALT_OS
#endif

namespace elyrium {

namespace config {

inline constexpr const char* version = ELYRIUM_LIB_VERSION;

} // namespace config

} // namespace elyrium

