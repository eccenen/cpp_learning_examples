# CMakeLists.txt for third-party library examples
# This directory contains demonstrations of various third-party libraries

message(STATUS "Configuring third_party_examples subdirectory")

# Add spdlog example subdirectory
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/spdlog_example/CMakeLists.txt")
    add_subdirectory(spdlog_example)
endif()

# Future third-party library examples can be added here:
# if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/boost_example/CMakeLists.txt")
#     add_subdirectory(boost_example)
# endif()
# if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/nlohmann_json_example/CMakeLists.txt")
#     add_subdirectory(nlohmann_json_example)
# endif()
