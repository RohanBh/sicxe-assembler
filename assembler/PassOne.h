//
// Created by rohan on 31/3/18.
//

#ifndef SICXE_ASSEMBLER_PASSONE_H
#define SICXE_ASSEMBLER_PASSONE_H

#include <string>

std::string createIntermediate(std::string filename);

int initOpTab(std::string projectRoot);

int initSymTab(std::string projectRoot);

#endif //SICXE_ASSEMBLER_PASSONE_H
