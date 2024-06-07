cmake_minimum_required(VERSION 3.24.0)

project(Elyrium VERSION 1.0.0)


# build the content manager
add_executable(Elyrium
	"src/main.cpp"
)


# set the executable name to lowercase to match the standard with other languages
set_target_properties(Elyrium PROPERTIES OUTPUT_NAME "Elyrium")


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
	${Elyrium_LIB_INCLUDE_DIR}

	# utility libraries
	${LIBRARY_PATH}/lsd/
	${LIBRARY_PATH}/fmt/include/
)


# link libraries
target_link_libraries(Elyrium
PRIVATE
	#ElyriumLib
)
