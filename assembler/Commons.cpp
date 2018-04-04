//
// Created by rohan on 1/4/18.
//

#include <utility>
#include <vector>
#include "PassOne.h"
#include "Commons.h"
#include "Utils.h"
#include <sstream>
#include <iostream>
#include <algorithm>

using namespace std;

string *split(string line);

basic_istream<char, char_traits<char>> &readLine(ifstream &fin, vector<string> &parts);

std::map<std::string, pss > OPTAB;
std::map<std::string, std::string> SYMTAB;
std::map<std::string, Block> BLOCKTAB;

string programName;
int startAddr;
int programLength;

int initOpTab(string projectRoot) {
    if (!OPTAB.empty()) {
        return static_cast<int>(OPTAB.size());
    }
    std::ifstream fin(projectRoot + "assembler/opcodes.txt");
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
    std::cerr << "Missing file: opcodes.txt\n";
    return -1;
}

int initSymTab(string projectRoot) {
    if (!SYMTAB.empty()) {
        return static_cast<int>(SYMTAB.size());
    }
    std::ifstream fin(projectRoot + "assembler/symbols.txt");
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
    std::cerr << "Missing file: symbols.txt\n";
    return -1;
}

string *split(string line) {
    string delimiter = " ";
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

basic_istream<char, char_traits<char>> &readLine(ifstream &fin, vector<string> &parts) {
    parts.clear();
    string line, temp;
    basic_istream<char, char_traits<char>> &retVal = getline(fin, line);
    stringstream s(line);
    while (s >> temp) {
        if (temp[0] == '.') {
            break;
        }
        trim(temp);
        parts.push_back(temp);
    }
    return retVal;
}

void insertBlock(std::string blockName, Block blockInfo) {
    auto val = pair<std::string, Block>(blockName, blockInfo);
    auto pitb = BLOCKTAB.insert(val);
    if (!pitb.second) {
        auto it = BLOCKTAB.find(blockName);
        it->second = blockInfo;
    }
}

bool compare(pair<string, Block> a, pair<string, Block> b) {
    return a.second.index < b.second.index;
}

void updateBlockAddr() {
    vector<pair<string, Block>> blocks;
    for (auto x: BLOCKTAB) {
        blocks.emplace_back(x);
    }
    sort(blocks.begin(), blocks.end(), compare);
    for (int i = 1; i < blocks.size(); i++) {
        Block &lastBlockInfo = blocks[i - 1].second;
        Block &currBlockInfo = blocks[i].second;
        currBlockInfo.blockAddr = intToHexStr(
                hexStrToInt(lastBlockInfo.blockLength) + hexStrToInt(lastBlockInfo.blockAddr));
        BLOCKTAB.find(blocks[i].first)->second.blockAddr = currBlockInfo.blockAddr;
    }
}
