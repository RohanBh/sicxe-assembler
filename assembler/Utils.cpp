//
// Created by rohan on 1/4/18.
//

#include <cstddef>
#include "Utils.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <iostream>

using namespace std;

int hexStrToInt(string &s) {
    toLower(s);
    return stoi(s, nullptr, 16);

}

string intToHexStr(int i, int size) {
    stringstream stream;
    stream << std::setfill('0') << std::setw(size)
           << std::hex << i;
    return stream.str();
}

void toLower(string &str) {
    transform(str.begin(), str.end(), str.begin(), ::tolower);
}

void toUpper(string &str) {
    transform(str.begin(), str.end(), str.begin(), ::toupper);
}

string strToHexStr(string &input) {
    static const char *const hexnums = "0123456789ABCDEF";
    size_t len = input.length();

    string output;
    output.reserve(2 * len);
    for (size_t i = 0; i < len; ++i) {
        const unsigned char c = input[i];
        output.push_back(hexnums[c >> 4]);
        output.push_back(hexnums[c & 15]);
    }
    return output;
}

// trim from start (in place)
void ltrim(string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
        return !std::isspace(ch);
    }));
}

// trim from end (in place)
void rtrim(string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

// trim from both ends (in place)
void trim(string &s) {
    ltrim(s);
    rtrim(s);
}
