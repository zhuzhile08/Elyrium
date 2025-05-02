cmake_minimum_required (VERSION 3.24.0)
set(CMAKE_OSX_DEPLOYMENT_TARGET "14.3" CACHE STRING "Minimum OS X deployment version")

project (ElyriumToolchain)


# project options
option(BUILD_STATIC "Build the Elyrium toolchain and its dependencies statically" ON)
if (BUILD_STATIC)
	option(BUILD_SHARED_LIBS OFF)
else ()
	option(BUILD_SHARED_LIBS ON)
endif()


# Dependencies path
set(LIBRARY_PATH ${CMAKE_SOURCE_DIR}/Deps)


# set global options
set(CMAKE_CXX_STANDARD 23)

# Include sub-projects.
add_subdirectory ("Deps")
add_subdirectory ("Lib")
add_subdirectory ("CLI")
add_subdirectory ("Tests")
