include(CheckFunctionExists)
check_function_exists(accept4 HAVE_ACCEPT4)
if (HAVE_ACCEPT4)
    set_source_files_properties(Socket.cpp PROPERTIES COMPILE_FLAGS "-DHAVE_ACCEPT4")
endif ()

file(GLOB WIND_NET_SRC ./*.cpp)
add_library(wind_net ${WIND_NET_SRC})
target_link_libraries(wind_net wind_base pthread)

add_subdirectory(unittests)