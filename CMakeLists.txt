cmake_minimum_required(VERSION 3.22)
project(PerfScope LANGUAGES CXX)
set(PROJECT_NAME PerfScope)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Specify the source files
file(GLOB_RECURSE TEST_SOURCES tests/*.cpp)
# Add the executable target
add_executable(${PROJECT_NAME} ${TEST_SOURCES})

find_package(PkgConfig REQUIRED)
find_package(GTest REQUIRED)

pkg_check_modules(LIBCAP REQUIRED libcap)

# If you need to link any libraries, you can specify them here
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(${PROJECT_NAME} gtest::gtest)