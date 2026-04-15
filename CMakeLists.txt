cmake_minimum_required(VERSION 3.28)
project(APP VERSION 1.0.1)

add_subdirectory(soem)
add_executable(app main.c)
target_link_libraries(app soem)
