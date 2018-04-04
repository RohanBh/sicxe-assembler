#include <iostream>
#include <PassOne.h>
#include <PassTwo.h>

int main() {
    std::string intermediate = createIntermediate("../assembler_tests/basic_assembler_tests/program_block_example.asmb");
    //createObjectFile(intermediate);
    return 0;
}
