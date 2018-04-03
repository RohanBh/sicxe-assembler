//
// Created by rohan on 31/3/18.
//

#include "PassOne.h"
#include "Utils.h"
#include "Commons.h"
#include <iostream>
#include <sstream>

using namespace std;

void printLine(ostream &fout, int locctr, vector<string> &line) {
    std::string outLine = intToHexStr(locctr) + " ";
    for (auto const &s : line) {
        outLine += s + " ";
    }
    fout << outLine << "\n";
}

string getIntermediateFileName(string assemblyFile) {
    size_t pos = assemblyFile.find(".asmb");
    string intermediateFile;
    if (pos != string::npos) {
        intermediateFile = assemblyFile.substr(0, pos);
    } else {
        intermediateFile = assemblyFile;
    }
    size_t _pos = intermediateFile.find_last_of('/');
    if (_pos != string::npos) {
        intermediateFile = intermediateFile.substr(_pos + 1);
    }
    return intermediateFile + ".imd";
}

std::string createIntermediate(std::string assemblyFile) {
    initOpTab("../");
    initSymTab("../");
    
    ifstream fin(assemblyFile.c_str());
    string intermediateFile = getIntermediateFileName(assemblyFile);
    ofstream fout(intermediateFile.c_str());
    vector<string> line;
    string opcode, operand, label;
    startAddr = 0;
    int locctr = 0;

    if (!fin.is_open()) {
        cerr << "Couldn't open assembly file\n" << "Filename: " << assemblyFile << "\n";
        return "";
    }
    if (!fout.is_open()) {
        cerr << "Could not create intermediate file\n" << "Filename: " << intermediateFile << "\n";
        return "";
    }

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
            if (OPTAB.safeFind(opcode) == OPTAB.end()) {
                label = line[0];
                opcode = line[1];
                if (OPTAB.safeFind(opcode) == OPTAB.end()) {
                    cerr << "Undefined Line\n";
                    printLine(cerr, oldlocctr, line);
                }
            }
        } else if (line.size() == 1) {
            opcode = line[0];
        }
        if (opcode == "START") {
            startAddr = hexStrToInt(operand);
            locctr = startAddr;
            printLine(fout, oldlocctr, line);
            programName = label;
            if (programName.empty()) {
                programName = assemblyFile;
            }
            continue;
        } else if (opcode == "END") {
            printLine(fout, oldlocctr, line);
            break;
        } else if (opcode == "BASE" || opcode == "NOBASE") {
            printLine(fout, oldlocctr, line);
            continue;
        } else {
            if (!label.empty()) {
                if (SYMTAB.find(label) != SYMTAB.end()) {
                    cerr << "Duplicate Symbol!\n";
                    printLine(cerr, oldlocctr, line);
                } else {
                    SYMTAB.insert(pss(label, intToHexStr(locctr)));
                }
            }
            auto it = OPTAB.safeFind(opcode);
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
                printLine(cerr, oldlocctr, line);
            }
            printLine(fout, oldlocctr, line);
            continue;
        }
    }
    programLength = locctr - startAddr;
    string retVal = fout.is_open() ? intermediateFile : "Not created";
    fout.close();
    fin.close();
    return retVal;
}

