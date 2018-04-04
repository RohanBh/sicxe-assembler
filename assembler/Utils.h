//
// Created by rohan on 1/4/18.
//

#ifndef SICXE_ASSEMBLER_UTILS_H
#define SICXE_ASSEMBLER_UTILS_H

#include <string>

void toUpper(std::string &str);

void toLower(std::string &str);

int hexStrToInt(std::string s);

std::string intToHexStr(int i, int size = 6);

std::string strToHexStr(std::string &input);

void trim(std::string &s);


#endif //SICXE_ASSEMBLER_UTILS_H
