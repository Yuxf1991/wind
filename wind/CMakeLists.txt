# utils
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(WIND_SRC
    TimeStamp.cpp
    CurrentThread.h
    CurrentThread.cpp
    Buffer.cpp)

add_library(wind ${WIND_SRC})
add_subdirectory(tests)
