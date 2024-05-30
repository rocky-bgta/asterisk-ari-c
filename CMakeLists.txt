cmake_minimum_required(VERSION 3.14)

# Project name and version
project(MyProject VERSION 1.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set the path to the vcpkg toolchain file
#set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake")
set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# List of source files explicitly
set(SOURCES
    #${PROJECT_SOURCE_DIR}/src/main.cpp
    #${PROJECT_SOURCE_DIR}/src/hold_call.cpp
    #${PROJECT_SOURCE_DIR}/src/mixing_bridge.cpp

    ${PROJECT_SOURCE_DIR}/src/holding_bridge.cpp
    #${PROJECT_SOURCE_DIR}/src/query.cpp
    #${PROJECT_SOURCE_DIR}/src/other_source_file1.cpp
    #${PROJECT_SOURCE_DIR}/src/other_source_file2.cpp
    # Add other necessary source files here
)

# Add an executable
add_executable(MyProject ${SOURCES})

# Find and link Boost libraries
find_package(Boost REQUIRED COMPONENTS program_options)
if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    target_link_libraries(MyProject Boost::program_options)
endif()

find_package(nlohmann_json CONFIG REQUIRED)
if(nlohmann_json_FOUND)
    include_directories(${nlohmann_json_INCLUDE_DIRS})
    target_link_libraries(MyProject nlohmann_json::nlohmann_json)
endif()

# Example: fmt
#[[find_package(fmt REQUIRED)
if (fmt_FOUND)
    target_link_libraries(MyProject fmt::fmt)
endif()]]
