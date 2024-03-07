#include <iostream>
#include <algorithm>
#include <sstream>
#include <vector>
#include <map>
#include <fstream>
#include <string>

#include "assembler.h"

using namespace std;

// Utility Functions

unsigned int clip(unsigned int x, int left, int right = 0) {
    return (x >> right) & ((1 << (left - right + 1)) - 1);
}

void trim(string &s) {
    s.erase(0, s.find_first_not_of(" \t\n"));
    s.erase(s.find_last_not_of(" \t\n") + 1);
}

// Assembler Functions

Assembler::Assembler(string path) {
    parse(path);
}

unsigned int Assembler::parseRegister(string reg) {
    if(reg[0] != 'x') reg = registerLookup[reg];
    return stoi(reg.substr(1));
}

unsigned int Assembler::parseImmediate(string imm, unsigned int pc) {
    if(addressLookup.find(imm) != addressLookup.end()) return addressLookup[imm];
    else if(labelLookup.find(imm) != labelLookup.end()) return labelLookup[imm] - pc;
    else return stoi(imm);
}

void Assembler::clean(string &line) {
    line = line.substr(0, line.find('#'));
    replace(line.begin(), line.end(), ',', ' ');
    replace(line.begin(), line.end(), '(' , ' ');
    replace(line.begin(), line.end(), ')' , ' ');
    trim(line);
}

vector<string> Assembler::extractLabels(string &line) {
    vector<string> labels;
    if(line.find(':') == string::npos) return labels;
    string label = line.substr(0, line.find(':'));
    line = line.substr(line.find(':') + 1);
    trim(label);
    labels = extractLabels(line);
    labels.push_back(label);
    return labels;
}

unsigned int Assembler::toMachineCode(Command command) {
    Instruction *instruction = Instruction::commandLookup[command.name];
    instruction->setOperands(command.operands, command.pc, this);
    return instruction->toMachineCode();
}

void Assembler::parseText(unsigned int &pc, string line) {

    stringstream ss(line);
    string word;
    Command command;

    ss >> word;
    command.name = word;
    command.pc = pc;

    while(ss >> word) command.operands.push_back(word);

    machineCode.push_back({pc, toMachineCode(command)});
    pc += 4;
}

void Assembler::parseData(unsigned int &address, string line) {
    
    stringstream ss(line);
    string word;
    unsigned int value;
    unsigned int size;

    ss >> word;

    if(word == ".asciiz") {
        ss >> word;
        word = word.substr(1, word.size() - 2);
        reverse(word.begin(), word.end());
        for(char c : word) machineCode.push_back({address++, c});
    } else {
        size = sizeLookup[word];
        while(ss >> word) {
            value = stoi(word);
            for(int i = 0; i < size; i++) {
                machineCode.push_back({address++, value & 0xff});
                value >>= 8;
            }
        }
    }
}

void Assembler::parse(string path) {

    ifstream file(path);
    string line, word;
    vector<string> labels;
    unsigned int pc = 0;
    unsigned int address = 0x10000000;
    bool data = false;
    
    while(getline(file, line)) {

        clean(line);
        labels = extractLabels(line);
        for(string label : labels) data ? addressLookup[label] = address : labelLookup[label] = pc;
        if(line.empty()) continue;

        word = line.substr(0, line.find(' '));

        if(word == ".data") data = true;
        else if(word == ".text") data = false;

        else {
            if(data) parseData(address, line);
            else parseText(pc, line);
        }
    }
}

int main() { 

    return 0;
}

// Instruction Classes

Instruction::Instruction(unsigned int opcode) : opcode(opcode) {}

class R : public Instruction { // R-type instruction format

    public:
    unsigned int rd, rs1, rs2, funct3, funct7;

    unsigned int toMachineCode() {
        return (funct7 << 25) | (rs2 << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | opcode;
    }

    void setOperands(vector<string> &operands, unsigned int pc, Assembler *assembler) {
        rd = assembler->parseRegister(operands[0]);
        rs1 = assembler->parseRegister(operands[1]);
        rs2 = assembler->parseRegister(operands[2]);
    }

    R(unsigned int opcode, unsigned int funct3, unsigned int funct7) : Instruction(opcode), funct3(funct3), funct7(funct7) {}
};

class I : public Instruction { // I-type instruction format

    public:
    unsigned int rd, rs1, imm, funct3;

    unsigned int toMachineCode() {
        return (imm << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | opcode;
    }

    void setOperands(vector<string> &operands, unsigned int pc, Assembler *assembler) {

    }

    I(unsigned int opcode, unsigned int funct3) : Instruction(opcode), funct3(funct3), imm(0) {}
    I(unsigned int opcode, unsigned int funct3, unsigned int funct7) : Instruction(opcode), funct3(funct3), imm(funct7 << 5) {}
};

class S : public Instruction { // S-type instruction format

    public:
    unsigned int rs1, rs2, imm, funct3;

    unsigned int toMachineCode() {
        unsigned int imm1 = clip(imm, 4, 0);
        unsigned int imm2 = clip(imm, 11, 5);
        return (imm2 << 25) | (rs2 << 20) | (rs1 << 15) | (funct3 << 12) | (imm1 << 7) | opcode;
    }

    void setOperands(vector<string> &operands, unsigned int pc, Assembler *assembler) {

    }

    S(unsigned int opcode, unsigned int funct3) : Instruction(opcode), funct3(funct3) {}
};

class SB : public Instruction { // SB-type instruction format

    public:
    unsigned int rs1, rs2, imm, funct3;

    unsigned int toMachineCode() {
        unsigned int imm1 = clip(imm, 3, 0) << 1 | clip(imm, 10, 10);
        unsigned int imm2 = clip(imm, 11, 11) << 6 | clip(imm, 9, 4);
        return (imm2 << 25) | (rs2 << 20) | (rs1 << 15) | (funct3 << 12) | (imm1 << 7) | opcode;
    }

    void setOperands(vector<string> &operands, unsigned int pc, Assembler *assembler) {

    }

    SB(unsigned int opcode, unsigned int funct3) : Instruction(opcode), funct3(funct3) {}
};

class U : public Instruction { // U-type instruction format

    public:
    unsigned int rd, imm;

    unsigned int toMachineCode() {
        return (imm << 12) | (rd << 7) | opcode;
    }

    void setOperands(vector<string> &operands, unsigned int pc, Assembler *assembler) {}

    U(unsigned int opcode) : Instruction(opcode) {}
};

class UJ : public Instruction { // UJ-type instruction format

    public:
    unsigned int rd, imm;

    unsigned int toMachineCode() {
        unsigned int imm1 = clip(imm, 19, 19) << 19 | clip(imm, 9, 0) << 9 | clip(imm, 10, 10) << 8 | clip(imm, 18, 11);
        return (imm1 << 12) | (rd << 7) | opcode;
    }

    void setOperands(vector<string> &operands, unsigned int pc, Assembler *assembler) {
        rd = assembler->parseRegister(operands[0]);
        imm = assembler->parseImmediate(operands[1], pc);
    }

    UJ(unsigned int opcode) : Instruction(opcode) {}
};

// Lookup Tables

map<string, Instruction*> Instruction::commandLookup = { // Returns the instruction object corresponding to the command name

    {"add",    new R(0x33, 0x0, 0x00)},
    {"and",    new R(0x33, 0x7, 0x00)},
    {"or",     new R(0x33, 0x6, 0x00)},
    {"sll",    new R(0x33, 0x1, 0x00)},
    {"slt",    new R(0x33, 0x2, 0x00)},
    {"sra",    new R(0x33, 0x5, 0x20)},
    {"srl",    new R(0x33, 0x5, 0x00)},
    {"sub",    new R(0x33, 0x0, 0x20)},
    {"xor",    new R(0x33, 0x4, 0x00)},
    {"mul",    new R(0x33, 0x0, 0x01)},
    {"div",    new R(0x33, 0x4, 0x01)},
    {"rem",    new R(0x33, 0x6, 0x01)},

    {"addi",   new I(0x13, 0x0)},
    {"andi",   new I(0x13, 0x7)},
    {"ori",    new I(0x13, 0x6)},
    {"lb",     new I(0x03, 0x0)},
    {"ld",     new I(0x03, 0x3)},
    {"lh",     new I(0x03, 0x1)},
    {"lw",     new I(0x03, 0x2)},
    {"jalr",   new I(0x67, 0x0)},

    {"sb",     new S(0x23, 0x0)},
    {"sw",     new S(0x23, 0x2)},
    {"sd",     new S(0x23, 0x3)},
    {"sh",     new S(0x23, 0x1)},

    {"beq",    new SB(0x63, 0x0)},
    {"bne",    new SB(0x63, 0x1)},
    {"bge",    new SB(0x63, 0x5)},
    {"blt",    new SB(0x63, 0x4)},

    {"auipc",  new U(0x17)},
    {"lui",    new U(0x37)},

    {"jal",    new UJ(0x6f)},
};

map<string, string> Assembler::registerLookup = {

    {"zero", "x0"},
    {"ra",   "x1"},
    {"sp",   "x2"},
    {"gp",   "x3"},
    {"tp",   "x4"},
    {"t0",   "x5"},
    {"t1",   "x6"},
    {"t2",   "x7"},
    {"s0",   "x8"},
    {"s1",   "x9"},
    {"a0",   "x10"},
    {"a1",   "x11"},
    {"a2",   "x12"},
    {"a3",   "x13"},
    {"a4",   "x14"},
    {"a5",   "x15"},
    {"a6",   "x16"},
    {"a7",   "x17"},
    {"s2",   "x18"},
    {"s3",   "x19"},
    {"s4",   "x20"},
    {"s5",   "x21"},
    {"s6",   "x22"},
    {"s7",   "x23"},
    {"s8",   "x24"},
    {"s9",   "x25"},
    {"s10",  "x26"},
    {"s11",  "x27"},
    {"t3",   "x28"},
    {"t4",   "x29"},
    {"t5",   "x30"},
    {"t6",   "x31"},
};

map<string, unsigned int> Assembler::sizeLookup = {

    {".byte",  1},
    {".half",  2},
    {".word",  4},
    {".dword", 8},
};


