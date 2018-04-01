#include <iostream>
#include <PassOne.h>
#include <PassTwo.h>

int main() {
    std::string intermediate =
            createIntermediate(
                    "/home/rohan/CLionProjects/sicxe_assembler/assembler_tests/basic_assembler_tests/EvenOddSeparator.asm");
    createObjectFile(intermediate);
    return 0;
}
