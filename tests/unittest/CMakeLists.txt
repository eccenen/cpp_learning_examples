enable_testing()
include(${CMAKE_SOURCE_DIR}/cmake/vcpkg_helpers.cmake)

# Find or fetch doctest testing framework
find_or_fetch_package_config(doctest DOCTEST
    HEADER "doctest/doctest.h"
    GIT_REPO "https://github.com/doctest/doctest.git"
    GIT_TAG "v2.4.12"
)

# Collect all test sources
file(GLOB TEST_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp")

if(NOT TEST_SRCS)
    message(FATAL_ERROR "No test sources found in ${CMAKE_CURRENT_SOURCE_DIR}")
endif()

# Create test executable for each source file
foreach(src IN LISTS TEST_SRCS)
    get_filename_component(src_name ${src} NAME_WE)
    # Remove leading 'test' prefix if present: test_foo -> doctest_foo
    string(REGEX REPLACE "^test[_-]?" "" bin_name_base ${src_name})
    set(bin_name "doctest_${bin_name_base}")

    add_executable(${bin_name} ${src})
    
    # Include path for heap_only test specific headers
    target_include_directories(${bin_name} PRIVATE 
        ${CMAKE_SOURCE_DIR}/csrc/techniques/heap_only_create
    )
    
    # Link common dependencies (fmt, spdlog, csrc includes)
    target_link_libraries(${bin_name} PRIVATE 
        csrc::common
        $<$<BOOL:${DOCTEST_TARGET}>:${DOCTEST_TARGET}>
    )

    # Register test with CTest
    add_test(NAME ${bin_name} COMMAND ${bin_name})
endforeach()


