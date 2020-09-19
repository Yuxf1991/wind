include_directories(include)

file(GLOB WIND_BASE_SRC ./*.cpp)
add_library(wind_base ${WIND_BASE_SRC})
target_link_libraries(wind_base pthread)

add_subdirectory(tests)