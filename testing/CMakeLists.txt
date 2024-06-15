

add_executable(base_test base.c ../hash.c ../sha2.c)
target_include_directories(base_test PRIVATE ${Python3_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(base_test PRIVATE Python3::Python Python3::Module OpenMP::OpenMP_C)

target_compile_options(base_test PRIVATE -Wall -Wextra -Wpedantic -O3 -fopenmp)