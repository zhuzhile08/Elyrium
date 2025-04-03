cmake_minimum_required(VERSION 3.24.0)

project(ElyriumCLI VERSION 0.1.0)


add_executable(ElyriumCLI
	"src/main.cpp"
)


# Set version macro
target_compile_definitions(ElyriumCLI PUBLIC ELYRIUM_CLI_VERSION="${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")


# Set the executable name to lowercase to match the standard with other languages
set_target_properties(ElyriumCLI PROPERTIES OUTPUT_NAME "elyrium")


if (WIN32) 
	target_compile_options(ElyriumCLI PRIVATE /WX)
else () 
	target_compile_options(ElyriumCLI PRIVATE -Wall -Wextra -Wpedantic)
endif ()


target_include_directories(ElyriumCLI PRIVATE
	# local include directory
	${CMAKE_CURRENT_SOURCE_DIR}

	# utility libraries
	${LIBRARY_PATH}/lsd/
)


target_link_libraries(ElyriumCLI
PRIVATE
	Elyrium::Elyrium-static
	Elyrium::Headers
)
