//
// Created by rohan on 1/4/18.
//

#include "PassTwo.h"
#include "Commons.h"
#include "Utils.h"
#include <sstream>
#include <iostream>


using namespace std;

int parse(string operand);

string assInstr_34(string opcodeHex, int ni, int xbpe, int actualOperandVal);

string assInstr_2(string opcodeHex, string r1, string r2);

template<typename T>
int getNextInstrAdd(T it_op, int locctr, string opcode);

bool checkPCRel(int operandVal) {
    return operandVal >= -2048 && operandVal < 2047;
}

bool checkBaseRel(int operandVal) {
    return operandVal > 0 && operandVal < 4095;
}

template<typename T, typename U>
int calcPCRelOperand(T it_op, string locctr, string opcode, U it_sym) {
    int pcPos = getNextInstrAdd(it_op, hexStrToInt(locctr), opcode);
    int symVal = hexStrToInt(it_sym->second);
    return symVal - pcPos;
}

template<typename T>
int calcBaseRelOperand(string baseHex, T it_sym) {
    int basePos = hexStrToInt(baseHex);
    int symVal = hexStrToInt(it_sym->second);
    return symVal - basePos;
}

void writeTextRecord(ostream &fout, string &textRecord, string &objectCode) {
    int totalBytes = static_cast<int>(objectCode.size() / 2);
    if (totalBytes == 0) {
        return;
    }
    textRecord += intToHexStr(totalBytes, 2);
    textRecord += objectCode;
    fout << textRecord << "\n";
    textRecord = "T";
    objectCode = "";
}

string createObjectFile(std::string filename) {
    ifstream fin(filename.c_str());
    if (!fin.is_open()) {
        cerr << "Intermediate File doesn't exits\n" << "Filename: " << filename << "\n";
        return "";
    }
    vector<string> line;
    string outFile = filename.substr(0, filename.find("_intermediate.txt")) + ".object";
    ofstream fout(outFile.c_str());
    if (!fout.is_open()) {
        cerr << "Can not open out file\n" << "Filename: " << outFile << "\n";
        return "";
    }
    string opcode, operand, label, locctr;
    string baseLocHex;
    string textRecord = "T";
    string objectCode = "";
    bool canUseBase = false;
    while (readLine(fin, line)) {
        if (line.empty() || line.size() == 1) {
            continue;
        }
        locctr = line[0];
        opcode = "";
        operand = "";
        label = "";
        if (line.size() >= 4) {
            label = line[1];
            opcode = line[2];
            operand = line[3];
        } else if (line.size() == 3) {
            opcode = line[1];
            operand = line[2];
            if (OPTAB.safeFind(opcode) == OPTAB.end()) {
                label = line[1];
                opcode = line[2];
                if (OPTAB.safeFind(operand) == OPTAB.end()) {
                    cerr << "Unknown Line\n";
                }
            }
        } else if (line.size() == 2) {
            opcode = line[1];
        }
        if (opcode == "START") {
            string headername = programName;
            if (headername.size() < 6) {
                while (headername.size() != 6) {
                    headername += " ";
                }
            } else if (headername.size() > 6) {
                headername = headername.substr(0, 6);
            }
            fout << "H" << headername << intToHexStr(startAddr) << intToHexStr(programLength) << "\n";
        } else if (opcode == "BASE") {
            auto it_sym = SYMTAB.find(operand);
            baseLocHex = it_sym->second;
            canUseBase = true;
        } else if (opcode == "NOBASE") {
            canUseBase = false;
        } else if (opcode == "END") {
            writeTextRecord(fout, textRecord, objectCode);
            string firstExIntrHex = intToHexStr(startAddr);
            if (!operand.empty()) {
                auto it = SYMTAB.find(operand);
                firstExIntrHex = it->second;
            }
            fout << "E" << firstExIntrHex;
            break;
        } else {
            if (textRecord == "T") {
                textRecord += locctr;
            }
            auto it_op = OPTAB.safeFind(opcode);
            string instructionHex = "";
            if (it_op != OPTAB.end()) {
                pss instrInfo = it_op->second;
                switch (parse(operand)) {
                    case 0: {
                        if (instrInfo.first == "1") {
                            instructionHex = instrInfo.second;
                        } else if (instrInfo.first == "2") {
                            cerr << "No operand specified for " << opcode << " instruction\n";
                        } else if (safe(opcode) == "RSUB") {
                            int xbpe = opcode[0] == '+' ? 1 : 0;
                            instructionHex = assInstr_34(instrInfo.second, 3, xbpe, 0);
                        } else {
                            cerr << "No operand specified for ";
                        }
                        break;
                    }
                    case 1: {
                        string _operand = operand.substr(1);
                        auto it_sym = SYMTAB.find(_operand);
                        if (it_sym != SYMTAB.end()) {
                            if (instrInfo.first == "3/4") {
                                int pcRelOp = calcPCRelOperand(it_op, locctr, opcode, it_sym);
                                int baseRelOp = canUseBase ? calcBaseRelOperand(baseLocHex, it_sym) : -1;
                                if (opcode[0] == '+') {
                                    instructionHex = assInstr_34(instrInfo.second, 1, 1, hexStrToInt(it_sym->second));
                                } else if (checkPCRel(pcRelOp)) {
                                    instructionHex = assInstr_34(instrInfo.second, 1, 2, pcRelOp);
                                } else if (canUseBase && checkBaseRel(baseRelOp)) {
                                    instructionHex = assInstr_34(instrInfo.second, 1, 4, baseRelOp);
                                } else {
                                    cerr << "Can't use PC rel or Base rel addr to assemble this instr\n";
                                }
                            } else {
                                cerr << "Can't use Immediate addr with format " << instrInfo.first << "\n";
                            }
                        } else {
                            unsigned int operand_dec;
                            try {
                                operand_dec = static_cast<unsigned int>(stoi(_operand));
                            }
                            catch (const invalid_argument &ia) {
                                cerr << "Unspecified immediate operand" << _operand << "\n";
                                break;
                            }
                            if (opcode[0] == '+') {
                                if (operand_dec > 1048575) {
                                    cerr << "operand too large to fit in format 4: " << operand;
                                } else {
                                    instructionHex = assInstr_34(instrInfo.second, 1, 1, operand_dec);
                                }
                            } else if (operand_dec < 4096) {
                                instructionHex = assInstr_34(instrInfo.second, 1, 0, operand_dec);
                            } else {
                                cerr << "operand too large to fit in format 3: " << operand;
                            }
                        }
                        break;
                    }
                    case 2: {
                        string _operand = operand.substr(1);
                        auto it_sym = SYMTAB.find(_operand);
                        if (it_sym != SYMTAB.end()) {
                            if (instrInfo.first == "3/4") {
                                int pcRelOp = calcPCRelOperand(it_op, locctr, opcode, it_sym);
                                int baseRelOp = canUseBase ? calcBaseRelOperand(baseLocHex, it_sym) : -1;
                                if (opcode[0] == '+') {
                                    instructionHex = assInstr_34(instrInfo.second, 2, 1, hexStrToInt(it_sym->second));
                                } else if (checkPCRel(pcRelOp)) {
                                    instructionHex = assInstr_34(instrInfo.second, 2, 2, pcRelOp);
                                } else if (canUseBase && checkBaseRel(baseRelOp)) {
                                    instructionHex = assInstr_34(instrInfo.second, 2, 4, baseRelOp);
                                } else {
                                    cerr << "Can't use PC rel or Base rel addr to assemble this instr\n";
                                }
                            } else {
                                cerr << "Can't use indirect addr with format " << instrInfo.first << "\n";
                            }
                        } else {
                            cerr << "Symbol: " << it_sym->first << " not found in symtab";
                        }
                        break;
                    }
                    case 3: {
                        string _operand = operand.substr(0, operand.size() - 2);
                        auto it_sym = SYMTAB.find(_operand);
                        if (it_sym != SYMTAB.end()) {
                            if (instrInfo.first == "3/4") {
                                int pcRelOp = calcPCRelOperand(it_op, locctr, opcode, it_sym);
                                int baseRelOp = canUseBase ? calcBaseRelOperand(baseLocHex, it_sym) : -1;
                                if (opcode[0] == '+') {
                                    instructionHex = assInstr_34(instrInfo.second, 3, 9, hexStrToInt(it_sym->second));
                                } else if (checkPCRel(pcRelOp)) {
                                    instructionHex = assInstr_34(instrInfo.second, 3, 10, pcRelOp);
                                } else if (canUseBase && checkBaseRel(baseRelOp)) {
                                    instructionHex = assInstr_34(instrInfo.second, 3, 12, baseRelOp);
                                } else {
                                    cerr << "Can't use PC rel or Base rel addr to assemble this instr\n";
                                }
                                break;
                            } else if (instrInfo.first != "2") {
                                cerr << "Can't use indexed addr with format " << instrInfo.first << "\n";
                                break;
                            }
                        } else {
                            cerr << "Symbol: " << it_sym->first << " not found in symtab";
                            break;
                        }
                    }
                    case 4: {
                        if (instrInfo.first != "2") {
                            cerr << "Unknown symbol: " << operand;
                        } else {
                            string r1, r2;
                            unsigned long pos = operand.find(',');
                            r1 = operand.substr(0, pos);
                            r2 = operand.substr(pos + 1, operand.size() - pos);
                            auto it1 = SYMTAB.find(r1);
                            auto it2 = SYMTAB.find(r2);
                            if (it1 == SYMTAB.end() || it2 == SYMTAB.end()) {
                                cerr << "Invalid register symbol: " << operand << "\n";
                                break;
                            }
                            if (stoi(it1->second) < 0 || stoi(it1->second) > 9 || stoi(it2->second) == 7) {
                                cerr << "Expected a register, found: " << it1->first << "\n";
                                break;
                            }
                            if (stoi(it2->second) < 0 || stoi(it2->second) > 9 || stoi(it2->second) == 7) {
                                cerr << "Expected a register, found: " << it2->first << "\n";
                                break;
                            }
                            instructionHex = assInstr_2(instrInfo.second, it1->second, it2->second);
                        }
                        break;
                    }
                    case 5: {
                        auto it_sym = SYMTAB.find(operand);
                        if (it_sym != SYMTAB.end()) {
                            if (instrInfo.first == "2") {
                                instructionHex = assInstr_2(instrInfo.second, it_sym->second, "0");
                            } else if (instrInfo.first == "3/4") {
                                int pcRelOp = calcPCRelOperand(it_op, locctr, opcode, it_sym);
                                int baseRelOp = canUseBase ? calcBaseRelOperand(baseLocHex, it_sym) : -1;
                                if (opcode[0] == '+') {
                                    instructionHex = assInstr_34(instrInfo.second, 3, 1, hexStrToInt(it_sym->second));
                                } else if (checkPCRel(pcRelOp)) {
                                    instructionHex = assInstr_34(instrInfo.second, 3, 2, pcRelOp);
                                } else if (canUseBase && checkBaseRel(baseRelOp)) {
                                    instructionHex = assInstr_34(instrInfo.second, 3, 4, baseRelOp);
                                }
                            }
                        } else {
                            cerr << "Symbol: " << it_sym->first << " not found in symtab";
                        }
                        break;
                    }
                }
            } else if (opcode == "BYTE" | opcode == "WORD") {
                if (tolower(operand[0]) == 'x') {
                    instructionHex = operand.substr(2, operand.size() - 3);
                } else if (tolower(operand[0]) == 'c') {
                    string chars = operand.substr(2, operand.size() - 3);
                    instructionHex = strToHexStr(chars);
                } else {
                    try {
                        int dec_val = stoi(operand);
                        instructionHex = intToHexStr(dec_val);
                    }
                    catch (const invalid_argument &ia) {
                        cerr << "wrong WORD value " << operand << "\n";
                    }
                }
            }
            if (objectCode.size() + instructionHex.size() > 60 || opcode == "RESW" || opcode == "RESB") {
                writeTextRecord(fout, textRecord, objectCode);
                textRecord += locctr;
                objectCode += instructionHex;
            } else {
                objectCode += instructionHex;
            }
        }
    }
    string retVal = fout.is_open() ? outFile : "Not created";
    fin.close();
    fout.close();
    return retVal;
}

int parse(string operand) {
    if (operand.empty()) {
        return 0;
    } else if (operand[0] == '#') {
        return 1;
    } else if (operand[0] == '@') {
        return 2;
    } else if (operand.find(",X") != string::npos || operand.find(",x") != string::npos) {
        return 3;
    } else if (operand.find(',') != string::npos) {
        return 4;
    } else {
        return 5;
    }
}

template<typename T>
int getNextInstrAdd(T it_op, int locctr, string opcode) {
    pss info = it_op->second;
    if (info.first == "3/4") {
        if (opcode[0] == '+') {
            return locctr + 4;
        } else {
            return locctr + 3;
        }
    } else if (info.first == "2") {
        return locctr + 2;
    } else {
        return locctr + 1;
    }
}

string assInstr_2(string opcodeHex, string r1, string r2) {
    int opcode_dec = hexStrToInt(opcodeHex);
    int a, b;
    try {
        a = (stoi(r1) << 4);
        b = stoi(r2);
    }
    catch (const invalid_argument &ia) {
        cerr << "Not correct register types: " << r1 << " " << r2 << "\n";
    }
    return intToHexStr((opcode_dec << 8) + a + b, 4);
}

string assInstr_34(string opcodeHex, int ni, int xbpe, int actualOperandVal) {
    int opcode_dec = hexStrToInt(opcodeHex);
    opcode_dec += ni;
    int finalInstr = 0;
    int size;
    if ((xbpe & 1) == 0) {
        finalInstr = (opcode_dec << 16) + (xbpe << 12) + actualOperandVal;
        size = 6;
    } else {
        finalInstr = (opcode_dec << 24) + (xbpe << 20) + actualOperandVal;
        size = 8;
    }
    return intToHexStr(finalInstr, size);
}
