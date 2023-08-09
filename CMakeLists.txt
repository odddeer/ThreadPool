# cmake最低要求
cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 17)

# -g
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)

# 项目名称
project(threadpool)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

include_directories(${PROJECT_SOURCE_DIR})

add_executable(threadpool test.cpp threadpool.cpp)


target_link_libraries(threadpool pthread)
