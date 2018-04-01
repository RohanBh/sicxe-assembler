//
// Created by rohan on 1/4/18.
//

#ifndef SICXE_ASSEMBLER_COMMONS_H
#define SICXE_ASSEMBLER_COMMONS_H

#include <string>
#include <map>

#define pss std::pair<std::string, std::string>

extern std::map<std::string, pss > OPTAB;
extern std::map<std::string, std::string> SYMTAB;

int initOpTab(std::__cxx11::string projectRoot);

int initSymTab(std::__cxx11::string projectRoot);

#endif //SICXE_ASSEMBLER_COMMONS_H
