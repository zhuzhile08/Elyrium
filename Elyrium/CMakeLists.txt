cmake_minimum_required(VERSION 3.24.0)

project(Elyrium VERSION 0.0.1)


# build the content manager
add_executable(Elyrium
	"src/main.cpp"
)


# set version macro
target_compile_definitions(ElyriumLib PUBLIC ELYRIUM_VERSION="${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")


# set the executable name to lowercase to match the standard with other languages
set_target_properties(Elyrium PROPERTIES OUTPUT_NAME "elyrium")


# build error level
if (WIN32) 
	target_compile_options(Elyrium PRIVATE /WX)
else () 
	target_compile_options(Elyrium PRIVATE -Wall -Wextra -Wpedantic)
endif ()


# includes
target_include_directories(Elyrium PRIVATE
	# local include directory
	${CMAKE_CURRENT_SOURCE_DIR}/include/
	${ELYRIUM_LIB_INCLUDE_DIR}

	# utility libraries
	${LIBRARY_PATH}/lsd/
	${LIBRARY_PATH}/fmt/include/
)


# link libraries
target_link_libraries(Elyrium
PRIVATE
	ElyriumLib
)
