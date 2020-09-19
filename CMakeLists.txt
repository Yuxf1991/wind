cmake_minimum_required(VERSION 3.17)
project(Wind C CXX)

set(CMAKE_CXX_STANDARD 17)

include_directories(
        thirdparty/googletest/googletest
        thirdparty/googletest/googlemock
)

add_subdirectory(thirdparty/googletest)
add_subdirectory(src/base)