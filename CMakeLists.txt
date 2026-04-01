cmake_minimum_required(VERSION 3.22)

project(
	libniven
	VERSION 0.2.4
	DESCRIPTION "a cross-platform acquisition library for the Perception-SI Ltd range of cameras"
	LANGUAGES CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS on)
set(CMAKE_BUILD_TYPE RelWithDebInfo)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_CXX_EXTENSIONS off)
# set(CMAKE_CXX_COMPILER clang++)
# set(CMAKE_CXX_COMPILER_LINKER clang++)


include_directories(include)
add_compile_options(-Wall -Wextra -Wpedantic -fno-omit-frame-pointer -fPIC -D_FORTIFY_SOURCE=2)

file(GLOB_RECURSE psinc_sources
	src/psinc/*.cpp
	src/psinc-c/*.cpp
)

add_library(psinc SHARED ${psinc_sources})
target_link_libraries(psinc PRIVATE usb-1.0 freeimage)
set_target_properties(psinc PROPERTIES SOVERSION 0)
