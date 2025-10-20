message(STATUS "Configuring design_patterns subdirectory")

# Add template_method example if present
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/template_method/CMakeLists.txt")
    add_subdirectory(template_method)
endif()
