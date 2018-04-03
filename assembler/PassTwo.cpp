//
// Created by rohan on 1/4/18.
//

#include "PassTwo.h"
#include "Commons.h"
#include "Utils.h"
#include <sstream>
#include <iostream>


using namespace std;

/* ========== Methods to determine which relative addressing mode is to be used ========== */
bool checkPCRel(int operandVal);

bool checkBaseRel(int operandVal);

template<typename T, typename U>
int calcPCRelOperand(T it_op, string locctr, char opcode_first_char, U it_sym);

template<typename T>
int calcBaseRelOperand(string baseHex, T it_sym);
/* ======================================================================================= */

/* ========== Methods to assemble instructions of formats 2, 3 and 4 ========== */
string assInstr_34(string opcodeHex, int ni, int xbpe, int actualOperandVal);

string assInstr_2(string opcodeHex, string r1, string r2);
/* ============================================================================ */

/* ========== Miscellaneous methods ========== */
int parse(string operand);

template<typename T>
int getNextInstrAdd(T it_op, int locctr, char opcode_first_char);

void initVariables(vector<string> &line, string &label, string &opcode, string &operand);

string handleImmediateIntegerOperand(string opcodeHex, const string &operand, char opcode_first_char);

template<typename T>
bool isRegisterValid(T it1, const string &operand);
/* =========================================== */

/* ========== Methods to write the records to the object file ========== */
void writeHeaderRecord(ofstream &fout);

void writeTextRecord(ostream &fout, string &textRecord, string &objectCode);

void writeEndRecord(ofstream &fout, string operand);

/* ===================================================================== */

string createObjectFile(std::string intermediateFile) {
    ifstream fin(intermediateFile.c_str());
    if (!fin.is_open()) {
        cerr << "Intermediate File doesn't exits\n" << "Filename: " << intermediateFile << "\n";
        return "";
    }
    string objectFile = intermediateFile.substr(0, intermediateFile.find(".imd")) + ".ob";
    ofstream fout(objectFile.c_str());
    if (!fout.is_open()) {
        cerr << "Could not create object file\n" << "Filename: " << objectFile << "\n";
        return "";
    }
    vector<string> line;
    string opcode, operand, label, locctr;
    string baseLocHex;
    string textRecord = "T";
    string objectCode = "";
    string modificationRecord = "";
    bool shouldAddMRecord = startAddr == 0;
    bool canUseBase = false;
    while (readLine(fin, line)) {
        // Skip the line if it's empty or contains on the location counter's value
        if (line.empty() || line.size() == 1) {
            continue;
        }
        locctr = line[0];
        initVariables(line, label, opcode, operand);
        if (opcode == "START") {
            writeHeaderRecord(fout);
        } else if (opcode == "BASE") {
            auto it_sym = SYMTAB.find(operand);
            baseLocHex = it_sym->second;
            canUseBase = true;
        } else if (opcode == "NOBASE") {
            canUseBase = false;
        } else if (opcode == "END") {
            writeTextRecord(fout, textRecord, objectCode);
            if (shouldAddMRecord) {
                fout << modificationRecord;
            }
            writeEndRecord(fout, operand);
            break;
        } else {
            auto it_op = OPTAB.safeFind(opcode);
            string instructionHex = "";
            if (it_op != OPTAB.end()) {
                if (textRecord == "T") {
                    textRecord += locctr;
                }
                pss instrInfo = it_op->second;
                auto sym_it = SYMTAB.begin();
                bool shouldHandle = false;
                int ni = 0, x = 0;
                switch (parse(operand)) {
                    // No operand
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
                        // Immediate addressing
                    case 1: {
                        string _operand = operand.substr(1);
                        auto it_sym = SYMTAB.find(_operand);
                        if (it_sym != SYMTAB.end()) {
                            if (instrInfo.first == "3/4") {
                                // Handle this case in the end to avoid code duplication
                                shouldHandle = true;
                                sym_it = it_sym;
                                ni = 1;
                            } else {
                                cerr << "Can't use Immediate addr with format " << instrInfo.first << "\n";
                            }
                        } else {
                            instructionHex = handleImmediateIntegerOperand(instrInfo.second, operand, opcode[0]);
                        }
                        break;
                    }
                        // Indirect addressing
                    case 2: {
                        string _operand = operand.substr(1);
                        auto it_sym = SYMTAB.find(_operand);
                        if (it_sym != SYMTAB.end()) {
                            if (instrInfo.first == "3/4") {
                                // Handle this case in the end to avoid code duplication
                                shouldHandle = true;
                                sym_it = it_sym;
                                ni = 2;
                            } else {
                                cerr << "Can't use indirect addr with format " << instrInfo.first << "\n";
                            }
                        } else {
                            cerr << "Symbol: " << it_sym->first << " not found in symtab";
                        }
                        break;
                    }
                        // Indexed addressing
                    case 3: {
                        string _operand = operand.substr(0, operand.size() - 2);
                        auto it_sym = SYMTAB.find(_operand);
                        if (it_sym != SYMTAB.end()) {
                            if (instrInfo.first == "3/4") {
                                // Handle this case in the end to avoid code duplication
                                shouldHandle = true;
                                sym_it = it_sym;
                                ni = 3;
                                x = 8;
                                break;
                            } else if (instrInfo.first != "2") {
                                cerr << "Can't use indexed addr with format " << instrInfo.first << "\n";
                                break;
                            }
                        } else {
                            cerr << "Symbol: " << it_sym->first << " not found in symtab";
                            break;
                        }
                        // Break not added to allow the passing of format 2 instructions into the next case.
                    }
                        // Two register instructions
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
                            if (isRegisterValid(it1, operand) && isRegisterValid(it2, operand)) {
                                instructionHex = assInstr_2(instrInfo.second, it1->second, it2->second);
                            }
                        }
                        break;
                    }
                        // One operand instructions (direct addressing and one register instructions)
                    case 5: {
                        auto it_sym = SYMTAB.find(operand);
                        if (it_sym != SYMTAB.end()) {
                            if (instrInfo.first == "2") {
                                instructionHex = assInstr_2(instrInfo.second, it_sym->second, "0");
                            } else if (instrInfo.first == "3/4") {
                                // Handle this case in the end to avoid code duplication
                                shouldHandle = true;
                                sym_it = it_sym;
                                ni = 3;
                            }
                        } else {
                            cerr << "Symbol: " << it_sym->first << " not found in symtab";
                        }
                        break;
                    }
                }
                // Handle the same routine that is followed in each case
                if (shouldHandle) {
                    int pcRelOp = calcPCRelOperand(it_op, locctr, opcode[0], sym_it);
                    int baseRelOp = canUseBase ? calcBaseRelOperand(baseLocHex, sym_it) : -1;
                    if (opcode[0] == '+') {
                        instructionHex = assInstr_34(instrInfo.second, ni, x + 1, hexStrToInt(sym_it->second));
                        if (shouldAddMRecord) {
                            int addrFieldStartLoc = hexStrToInt(locctr) + 1;
                            modificationRecord += ("M" + intToHexStr(addrFieldStartLoc) + "05\n");
                        }
                    } else if (checkPCRel(pcRelOp)) {
                        instructionHex = assInstr_34(instrInfo.second, ni, x + 2, pcRelOp);
                    } else if (canUseBase && checkBaseRel(baseRelOp)) {
                        instructionHex = assInstr_34(instrInfo.second, ni, x + 4, baseRelOp);
                    } else {
                        cerr << "Can't use PC rel or Base rel addr to assemble this instr\n";
                    }
                }
            } else if (opcode == "BYTE" | opcode == "WORD") {
                if (textRecord == "T") {
                    textRecord += locctr;
                }
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
            if (objectCode.size() + instructionHex.size() > 60) {
                writeTextRecord(fout, textRecord, objectCode);
                textRecord += locctr;
                objectCode += instructionHex;
            } else if (opcode == "RESW" || opcode == "RESB") {
                writeTextRecord(fout, textRecord, objectCode);
            } else {
                objectCode += instructionHex;
            }
        }
    }
    string retVal = fout.is_open() ? objectFile : "Not created";
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
int getNextInstrAdd(T it_op, int locctr, char opcode_first_char) {
    pss info = it_op->second;
    if (info.first == "3/4") {
        if (opcode_first_char == '+') {
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

void initVariables(vector<string> &line, string &label, string &opcode, string &operand) {
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
}

string handleImmediateIntegerOperand(string opcodeHex, const string &operand, char opcode_first_char) {
    string _operand = operand.substr(1);
    unsigned int operand_dec;
    try {
        operand_dec = static_cast<unsigned int>(stoi(_operand));
    }
    catch (const invalid_argument &ia) {
        cerr << "Unspecified immediate operand: " << _operand << "\n";
        return "";
    }
    if (opcode_first_char == '+') {
        if (operand_dec > 1048575) {
            cerr << "operand too large to fit in format 4: " << operand;
        } else {
            return assInstr_34(opcodeHex, 1, 1, operand_dec);
        }
    } else if (operand_dec < 4096) {
        return assInstr_34(opcodeHex, 1, 0, operand_dec);
    } else {
        cerr << "operand too large to fit in format 3: " << operand;
    }
}

template<typename T>
bool isRegisterValid(T it1, const string &operand) {
    if (it1 == SYMTAB.end()) {
        cerr << "Invalid register symbol: " << operand << "\n";
        return false;
    }
    if (stoi(it1->second) < 0 || stoi(it1->second) > 9 || stoi(it1->second) == 7) {
        cerr << "Expected a register, found: " << it1->first << "\n";
        return false;
    }
    return true;
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
        finalInstr = (opcode_dec << 16) + (xbpe << 12) + (actualOperandVal & 0xFFF);
        size = 6;
    } else {
        finalInstr = (opcode_dec << 24) + (xbpe << 20) + (actualOperandVal & 0xFFFFF);
        size = 8;
    }
    return intToHexStr(finalInstr, size);
}

bool checkPCRel(int operandVal) {
    return operandVal >= -2048 && operandVal < 2048;
}

bool checkBaseRel(int operandVal) {
    return operandVal >= 0 && operandVal < 4096;
}

template<typename T, typename U>
int calcPCRelOperand(T it_op, string locctr, char opcode_first_char, U it_sym) {
    int pcPos = getNextInstrAdd(it_op, hexStrToInt(locctr), opcode_first_char);
    int symVal = hexStrToInt(it_sym->second);
    return symVal - pcPos;
}

template<typename T>
int calcBaseRelOperand(string baseHex, T it_sym) {
    int basePos = hexStrToInt(baseHex);
    int symVal = hexStrToInt(it_sym->second);
    return symVal - basePos;
}

void writeHeaderRecord(ofstream &fout) {
    string headername = programName;
    if (headername.size() < 6) {
        while (headername.size() != 6) {
            headername += " ";
        }
    } else if (headername.size() > 6) {
        headername = headername.substr(0, 6);
    }
    string startAddrHex = intToHexStr(startAddr);
    string programLengthHex = intToHexStr(programLength);
    toUpper(startAddrHex);
    toUpper(programLengthHex);
    fout << "H" << headername << startAddrHex << programLengthHex << "\n";
}

void writeTextRecord(ostream &fout, string &textRecord, string &objectCode) {
    int totalBytes = static_cast<int>(objectCode.size() / 2);
    if (totalBytes == 0) {
        textRecord = "T";
        objectCode = "";
        return;
    }
    textRecord += intToHexStr(totalBytes, 2);
    textRecord += objectCode;
    toUpper(textRecord);
    fout << textRecord << "\n";
    textRecord = "T";
    objectCode = "";
}

void writeEndRecord(ofstream &fout, string operand) {
    string firstExIntrHex = intToHexStr(startAddr);
    if (!operand.empty()) {
        auto it = SYMTAB.find(operand);
        firstExIntrHex = it->second;
    }
    toUpper(firstExIntrHex);
    fout << "E" << firstExIntrHex;
}
