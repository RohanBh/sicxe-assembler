//
// Created by rohan on 1/4/18.
//

#include "PassTwo.h"
#include "Commons.h"
#include "Utils.h"
#include <sstream>
#include <iostream>
#include <iomanip>


using namespace std;

ofstream log("log.txt", ios::ate | ios::app);
int lineNum = 0;
bool errorFlag = false;

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

template<typename T>
string getSymbolAddr(T it_sym);
/* =========================================== */

/* ========== Methods to write the records to the object file ========== */
void writeHeaderRecord(ofstream &fout);

void writeTextRecord(ostream &fout, string &textRecord, string &objectCode);

void writeEndRecord(ofstream &fout, const string &operand);

/* ===================================================================== */


void prepareListingFile(ofstream &lout) {
    lout << "LINE" << "  ";
    lout << setw(4) << left << "Loc" << "  ";
    lout << setw(28) << left << "      Source Statement      ";
    lout << "Object Code\n";

}

void
printListLine(ostream &lout, int lineNum, int locctr, string label, string opcode, string operand, string objectcode) {
    lout << setw(4) << left << lineNum << "  ";
    lout << setw(4) << right << intToHexStr(locctr, 4) << "  ";
    lout << setw(8) << left << label << "  ";
    lout << setw(6) << left << opcode << "  ";
    lout << setw(10) << left << operand << "  ";
    lout << setw(10) << left << objectcode << "\n";
}

void printLogLine(vector<string> &line) {
    errorFlag = true;
    std::string outLine = "";
    for (auto const &s : line) {
        outLine += s + " ";
    }
    log << outLine << "\n";
}

string createObjectFile(std::string intermediateFile) {
    ifstream fin(intermediateFile.c_str());
    if (!fin.is_open()) {
        cerr << "Intermediate File doesn't exits\n" << "Filename: " << intermediateFile << "\n";
        return "";
    }
    string objectFile = intermediateFile.substr(0, intermediateFile.find(".imd")) + ".ob";
    string listingFile = intermediateFile.substr(0, intermediateFile.find(".imd")) + ".txt";

    ofstream fout(objectFile.c_str());
    ofstream lout(listingFile.c_str());
    if (!log.is_open()) {
        log.open("log.txt", ios::ate | ios::app);
    }

    prepareListingFile(lout);

    if (!fout.is_open()) {
        cerr << "Could not create object file\n" << "Filename: " << objectFile << "\n";
        return "";
    }
    vector<string> line;
    string opcode, operand, label, locctr;
    lineNum = 5;
    string baseLocHex;
    string textRecord = "T";
    string objectCode = "";
    string modificationRecord = "";
    bool shouldAddMRecord = startAddr == 0;
    bool canUseBase = false;
    errorFlag = false;
    string currBlock = "DEFAULT";
    while (readLine(fin, line)) {
        // Skip the line if it's empty or contains on the location counter's value
        if (line.empty() || line.size() == 1) {
            continue;
        }
        locctr = intToHexStr(hexStrToInt(BLOCKTAB.find(currBlock)->second.blockAddr) + hexStrToInt(line[0]));
        initVariables(line, label, opcode, operand);
        string instructionHex = "";
        if (opcode == "START") {
            writeHeaderRecord(fout);
        } else if (opcode == "BASE") {
            auto it_sym = SYMTAB.find(operand);
            baseLocHex = getSymbolAddr(it_sym);
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
        } else if (opcode == "USE") {
            writeTextRecord(fout, textRecord, objectCode);
            if (operand.empty()) {
                currBlock = "DEFAULT";
            } else {
                currBlock = operand;
            }
        } else {
            auto it_op = OPTAB.safeFind(opcode);
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
                            log << "Line: " << lineNum << ", No operand specified for " << opcode << " instruction\n";
                        } else if (safe(opcode) == "RSUB") {
                            int xbpe = opcode[0] == '+' ? 1 : 0;
                            instructionHex = assInstr_34(instrInfo.second, 3, xbpe, 0);
                        } else {
                            log << "Line: " << lineNum << ", No operand specified for " << opcode << " instruction\n";
                            printLogLine(line);
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
                                log << "Line: " << lineNum << "Can't use Immediate addr with format " << instrInfo.first
                                    << "\n";
                                printLogLine(line);
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
                                log << "Line: " << lineNum << ", Can't use indirect addr with format "
                                    << instrInfo.first << "\n";
                                printLogLine(line);
                            }
                        } else {
                            log << "Line: " << lineNum << ", Symbol: " << it_sym->first << " not found in symtab";
                            printLogLine(line);
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
                                log << "Line: " << lineNum << ", Can't use indexed addr with format " << instrInfo.first
                                    << "\n";
                                printLogLine(line);
                                break;
                            }
                        } else {
                            log << "Line: " << lineNum << ", Symbol: " << it_sym->first << " not found in symtab";
                            printLogLine(line);
                            break;
                        }
                        // Break not added to allow the passing of format 2 instructions into the next case.
                    }
                        // Two register instructions
                    case 4: {
                        if (instrInfo.first != "2") {
                            log << "Line: " << lineNum << ", Unknown symbol: " << operand;
                            printLogLine(line);
                        } else {
                            string r1, r2;
                            unsigned long pos = operand.find(',');
                            r1 = operand.substr(0, pos);
                            r2 = operand.substr(pos + 1, operand.size() - pos);
                            auto it1 = SYMTAB.find(r1);
                            auto it2 = SYMTAB.find(r2);
                            if (isRegisterValid(it1, operand) && isRegisterValid(it2, operand)) {
                                instructionHex = assInstr_2(instrInfo.second, it1->second.first, it2->second.first);
                            }
                        }
                        break;
                    }
                        // One operand instructions (direct addressing and one register instructions)
                    case 5: {
                        auto it_sym = SYMTAB.find(operand);
                        if (it_sym != SYMTAB.end()) {
                            if (instrInfo.first == "2") {
                                instructionHex = assInstr_2(instrInfo.second, it_sym->second.first, "0");
                            } else if (instrInfo.first == "3/4") {
                                // Handle this case in the end to avoid code duplication
                                shouldHandle = true;
                                sym_it = it_sym;
                                ni = 3;
                            }
                        } else {
                            log << "Line: " << lineNum << ", Symbol: " << it_sym->first << " not found in symtab";
                            printLogLine(line);
                        }
                        break;
                    }
                }
                // Handle the same routine that is followed in each case
                if (shouldHandle) {
                    int pcRelOp = calcPCRelOperand(it_op, locctr, opcode[0], sym_it);
                    int baseRelOp = canUseBase ? calcBaseRelOperand(baseLocHex, sym_it) : -1;
                    if (opcode[0] == '+') {
                        instructionHex = assInstr_34(instrInfo.second, ni, x + 1, hexStrToInt(getSymbolAddr(sym_it)));
                        if (shouldAddMRecord) {
                            int addrFieldStartLoc = hexStrToInt(locctr) + 1;
                            modificationRecord += ("M" + intToHexStr(addrFieldStartLoc) + "05\n");
                        }
                    } else if (checkPCRel(pcRelOp)) {
                        instructionHex = assInstr_34(instrInfo.second, ni, x + 2, pcRelOp);
                    } else if (canUseBase && checkBaseRel(baseRelOp)) {
                        instructionHex = assInstr_34(instrInfo.second, ni, x + 4, baseRelOp);
                    } else {
                        log << "Line: " << lineNum << ", Can't use PC rel or Base rel addr to assemble this instr\n";
                        printLogLine(line);
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
                        log << "Line: " << lineNum << ", wrong WORD value " << operand << "\n";
                        printLogLine(line);
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
        printListLine(lout, lineNum, hexStrToInt(locctr), label, opcode, operand, instructionHex);
        lineNum += 5;
    }
    string retVal = fout.is_open() ? objectFile : "Not created";
    fin.close();
    fout.close();
    log.close();
    if (errorFlag) {
        cerr << "Assembly not successful, check log.txt for more info\n";
    } else {
        cout << "Object program File successfully generated!\n";
    }
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
        log << "Line: " << lineNum << ", Unspecified immediate operand: " << _operand << "\n";
        errorFlag = true;
        return "";
    }
    if (opcode_first_char == '+') {
        if (operand_dec > 1048575) {
            log << "Line: " << lineNum << ", operand too large to fit in format 4: " << operand;
            errorFlag = true;
        } else {
            return assInstr_34(opcodeHex, 1, 1, operand_dec);
        }
    } else if (operand_dec < 4096) {
        return assInstr_34(opcodeHex, 1, 0, operand_dec);
    } else {
        log << "Line: " << lineNum << ", operand too large to fit in format 3: " << operand;
        errorFlag = true;
    }
}

template<typename T>
bool isRegisterValid(T it1, const string &operand) {
    if (it1 == SYMTAB.end()) {
        log << "Line: " << lineNum << ", Invalid register symbol: " << operand << "\n";
        errorFlag = true;
        return false;
    }
    if (stoi(it1->second.first) < 0 || stoi(it1->second.first) > 9 || stoi(it1->second.first) == 7) {
        log << "Line: " << lineNum << ", Expected a register, found: " << it1->first << "\n";
        errorFlag = true;
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
        log << "Line: " << lineNum << ", Not correct register types: " << r1 << " " << r2 << "\n";
        errorFlag = true;
    }
    return intToHexStr((opcode_dec << 8) + a + b, 4);
}

string assInstr_34(string opcodeHex, int ni, int xbpe, int actualOperandVal) {
    int opcode_dec = hexStrToInt(std::move(opcodeHex));
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
    int pcPos = getNextInstrAdd(it_op, hexStrToInt(std::move(locctr)), opcode_first_char);
    int symVal = hexStrToInt(getSymbolAddr(it_sym));
    return symVal - pcPos;
}

template<typename T>
int calcBaseRelOperand(string baseHex, T it_sym) {
    int basePos = hexStrToInt(std::move(baseHex));
    int symVal = hexStrToInt(getSymbolAddr(it_sym));
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
    auto totalBytes = static_cast<int>(objectCode.size() / 2);
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

void writeEndRecord(ofstream &fout, const string &operand) {
    string firstExIntrHex = intToHexStr(startAddr);
    if (!operand.empty()) {
        auto it = SYMTAB.find(operand);
        firstExIntrHex = it->second.first;
    }
    toUpper(firstExIntrHex);
    fout << "E" << firstExIntrHex;
}

template<typename T>
string getSymbolAddr(T it_sym) {
    auto it = BLOCKTAB.find(it_sym->second.second);
    string blockStartAddr = it->second.blockAddr;
    string symbolBlockRelAddr = it_sym->second.first;
    return intToHexStr(hexStrToInt(blockStartAddr) + hexStrToInt(symbolBlockRelAddr));
}
