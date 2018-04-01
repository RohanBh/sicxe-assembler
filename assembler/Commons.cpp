//
// Created by rohan on 1/4/18.
//

#include "PassOne.h"
#include "Commons.h"
#include <fstream>

using namespace std;

string *split(string line);

std::map<std::string, pss > OPTAB;
std::map<std::string, std::string> SYMTAB;

int initOpTab(string projectRoot) {
    if (!OPTAB.empty()) {
        return static_cast<int>(OPTAB.size());
    }
    std::ifstream fin(projectRoot + "opcodes.txt");
    if (fin.is_open()) {
        string line;
        while (getline(fin, line)) {
            string *parts = split(line);
            auto val = std::pair<string, pss >(parts[0], pss(parts[1], parts[2]));
            OPTAB.insert(val);
        }
        fin.close();
        return (int) OPTAB.size();
    }
    return -1;
}

int initSymTab(string projectRoot) {
    if (!SYMTAB.empty()) {
        return static_cast<int>(SYMTAB.size());
    }
    std::ifstream fin(projectRoot + "symbols.txt");
    if (fin.is_open()) {
        string line;
        while (getline(fin, line)) {
            string *parts = split(line);
            auto val = std::pair<string, string>(parts[0], parts[1]);
            SYMTAB.insert(val);
        }
        fin.close();
        return (int) SYMTAB.size();
    }
    return -1;
}

string *split(string line) {
    std::string delimiter = " ";
    size_t pos = 0;
    int i = 0;
    string token;
    auto values = new string[3];
    while ((pos = line.find(delimiter)) != string::npos) {
        token = line.substr(0, pos);
        values[i++] = token;
        line.erase(0, pos + delimiter.length());
    }
    values[i] = line;
    return values;
}
