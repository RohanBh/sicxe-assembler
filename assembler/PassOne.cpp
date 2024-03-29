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

string getIntermediateFileName(const string &assemblyFile) {
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

    string intermediateFile = getIntermediateFileName(assemblyFile);
    ifstream fin(assemblyFile.c_str());
    ofstream fout(intermediateFile.c_str());
    ofstream log("log.txt");
    log << "Error at:\n";

    vector<string> line;
    string opcode, operand, label;
    startAddr = 0;
    int locctr = 0;
    int lineNum = 5;
    string currBlock = "DEFAULT";
    int blockIndex = 0;
    bool errorFlag = false;
    insertBlock(currBlock, Block(blockIndex, intToHexStr(startAddr), intToHexStr(locctr)));

    if (!fin.is_open()) {
        cerr << "Couldn't open assembly file\n" << "Filename: " << assemblyFile << "\n";
        return "";
    }
    if (!fout.is_open()) {
        cerr << "Could not create intermediate file\n" << "Filename: " << intermediateFile << "\n";
        return "";
    }

    while (readLine(fin, line)) {
        int currLinelocctr = locctr;
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
                    log << "Line " << lineNum << ", Unknown Line: ";
                    printLine(log, currLinelocctr, line);
                    errorFlag = true;
                    cout << "\n";
                }
            }
        } else if (line.size() == 1) {
            opcode = line[0];
        }
        if (opcode == "START") {
            startAddr = hexStrToInt(operand);
            locctr = startAddr;
            printLine(fout, currLinelocctr, line);
            programName = label;
            if (programName.empty()) {
                programName = assemblyFile;
            }
            insertBlock(currBlock, Block(blockIndex, intToHexStr(startAddr), intToHexStr(locctr)));
        } else if (opcode == "END") {
            printLine(fout, currLinelocctr, line);
            // update current block length
            auto it_curr = BLOCKTAB.find(currBlock);
            it_curr->second.blockLength = intToHexStr(locctr);
            break;
        } else if (opcode == "BASE" || opcode == "NOBASE") {
            printLine(fout, currLinelocctr, line);
        } else if (opcode == "USE") {
            printLine(fout, currLinelocctr, line);
            string nextBlock = "DEFAULT";
            if (!operand.empty()) {
                nextBlock = operand;
            }
            // check if block already exists
            auto it_next = BLOCKTAB.find(nextBlock);
            if (it_next == BLOCKTAB.end()) {
                blockIndex++;
                insertBlock(nextBlock, Block(blockIndex, intToHexStr(0), intToHexStr(0)));
                it_next = BLOCKTAB.find(nextBlock);
            }
            // update current block length
            auto it_curr = BLOCKTAB.find(currBlock);
            it_curr->second.blockLength = intToHexStr(currLinelocctr);
            // update locctr
            locctr = hexStrToInt(it_next->second.blockLength);
            currBlock = nextBlock;
        } else {
            if (!label.empty()) {
                if (SYMTAB.find(label) != SYMTAB.end()) {
                    log << "Line " << lineNum << ", Duplicate Symbol!\n";
                    printLine(log, currLinelocctr, line);
                    cout << "\n";
                    errorFlag = true;
                } else {
                    SYMTAB.insert(pair<string, pss >(label, pss(intToHexStr(locctr), currBlock)));
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
                log << "Line " << lineNum << ", Invalid opcode!\n";
                printLine(log, currLinelocctr, line);
                cout << "\n";
                errorFlag = true;
            }
            printLine(fout, currLinelocctr, line);
        }
        lineNum += 5;
    }
    string retVal = fout.is_open() ? intermediateFile : "Not created";
    fout.close();
    fin.close();
    log.close();
    Block lastBlock = updateBlockAddr();
    programLength = hexStrToInt(lastBlock.blockAddr) + hexStrToInt(lastBlock.blockLength);
    if (errorFlag) {
        cerr << "Assembly not successful, check log.txt for more info\n";
    }
    else{
        cout << "Intermediate File successfully generated\n";
    }
    return retVal;
}

