cmake_minimum_required(VERSION 3.10)
project(Wind C CXX)

set(CMAKE_CXX_STANDARD 17)

# compile options
add_compile_options(-Wextra -Wall -Werror)
if (CMAKE_BUILD_TYPE MATCHES "Debug")
        add_compile_options(-g)
endif()

include_directories(
        src
        src/base/include
        src/net/include
        third_party/googletest/googletest/include
        third_party/googletest/googlemock/include
)

add_subdirectory(third_party)
add_subdirectory(wind)
