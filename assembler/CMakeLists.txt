project(assembler)

set(HEADER_FILES PassOne.h PassTwo.h Utils.h Commons.h)

set(SOURCE_FILES PassOne.cpp PassTwo.cpp Utils.cpp Commons.cpp)

add_library(assembler STATIC ${SOURCE_FILES} ${HEADER_FILES})
