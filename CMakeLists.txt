cmake_minimum_required(VERSION 3.20)
project(cpp_qa_lab LANGUAGES C CXX CUDA)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find fmt once for the whole project (vcpkg toolchain file should make this work).
find_package(fmt CONFIG REQUIRED)

# Find spdlog for logging support across the project
find_package(spdlog CONFIG REQUIRED)

# Generate compile_commands.json for tools like clangd. Can be disabled by setting
# -DEXPORT_COMPILE_COMMANDS=OFF when invoking cmake.
option(EXPORT_COMPILE_COMMANDS "Generate compile_commands.json for clangd and other tools" ON)
if(EXPORT_COMPILE_COMMANDS)
	set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

## Delegate building of examples and libs in csrc/ to its own CMakeLists
add_subdirectory(csrc)

# Tests
option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
	add_subdirectory(tests)
endif()
