
file(GLOB BASIC_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp")
if(BASIC_SRCS)
    foreach(src IN LISTS BASIC_SRCS)
        get_filename_component(src_name ${src} NAME_WE)
        set(bin_name "${src_name}")
        add_executable(${bin_name} ${src})
        target_include_directories(${bin_name} PRIVATE ${CMAKE_SOURCE_DIR}/csrc ${CMAKE_CURRENT_SOURCE_DIR})
        target_compile_features(${bin_name} PRIVATE cxx_std_17)
        # Link csrc common deps (includes fmt and other shared libraries)
        target_link_libraries(${bin_name} PRIVATE csrc::common)
        # Place this executable under <build-tree>/bin/design_patterns for easy discovery
        set(design_patterns_bin_dir ${CMAKE_BINARY_DIR}/bin/design_patterns)
        set_target_properties(${bin_name} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${design_patterns_bin_dir}
            RUNTIME_OUTPUT_DIRECTORY_DEBUG ${design_patterns_bin_dir}
            RUNTIME_OUTPUT_DIRECTORY_RELEASE ${design_patterns_bin_dir}
            RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${design_patterns_bin_dir}
            RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${design_patterns_bin_dir}
        )
    endforeach()
endif()
