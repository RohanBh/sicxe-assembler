project(assembler)

set(HEADER_FILES PassOne.h PassTwo.h)

set(SOURCE_FILES PassOne.cpp PassTwo.cpp)

add_library(assembler STATIC ${SOURCE_FILES} ${HEADER_FILES})
