include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

set(BASIC_TEST_SOURCE basic_assembler_check.cpp)

add_executable(runBasicAssemblerTest ${BASIC_TEST_SOURCE})

target_link_libraries(runBasicAssemblerTest gtest gtest_main)
target_link_libraries(runBasicAssemblerTest assembler)
