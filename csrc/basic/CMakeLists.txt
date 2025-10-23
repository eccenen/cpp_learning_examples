## Build each .cpp in this directory as a standalone executable named basic_<srcname>

# Collect all .cpp files in this directory
file(GLOB BASIC_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp")

## Ensure runtime output for this directory goes to <build-tree>/bin/basic
set(basic_output_dir ${CMAKE_BINARY_DIR}/bin/basic)
set_property(DIRECTORY PROPERTY RUNTIME_OUTPUT_DIRECTORY ${basic_output_dir})
set_property(DIRECTORY PROPERTY RUNTIME_OUTPUT_DIRECTORY_DEBUG ${basic_output_dir})
set_property(DIRECTORY PROPERTY RUNTIME_OUTPUT_DIRECTORY_RELEASE ${basic_output_dir})
set_property(DIRECTORY PROPERTY RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${basic_output_dir})
set_property(DIRECTORY PROPERTY RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${basic_output_dir})

if(BASIC_SRCS)
	foreach(src IN LISTS BASIC_SRCS)
		get_filename_component(src_name ${src} NAME_WE)
		set(bin_name "basic_${src_name}")
		add_executable(${bin_name} ${src})
		target_include_directories(${bin_name} PRIVATE ${CMAKE_SOURCE_DIR}/csrc ${CMAKE_CURRENT_SOURCE_DIR})
		# Link csrc common deps (includes fmt and other shared libraries)
		target_link_libraries(${bin_name} PRIVATE csrc::common)
		target_compile_features(${bin_name} PRIVATE cxx_std_17)
		# Ensure the executable lands under build/bin/basic
		set_target_properties(${bin_name} PROPERTIES
			RUNTIME_OUTPUT_DIRECTORY ${basic_output_dir}
			RUNTIME_OUTPUT_DIRECTORY_DEBUG ${basic_output_dir}
			RUNTIME_OUTPUT_DIRECTORY_RELEASE ${basic_output_dir}
			RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${basic_output_dir}
			RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${basic_output_dir}
		)
	endforeach()
endif()
