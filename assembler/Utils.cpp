//
// Created by rohan on 1/4/18.
//

#include <cstddef>
#include "Utils.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

using namespace std;

int hexStrToInt(string &s) {
    toLower(s);
    return stoi(s, nullptr, 16);
}

string intToHexStr(int i) {
    stringstream stream;
    stream << std::setfill('0') << std::setw(sizeof(int) * 2)
           << std::hex << i;
    return stream.str();
}

void toLower(string &str) {
    transform(str.begin(), str.end(), str.begin(), ::tolower);
}

void toUpper(string &str) {
    transform(str.begin(), str.end(), str.begin(), ::toupper);
}
