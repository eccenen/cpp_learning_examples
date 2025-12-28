# OpenMP examples
message(STATUS "Configuring OpenMP examples")

# Create executable for hp.cpp
add_executable(hp hp.cpp)

# Set C++ standard
target_compile_features(hp PRIVATE cxx_std_17)

# Get the compiler directory to check for built-in OpenMP
get_filename_component(COMPILER_DIR ${CMAKE_CXX_COMPILER} DIRECTORY)
get_filename_component(COMPILER_ROOT ${COMPILER_DIR} DIRECTORY)

# Try to find OpenMP package (may fail for custom clang++ installations)
find_package(OpenMP)

if(OpenMP_CXX_FOUND)
    # Standard OpenMP found via CMake
    message(STATUS "OpenMP found via CMake: ${OpenMP_CXX_VERSION}")
    target_link_libraries(hp PRIVATE OpenMP::OpenMP_CXX)
else()
    # Fallback: manually configure OpenMP for clang++ with system libomp
    message(STATUS "OpenMP not found via CMake, trying manual configuration...")
    
    # Check if we're using clang
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        message(STATUS "Using Clang compiler, configuring OpenMP manually...")
        message(STATUS "Compiler root directory: ${COMPILER_ROOT}")
        
        # Add OpenMP compile flags
        target_compile_options(hp PRIVATE -fopenmp)
        
        # Try to find libomp in common locations (including compiler's own directory)
        find_library(OMP_LIBRARY
            NAMES omp gomp iomp5
            PATHS
                ${COMPILER_ROOT}/lib
                /usr/lib
                /usr/lib/x86_64-linux-gnu
                /usr/lib/llvm-14/lib
                /usr/lib/llvm-15/lib
                /usr/lib/llvm-16/lib
                /usr/local/lib
            NO_DEFAULT_PATH
        )
        
        # If not found, try default paths
        if(NOT OMP_LIBRARY)
            find_library(OMP_LIBRARY NAMES omp gomp iomp5)
        endif()
        
        if(OMP_LIBRARY)
            message(STATUS "Found OpenMP library: ${OMP_LIBRARY}")
            target_link_libraries(hp PRIVATE ${OMP_LIBRARY})
        else()
            # Just try linking with -lomp flag
            message(STATUS "OpenMP library not found in standard paths, using -lomp linker flag")
            target_link_options(hp PRIVATE -fopenmp)
        endif()
        
        # Add include path for omp.h (including compiler's own directory)
        find_path(OMP_INCLUDE_DIR
            NAMES omp.h
            PATHS
                ${COMPILER_ROOT}/include
                /usr/include
                /usr/lib/llvm-14/include
                /usr/lib/llvm-15/include
                /usr/lib/llvm-16/include
                /usr/local/include
            NO_DEFAULT_PATH
        )
        
        # If not found, try default paths
        if(NOT OMP_INCLUDE_DIR)
            find_path(OMP_INCLUDE_DIR NAMES omp.h)
        endif()
        
        if(OMP_INCLUDE_DIR)
            message(STATUS "Found OpenMP include directory: ${OMP_INCLUDE_DIR}")
            target_include_directories(hp PRIVATE ${OMP_INCLUDE_DIR})
        else()
            message(FATAL_ERROR "Could not find omp.h header file!")
        endif()
    else()
        message(FATAL_ERROR "OpenMP not found and compiler is not Clang. Please install OpenMP support.")
    endif()
endif()

# Print build info
message(STATUS "OpenMP example 'hp' configured successfully")
