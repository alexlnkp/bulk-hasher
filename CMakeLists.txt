cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(RVC_Hasher)

find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
find_package(OpenMP REQUIRED)

# Define the new shared library target
add_library(hasher MODULE hash.c sha2.c)

# Include Python headers and OpenMP
target_include_directories(hasher PRIVATE ${Python3_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(hasher PRIVATE Python3::Python Python3::Module OpenMP::OpenMP_C)

# Set the output name of the library to 'hasher' without any prefix (like 'lib')
set_target_properties(hasher PROPERTIES PREFIX "")
set_target_properties(hasher PROPERTIES OUTPUT_NAME "hasher")

# Optionally, set the suffix to .pyd for Windows or .so (the default) for Unix
if(WIN32)
    set_target_properties(hasher PROPERTIES SUFFIX ".pyd")
endif()

# Set the C standard and required flag
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

target_compile_options(hasher PRIVATE -Wall -Wextra -Wpedantic -O3 -fopenmp)