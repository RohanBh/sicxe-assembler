cmake_minimum_required(VERSION 3.10)
project(sicxe_assembler)

set(CMAKE_CXX_STANDARD 17)

add_executable(sicxe_assembler main.cpp)

include_directories(assembler)

add_subdirectory(assembler)
add_subdirectory(assembler_tests)

target_link_libraries(sicxe_assembler assembler)
