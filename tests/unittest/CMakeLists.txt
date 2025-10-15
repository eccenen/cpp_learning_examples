enable_testing()
include(${CMAKE_SOURCE_DIR}/cmake/vcpkg_helpers.cmake)

# Try to find or fetch; this will set DOCTEST_FOUND / DOCTEST_TARGET / DOCTEST_INCLUDE
find_or_fetch_package_config(doctest DOCTEST
    HEADER "doctest/doctest.h"
    GIT_REPO "https://github.com/doctest/doctest.git"
    GIT_TAG "v2.4.12"
)

file(GLOB TEST_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp")

if(NOT TEST_SRCS)
    message(FATAL_ERROR "No test sources found in ${CMAKE_CURRENT_SOURCE_DIR}")
endif()

# For each source file, create an executable named doctest_<srcname_without_test_prefix>
foreach(src IN LISTS TEST_SRCS)
    get_filename_component(src_name ${src} NAME_WE)
    # Remove leading 'test' or 'test_' or 'test-' prefix if present
    string(REGEX REPLACE "^test[_-]?" "" bin_name_base ${src_name})
    set(bin_name "doctest_${bin_name_base}")

    add_executable(${bin_name} ${src})

    if(DEFINED DOCTEST_INCLUDE AND DOCTEST_INCLUDE)
        target_include_directories(${bin_name} PRIVATE ${DOCTEST_INCLUDE})
    endif()

    target_include_directories(${bin_name} PRIVATE ${CMAKE_SOURCE_DIR}/csrc ${CMAKE_SOURCE_DIR}/csrc/techniques/heap_only_create)

    if(DEFINED DOCTEST_TARGET AND DOCTEST_TARGET)
        target_link_libraries(${bin_name} PRIVATE ${DOCTEST_TARGET})
    endif()

    # Ensure C++ standard and any project compile options are used
    target_compile_features(${bin_name} PRIVATE cxx_std_17)

    # Register with CTest
    add_test(NAME ${bin_name} COMMAND ${bin_name})
endforeach()


