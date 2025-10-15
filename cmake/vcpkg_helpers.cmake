include(CMakeParseArguments)
# Variant that only locates or fetches a package and returns info via variables.
# Usage: find_or_fetch_package_config(<PkgName> <Prefix> [HEADER <path>] [GIT_REPO <url> GIT_TAG <tag>])
# Returns variables (set in parent scope):
#   <Prefix>_FOUND (ON/OFF)
#   <Prefix>_TARGET (e.g. pkg::pkg) if available
#   <Prefix>_INCLUDE (path) if using header fallback
function(find_or_fetch_package_config pkg prefix)
    set(options)
    set(oneValueArgs HEADER GIT_REPO GIT_TAG)
    set(multiValueArgs)
    cmake_parse_arguments(PARSE_ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(${prefix}_FOUND OFF PARENT_SCOPE)
    set(${prefix}_TARGET "" PARENT_SCOPE)
    set(${prefix}_INCLUDE "" PARENT_SCOPE)

    # discover VCPKG_PATH if needed
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
        set(VCPKG_INCLUDE_DIR "${VCPKG_PATH}/installed/${VCPKG_TARGET_TRIPLET}/include")
    else()
        set(VCPKG_INCLUDE_DIR "")
    endif()

    # Try find_package
    find_package(${pkg} CONFIG QUIET)
    if(${pkg}_FOUND)
        set(${prefix}_FOUND ON PARENT_SCOPE)
        set(${prefix}_TARGET "${pkg}::${pkg}" PARENT_SCOPE)
        return()
    endif()

    # header fallback via vcpkg include
    if(PARSE_ARG_HEADER AND VCPKG_INCLUDE_DIR AND EXISTS "${VCPKG_INCLUDE_DIR}/${PARSE_ARG_HEADER}")
        set(${prefix}_FOUND ON PARENT_SCOPE)
        set(${prefix}_INCLUDE "${VCPKG_INCLUDE_DIR}" PARENT_SCOPE)
        return()
    endif()

    # FetchContent fallback
    if(PARSE_ARG_GIT_REPO)
        include(FetchContent)
        FetchContent_Declare(
            ${pkg}
            GIT_REPOSITORY ${PARSE_ARG_GIT_REPO}
            GIT_TAG ${PARSE_ARG_GIT_TAG}
        )
        FetchContent_MakeAvailable(${pkg})
        if(TARGET ${pkg}::${pkg})
            set(${prefix}_FOUND ON PARENT_SCOPE)
            set(${prefix}_TARGET "${pkg}::${pkg}" PARENT_SCOPE)
        else()
            # If no CMake target, still mark found but no target
            set(${prefix}_FOUND ON PARENT_SCOPE)
        endif()
        return()
    endif()

    # nothing found
    set(${prefix}_FOUND OFF PARENT_SCOPE)
endfunction()
