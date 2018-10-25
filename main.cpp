#include <iostream>
#include <PassOne.h>
#include <PassTwo.h>

int main(int argc, char **argv) {
    std::string testFile = "../assembler_tests/basic_assembler_tests/program_block_example.asmb";
    if (argc > 1) {
        int i = 1;
        while (i < argc) {
            std::string intermediate = createIntermediate(argv[i]);
            createObjectFile(intermediate);
            i++;
        }
    }
    return 0;
}
