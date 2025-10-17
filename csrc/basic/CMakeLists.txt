## Build each .cpp in this directory as a standalone executable named basic_<srcname>

# Collect all .cpp files in this directory
file(GLOB BASIC_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp")

if(BASIC_SRCS)
	foreach(src IN LISTS BASIC_SRCS)
		get_filename_component(src_name ${src} NAME_WE)
		set(bin_name "basic_${src_name}")
		add_executable(${bin_name} ${src})
		target_include_directories(${bin_name} PRIVATE ${CMAKE_SOURCE_DIR}/csrc ${CMAKE_CURRENT_SOURCE_DIR})
		target_compile_features(${bin_name} PRIVATE cxx_std_17)
	endforeach()
endif()
