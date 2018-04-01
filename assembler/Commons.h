//
// Created by rohan on 1/4/18.
//

#ifndef SICXE_ASSEMBLER_COMMONS_H
#define SICXE_ASSEMBLER_COMMONS_H

#include <string>
#include <map>
#include <vector>
#include <fstream>

#define pss std::pair<std::string, std::string>
#define safeFind(opcode) find(opcode[0] == '+' ? opcode.substr(1) : opcode)
#define safe(opcode) (opcode[0] == '+' ? opcode.substr(1) : opcode)

extern std::map<std::string, pss > OPTAB;
extern std::map<std::string, std::string> SYMTAB;

extern std::string programName;
extern int startAddr;
extern int programLength;

int initOpTab(std::__cxx11::string projectRoot);

int initSymTab(std::__cxx11::string projectRoot);

std::basic_istream<char, std::char_traits<char>> &
readLine(std::ifstream &fin, std::vector<std::string> &parts);

#endif //SICXE_ASSEMBLER_COMMONS_H
