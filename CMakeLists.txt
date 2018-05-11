cmake_minimum_required (VERSION 3.5 FATAL_ERROR)

project(sctlog)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
add_compile_options(-std=c++11 -Wall -Wpedantic)

add_library(sctlog STATIC sctlog.h sctlog.cc)
target_include_directories(sctlog PUBLIC .)

add_executable(sctlog_test sctlog_test.cc)
target_link_libraries(sctlog_test PRIVATE sctlog)
