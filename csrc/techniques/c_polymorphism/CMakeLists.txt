# CMakeLists.txt for C polymorphism demo
# This example uses pure C language to demonstrate OOP concepts

# Collect source files
set(C_POLY_SOURCES
    shape.c
    c_polymorphism_demo.c
)

# Create executable
set(target_name techniques_c_polymorphism)
add_executable(${target_name} ${C_POLY_SOURCES})

# Define _GNU_SOURCE for M_PI constant
target_compile_definitions(${target_name} PRIVATE _GNU_SOURCE)

# Include directories
target_include_directories(${target_name} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Use C11 standard
set_target_properties(${target_name} PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS OFF
)

# Link math library (for M_PI and fabs)
target_link_libraries(${target_name} PRIVATE m)

# Set output directory
set(techniques_output_dir ${CMAKE_BINARY_DIR}/bin/techniques)
set_target_properties(${target_name} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${techniques_output_dir}
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${techniques_output_dir}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${techniques_output_dir}
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${techniques_output_dir}
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${techniques_output_dir}
)
