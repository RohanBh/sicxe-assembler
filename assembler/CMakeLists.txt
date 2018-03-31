project(assembler)

set(HEADER_FILES PassOne.h)

set(SOURCE_FILES PassOne.cpp)

add_library(assembler STATIC ${SOURCE_FILES} ${HEADER_FILES})
