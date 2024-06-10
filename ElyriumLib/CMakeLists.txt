cmake_minimum_required(VERSION 3.24.0)

project(ElyriumLib VERSION 0.0.1)


# set include path
set (ELYRIUM_LIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

# set the source files of the project
set (ELYRIUM_LIB_SOURCE_FILES 
	"src/Core/Error.cpp"
	"src/Interpreter/Token.cpp"
	"src/Interpreter/Lexer.cpp"
)


# building the library
if (BUILD_STATIC)
	add_library(ElyriumLib STATIC ${ELYRIUM_LIB_SOURCE_FILES})
else ()
	add_library(ElyriumLib SHARED ${ELYRIUM_LIB_SOURCE_FILES})
endif ()


# use LSD num utilities extension
target_compile_definitions(ElyriumLib PUBLIC LSD_ENUM_UTILITIES)


# define library version
target_compile_definitions(ElyriumLib PUBLIC ELYRIUM_LIB_VERSION="${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")


# build error level
if (WIN32) 
	target_compile_options(ElyriumLib PRIVATE /WX)
else () 
	target_compile_options(ElyriumLib PRIVATE -Wall -Wextra -Wpedantic -Wformat)
endif ()


# includes
target_include_directories(ElyriumLib PUBLIC
	# local include directory
	${ELYRIUM_LIB_INCLUDE_DIR}

	# utility libraries
	${LIBRARY_PATH}/lsd/
	${LIBRARY_PATH}/fmt/include/
)


# link libraries
target_link_libraries(ElyriumLib
LINK_PUBLIC
	fmt::fmt
)


# precompile headers
target_precompile_headers(ElyriumLib
PUBLIC
	<typeindex>
	<type_traits>
	<algorithm>
	<array>
	<chrono>
	<cmath>
	<cstdio>
	<cstdint>
	<cstdlib>
	<deque>
	<filesystem>
	<fstream>
	<functional>
	<iostream>
	<initializer_list>
	<map>
	<set>
	<stddef.h>
	<stdexcept>
	<stdint.h>
	<string>
	<string_view>
	<type_traits>
	<utility>
	<wchar.h>
)
