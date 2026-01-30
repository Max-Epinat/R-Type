cmake_minimum_required(VERSION 3.20)
project(rtype LANGUAGES CXX)

option(RTYPE_BUILD_SERVER "Build the R-Type dedicated server" ON)
option(RTYPE_BUILD_CLIENT "Build the R-Type client" ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

add_subdirectory(src)
