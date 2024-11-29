cmake_minimum_required(VERSION 3.29)
project(IMS)

set(CMAKE_CXX_STANDARD 17)

add_executable(IMS main.cpp)
# Specify the SIMLIB library and headers
target_include_directories(IMS PRIVATE /usr/local/include)
target_link_directories(IMS PRIVATE /usr/local/lib)

# Link the SIMLIB library
target_link_libraries(IMS PRIVATE simlib)