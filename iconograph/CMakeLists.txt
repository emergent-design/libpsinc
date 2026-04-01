cmake_minimum_required(VERSION 3.22)

project(
	iconograph
	VERSION 0.2.4
	DESCRIPTION "a Qt application for streaming from libpsinc cameras"
	LANGUAGES CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS on)
set(CMAKE_BUILD_TYPE RelWithDebInfo)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_CXX_EXTENSIONS off)
set(CMAKE_AUTOUIC_SEARCH_PATHS ui)

add_compile_options(-Wall -Wextra -Wpedantic -fno-omit-frame-pointer -fPIC -frtti)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

file(GLOB_RECURSE iconograph_sources
	include/*.h
	src/*.cpp
	ui/*.ui
)

qt_add_resources(iconograph_sources resources/iconograph.qrc)
qt_add_executable(iconograph ${iconograph_sources})

target_include_directories(iconograph PUBLIC include ../include)
target_link_directories(iconograph PUBLIC ../build)
target_link_libraries(
	iconograph PRIVATE
	Qt6::Widgets
	psinc freeimage usb-1.0
)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
	set_target_properties(iconograph PROPERTIES WIN32_EXECUTABLE true)
endif()
