cmake_minimum_required(VERSION 3.10)
project(Wind C CXX)

set(CMAKE_CXX_STANDARD 17)
add_compile_options(-Wextra -Wall -Werror -g)
add_compile_definitions(CHECK_PTHREAD_RETURN_VALUE)

include_directories(
        src
        src/base/include
        src/net/include
        thirdparty/googletest/googletest
        thirdparty/googletest/googlemock
)

add_subdirectory(thirdparty/googletest)
add_subdirectory(src)
