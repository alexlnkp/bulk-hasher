cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(RVC_Hasher)

find_package(Python3 REQUIRED)
find_package(OpenMP REQUIRED)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(RVC_Hasher hash.c sha2.c)

target_compile_options(RVC_Hasher PRIVATE -Wall -Wextra -Wpedantic -O3)
target_link_libraries(RVC_Hasher PRIVATE python3.12 OpenMP::OpenMP_C)
target_include_directories(RVC_Hasher PRIVATE /usr/include/python3.12)
