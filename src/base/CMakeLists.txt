set(WIND_BASE_SRC
    Condition.cpp
    CountDownLatch.cpp
    CurrentThread.cpp
    Thread.cpp
    TimeStamp.cpp
    TimeZone.cpp
)
add_library(wind_base ${WIND_BASE_SRC})
target_link_libraries(wind_base pthread)

add_subdirectory(unittests)