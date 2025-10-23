# For each subdirectory under techniques, search for .cpp files and create executables
file(GLOB CHILD_DIRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*)

# Ensure runtime output for techniques goes to <build-tree>/bin/techniques
set(techniques_output_dir ${CMAKE_BINARY_DIR}/bin/techniques)
set_property(DIRECTORY PROPERTY RUNTIME_OUTPUT_DIRECTORY ${techniques_output_dir})
set_property(DIRECTORY PROPERTY RUNTIME_OUTPUT_DIRECTORY_DEBUG ${techniques_output_dir})
set_property(DIRECTORY PROPERTY RUNTIME_OUTPUT_DIRECTORY_RELEASE ${techniques_output_dir})
set_property(DIRECTORY PROPERTY RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${techniques_output_dir})
set_property(DIRECTORY PROPERTY RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${techniques_output_dir})
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
            # Link csrc common deps (includes fmt and other shared libraries)
            target_link_libraries(${tgt_name} PRIVATE csrc::common)
            # set target runtime output to build/bin/techniques
            set_target_properties(${tgt_name} PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY ${techniques_output_dir}
                RUNTIME_OUTPUT_DIRECTORY_DEBUG ${techniques_output_dir}
                RUNTIME_OUTPUT_DIRECTORY_RELEASE ${techniques_output_dir}
                RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${techniques_output_dir}
                RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${techniques_output_dir}
            )
        endif()
    endif()
endforeach()
