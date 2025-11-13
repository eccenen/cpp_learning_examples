message(STATUS "Configuring design_patterns subdirectory")

# Place runtime binaries for this directory and its subdirectories under
# <build-tree>/bin/design_patterns so users can find them easily after build.
set(design_patterns_output_dir ${CMAKE_BINARY_DIR}/bin/design_patterns)
# Use directory properties so subdirectories inherit these locations. Also set
# configuration-specific properties to support multi-config generators.
set_property(DIRECTORY PROPERTY RUNTIME_OUTPUT_DIRECTORY ${design_patterns_output_dir})
set_property(DIRECTORY PROPERTY RUNTIME_OUTPUT_DIRECTORY_DEBUG ${design_patterns_output_dir})
set_property(DIRECTORY PROPERTY RUNTIME_OUTPUT_DIRECTORY_RELEASE ${design_patterns_output_dir})
set_property(DIRECTORY PROPERTY RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${design_patterns_output_dir})
set_property(DIRECTORY PROPERTY RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${design_patterns_output_dir})

# Add template_method example if present
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/template_method/CMakeLists.txt")
    add_subdirectory(template_method)
endif()
