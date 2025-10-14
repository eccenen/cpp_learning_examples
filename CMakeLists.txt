cmake_minimum_required(VERSION 3.10)
project(cpp_qa_lab LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Generate compile_commands.json for tools like clangd. Can be disabled by setting
# -DEXPORT_COMPILE_COMMANDS=OFF when invoking cmake.
option(EXPORT_COMPILE_COMMANDS "Generate compile_commands.json for clangd and other tools" ON)
if(EXPORT_COMPILE_COMMANDS)
	set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

## Automatically create one executable per source file in csrc/
file(GLOB SRC_FILES "${CMAKE_SOURCE_DIR}/csrc/*.cpp")
foreach(src ${SRC_FILES})
	get_filename_component(name ${src} NAME_WE)
	add_executable(${name} ${src})
	target_include_directories(${name} PRIVATE ${CMAKE_SOURCE_DIR}/csrc)
	target_compile_features(${name} PRIVATE cxx_std_17)
endforeach()

# Tests
option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
	enable_testing()

	# Use doctest headers from the user's vcpkg installation
	# Prefer using vcpkg toolchain or environment variable VCPKG_PATH instead of
	# hardcoding a personal path. Also support FetchContent fallback so the
	# project can build in environments without vcpkg.
	#
	# Users can either:
	#  - pass -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
	#  - export VCPKG_PATH=/path/to/vcpkg and optionally -DVCPKG_TARGET_TRIPLET=<triplet>
	# If neither is present, doctest will be provided via FetchContent (internet required).

	if(NOT DEFINED VCPKG_PATH)
		if(DEFINED ENV{VCPKG_PATH})
			set(VCPKG_PATH $ENV{VCPKG_PATH} CACHE PATH "Path to vcpkg root (from VCPKG_PATH env)")
		else()
			set(VCPKG_PATH "" CACHE PATH "Path to vcpkg root (optional). Prefer using CMAKE_TOOLCHAIN_FILE instead.")
		endif()
	endif()

	if(VCPKG_PATH)
		if(NOT DEFINED VCPKG_TARGET_TRIPLET)
			set(VCPKG_TARGET_TRIPLET "x64-linux" CACHE STRING "vcpkg triplet (if using manual VCPKG_PATH)")
		endif()
		set(DOCTEST_INCLUDE_DIR "${VCPKG_PATH}/installed/${VCPKG_TARGET_TRIPLET}/include")
	else()
		set(DOCTEST_INCLUDE_DIR "")
	endif()

	add_executable(test_heap_only tests/test_heap_only.cpp)
	target_compile_features(test_heap_only PRIVATE cxx_std_17)
	# always make project headers available to the test target
	target_include_directories(test_heap_only PRIVATE csrc)

	# Try to find a packaged doctest (works if vcpkg toolchain or system package is used)
	find_package(doctest CONFIG QUIET)
	if(doctest_FOUND)
		message(STATUS "Found doctest via find_package")
		target_link_libraries(test_heap_only PRIVATE doctest::doctest)
	else()
		# If we detected a vcpkg layout via VCPKG_PATH, use its include directory
		if(DOCTEST_INCLUDE_DIR AND EXISTS "${DOCTEST_INCLUDE_DIR}/doctest/doctest.h")
			message(STATUS "Using doctest headers from VCPKG_PATH: ${DOCTEST_INCLUDE_DIR}")
			target_include_directories(test_heap_only PRIVATE ${DOCTEST_INCLUDE_DIR} csrc)
		else()
			# Fallback: fetch doctest via FetchContent (header-only)
			message(STATUS "doctest not found via find_package or VCPKG_PATH; fetching doctest via FetchContent")
			include(FetchContent)
			FetchContent_Declare(
				doctest
				GIT_REPOSITORY https://github.com/doctest/doctest.git
				GIT_TAG v2.4.12
			)
			FetchContent_MakeAvailable(doctest)
			target_link_libraries(test_heap_only PRIVATE doctest::doctest)
			# ensure project headers are visible
			target_include_directories(test_heap_only PRIVATE csrc)
		endif()
	endif()

	add_test(NAME heap_only_tests COMMAND test_heap_only)
endif()
