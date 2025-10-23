message(STATUS "Configuring csrc subdirectory")
option(BUILD_EXAMPLES "Build examples in csrc" ON)
if(BUILD_EXAMPLES)
    # Create a single INTERFACE library for all csrc common dependencies.
    # This is more maintainable than creating separate interface targets
    # per subdirectory (basic, techniques, design_patterns).
    # All subdirectories link against csrc::common to access fmt and other shared deps.
    if(NOT TARGET csrc_common)
        add_library(csrc_common INTERFACE)
        # Link all common third-party dependencies here (fmt is found at project root)
        target_link_libraries(csrc_common INTERFACE fmt::fmt)
        # Add csrc directory to include path so all subdirectories can use #include "common.h"
        target_include_directories(csrc_common INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
        # Expose a namespaced alias for cleaner usage in subdirectories
        add_library(csrc::common ALIAS csrc_common)
    endif()

    # add basic and techniques subdirs if present
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/basic/CMakeLists.txt")
        add_subdirectory(basic)
    endif()
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/techniques/CMakeLists.txt")
        add_subdirectory(techniques)
    endif()
    # add design_patterns subdir for design pattern examples
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/design_patterns/CMakeLists.txt")
        add_subdirectory(design_patterns)
    endif()
endif()
