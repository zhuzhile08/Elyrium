cmake_minimum_required(VERSION 3.24.0)

project(ElyriumLib VERSION 0.5.0)

target_include_directories(ElyriumLib PUBLIC
	# local include directory
	${CMAKE_CURRENT_SOURCE_DIR}/include/

	# utility libraries
	${LIBRARY_PATH}/lsd/
	${LIBRARY_PATH}/fmt/include/
)

set (Elyrium_LIB_SOURCE_FILES 
	"src/Common/Logger.cpp"
	"src/Common/Benchmark.cpp"
	"src/Common/FileSystem.cpp"
)

# building the library
if (BUILD_STATIC)
	add_library(ElyriumLib STATIC "${Elyrium_LIB_SOURCE_FILES}")
else ()
	add_library(ElyriumLib SHARED "${Elyrium_LIB_SOURCE_FILES}")
endif ()

# build error level
if (WIN32) 
	target_compile_options(ElyriumLib PRIVATE /WX)
else () 
	target_compile_options(ElyriumLib PRIVATE -Wall -Wextra -Wpedantic)
endif ()


# use LSD num utilities extension

target_compile_definitions(ElyriumLib PUBLIC LSD_ENUM_UTILITIES)


add_dependencies(ElyriumLib build_shaders)

# link some libraries
target_link_libraries(ElyriumLib
LINK_PUBLIC
	fmt::fmt
)

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
