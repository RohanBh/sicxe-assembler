//
// Created by rohan on 31/3/18.
//

#include "PassOne.h"
#include "Utils.h"
#include "Commons.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <sstream>

using namespace std;

basic_istream<char, char_traits<char>> &readLine(ifstream &fin, vector<string> &parts);

void printLine(ostream &fout, int locctr, vector<string> &line) {
    std::string outLine = intToHexStr(locctr) + " ";
    for (auto const &s : line) {
        outLine += s + " ";
    }
    fout << outLine << "\n";
}

std::string createIntermediate(std::string filename) {
    initOpTab("/home/rohan/CLionProjects/sicxe_assembler/assembler/");
    initSymTab("/home/rohan/CLionProjects/sicxe_assembler/assembler/");

    vector<string> line;
    ifstream fin(filename.c_str());
    string temp = filename + "_intermediate.txt";
    ofstream fout(temp.c_str());
    string opcode, operand, label;
    int startAddr = 0;
    int locctr = 0;

    while (fin.is_open() && readLine(fin, line)) {
        int oldlocctr = locctr;
        opcode = "";
        operand = "";
        label = "";
        if (line.empty()) {
            continue;
        }
        if (line.size() >= 3) {
            label = line[0];
            opcode = line[1];
            operand = line[2];
        } else if (line.size() == 2) {
            opcode = line[0];
            operand = line[1];
            if (OPTAB.find(opcode) == OPTAB.end()) {
                label = line[0];
                opcode = line[1];
                if (OPTAB.find(opcode) == OPTAB.end()) {
                    cerr << "Undefined Line\n";
                    printLine(cout, oldlocctr, line);
                }
            }
        } else if (line.size() == 1) {
            opcode = line[0];
        }
        if (opcode == "START") {
            startAddr = hexStrToInt(operand);
            locctr = startAddr;
            printLine(fout, oldlocctr, line);
            continue;
        } else if (opcode == "END") {
            printLine(fout, oldlocctr, line);
            break;
        } else {
            if (!label.empty()) {
                if (SYMTAB.find(label) != SYMTAB.end()) {
                    cerr << "Duplicate Symbol!\n";
                    printLine(cout, oldlocctr, line);
                } else {
                    SYMTAB.insert(pss(label, intToHexStr(locctr)));
                }
            }
            auto it = OPTAB.find(opcode);
            if (it != OPTAB.end()) {
                pss info = it->second;
                if (info.first == "3/4") {
                    if (opcode[0] == '+') {
                        locctr += 4;
                    } else {
                        locctr += 3;
                    }
                } else if (info.first == "2") {
                    locctr += 2;
                } else {
                    locctr += 1;
                }
            } else if (opcode == "WORD") {
                locctr += 3;
            } else if (opcode == "RESW") {
                locctr += 3 * stoi(operand, nullptr);
            } else if (opcode == "RESB") {
                locctr += stoi(operand, nullptr);
            } else if (opcode == "BYTE") {
                string substr = operand.substr(2, operand.size() - 3);
                if (tolower(operand[0]) == 'x') {
                    locctr += substr.size() / 2;
                } else {
                    locctr += substr.size();
                }
            } else {
                cerr << "Invalid opcode!\n";
                printLine(cout, oldlocctr, line);
            }
            printLine(fout, oldlocctr, line);
            continue;
        }
    }
    int programLength = locctr - startAddr;
    fout << "length=" << programLength;
    string retVal = fout.is_open() ? temp : "Not created";
    fout.close();
    fin.close();
    return retVal;
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
        parts.push_back(temp);
    }
    return retVal;
}