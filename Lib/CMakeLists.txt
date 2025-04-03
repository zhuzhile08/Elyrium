cmake_minimum_required(VERSION 3.24.0)

project(ElyriumLib VERSION 0.1.0)


set (ELYRIUM_LIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/)

set (ELYRIUM_LIB_SOURCE_FILES 
	"src/Core/Error.cpp"
	"src/Core/File.cpp"

	"src/Compiler/Token.cpp"
	"src/Compiler/Lexer.cpp"
	# "src/Compiler/Parser.cpp"
)

if (BUILD_STATIC)
	add_library(ElyriumLib STATIC ${ELYRIUM_LIB_SOURCE_FILES})
	add_library(Elyrium::Elyrium-static ALIAS ElyriumLib)
else ()
	add_library(ElyriumLib SHARED ${ELYRIUM_LIB_SOURCE_FILES})
	add_library(Elyrium::Elyrium-shared ALIAS ElyriumLib)
endif ()

add_library(Elyrium INTERFACE)
add_library(Elyrium_Headers INTERFACE)
add_library(Elyrium::Headers ALIAS Elyrium_Headers)

target_include_directories(ElyriumLib INTERFACE ${ELYRIUM_LIB_INCLUDE_DIR})


# Use LSD num utilities extension
target_compile_definitions(ElyriumLib PUBLIC LSD_ENUM_UTILITIES)

# Define library version
target_compile_definitions(ElyriumLib PUBLIC ELYRIUM_LIB_VERSION="${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")


if (WIN32) 
	target_compile_options(ElyriumLib PRIVATE /WX)
else () 
	target_compile_options(ElyriumLib PRIVATE -Wall -Wextra -Wpedantic -Wformat)
endif ()


target_include_directories(ElyriumLib PUBLIC
	# local include directory
	${ELYRIUM_LIB_INCLUDE_DIR}
)


target_link_libraries(ElyriumLib
LINK_PUBLIC
	LyraStandardLibrary::Headers
)
