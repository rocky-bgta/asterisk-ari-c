cmake_minimum_required(VERSION 3.10)

project(asterisk-ari-c)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set the vcpkg toolchain file if VCPKG_ROOT environment variable is defined
if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
endif()

# Find Boost libraries
find_package(Boost REQUIRED COMPONENTS program_options)

if(NOT Boost_FOUND)
    message(FATAL_ERROR "Could not find Boost")
endif()

# Include directories for Boost headers and your project headers
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Add the executable
add_executable(asterisk-ari-c src/main.cpp)

# Link Boost libraries
target_link_libraries(asterisk-ari-c PRIVATE Boost::program_options)