cmake_minimum_required(VERSION 3.28)
project(APP VERSION 1.0.1)

add_subdirectory(soem)
add_executable(read_vals read_vals.c)
add_executable(zero zero.c)

target_link_libraries(read_vals soem)
target_link_libraries(zero soem)
