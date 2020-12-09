set(WIND_BASE_SRC
    Utils.cpp
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