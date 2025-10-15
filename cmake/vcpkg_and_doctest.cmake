cmake_minimum_required(VERSION 3.10)

# Helper to configure doctest for a given target.
# Usage: setup_doctest_for_target(<target>)
function(setup_doctest_for_target target)
    # Detect VCPKG_PATH from cache or environment if not provided
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

    # Try to find a packaged doctest (works when vcpkg toolchain is active)
    find_package(doctest CONFIG QUIET)
    if(doctest_FOUND)
        message(STATUS "Found doctest via find_package")
        target_link_libraries(${target} PRIVATE doctest::doctest)
        return()
    endif()

    # If vcpkg include exists, use it
    if(DOCTEST_INCLUDE_DIR AND EXISTS "${DOCTEST_INCLUDE_DIR}/doctest/doctest.h")
        message(STATUS "Using doctest headers from VCPKG_PATH: ${DOCTEST_INCLUDE_DIR}")
        target_include_directories(${target} PRIVATE ${DOCTEST_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/csrc)
        return()
    endif()

    # Fallback: fetch doctest via FetchContent (header-only)
    message(STATUS "doctest not found via find_package or VCPKG_PATH; fetching doctest via FetchContent")
    include(FetchContent)
    FetchContent_Declare(
        doctest
        GIT_REPOSITORY https://github.com/doctest/doctest.git
        GIT_TAG v2.4.12
    )
    FetchContent_MakeAvailable(doctest)
    target_link_libraries(${target} PRIVATE doctest::doctest)
    target_include_directories(${target} PRIVATE ${CMAKE_SOURCE_DIR}/csrc)
endfunction()
