cmake_minimum_required(VERSION 3.10)

message(STATUS "Configuring csrc/techniques subdirectory")

# For each subdirectory under techniques, search for .cpp files and create executables
file(GLOB CHILD_DIRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*)
foreach(d ${CHILD_DIRS})
    if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${d})
        # collect all cpp files under this subdir
        file(GLOB SRCS ${CMAKE_CURRENT_SOURCE_DIR}/${d}/*.cpp)
        if(SRCS)
            # target name: techniques_<subdir>
            string(REPLACE "/" "_" subdir_name ${d})
            set(tgt_name "techniques_${subdir_name}")
            add_executable(${tgt_name} ${SRCS})
            target_include_directories(${tgt_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/${d} ${CMAKE_SOURCE_DIR}/csrc)
            target_compile_features(${tgt_name} PRIVATE cxx_std_17)
        endif()
    endif()
endforeach()
