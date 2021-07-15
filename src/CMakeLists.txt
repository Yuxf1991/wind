# utils
include_directories(
    utils/include
)
set(UTILS_SRC
    utils/TimeStamp.cpp
)
add_library(wind_utils ${UTILS_SRC})
add_subdirectory(utils/tests)

