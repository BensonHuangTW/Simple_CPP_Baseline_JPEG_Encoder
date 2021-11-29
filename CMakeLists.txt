#
#  libCPPEG - Simple JPEG Encoder
# ===============================================
#
# CMake build script

cmake_minimum_required(VERSION 3.0)
project(cppeg)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING
      "Choose the type of build, options are: Debug Release."
      FORCE)
endif(NOT CMAKE_BUILD_TYPE)

if(CMAKE_COMPILER_IS_GNUCXX OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -g")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")
endif()

# Add sources
file(GLOB SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp" "${PROJECT_SOURCE_DIR}/*.cpp")

# Specify include directory
find_package( OpenCV 4.0.0 REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories("${PROJECT_SOURCE_DIR}/include/")
link_directories(${OpenCV_LIB_DIR})

# Compile and generate the executable
add_executable(cppeg main.cpp src/RLC.cpp src/Encoder.cpp src/HuffmanTree.cpp src/Transform.cpp src/Utility.cpp)
target_link_libraries(cppeg ${OpenCV_LIBS})

set_property(TARGET cppeg PROPERTY CXX_STANDARD 17)
set_property(TARGET cppeg PROPERTY CXX_STANDARD_REQUIRED ON)