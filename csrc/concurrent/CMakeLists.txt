project(concurrent_gemm_examples)

set(SOURCES
    gemm_learning.cpp
    gemm_demo.cpp
)

add_executable(gemm_demo ${SOURCES})

target_include_directories(gemm_demo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(gemm_demo PRIVATE csrc::common)

find_package(Threads REQUIRED)
target_link_libraries(gemm_demo PRIVATE Threads::Threads)

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(gemm_demo PRIVATE OpenMP::OpenMP_CXX)
    target_compile_definitions(gemm_demo PRIVATE _OPENMP)
endif()

set_target_properties(gemm_demo PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)

# Ensure runtime output for this directory goes to <build-tree>/bin/concurrent
set(concurrent_output_dir ${CMAKE_BINARY_DIR}/bin/concurrent)
set_property(DIRECTORY PROPERTY RUNTIME_OUTPUT_DIRECTORY ${concurrent_output_dir})
set_property(DIRECTORY PROPERTY RUNTIME_OUTPUT_DIRECTORY_DEBUG ${concurrent_output_dir})
set_property(DIRECTORY PROPERTY RUNTIME_OUTPUT_DIRECTORY_RELEASE ${concurrent_output_dir})
set_property(DIRECTORY PROPERTY RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${concurrent_output_dir})
set_property(DIRECTORY PROPERTY RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${concurrent_output_dir})

set_target_properties(gemm_demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${concurrent_output_dir}
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${concurrent_output_dir}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${concurrent_output_dir}
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${concurrent_output_dir}
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${concurrent_output_dir}
)
