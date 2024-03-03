#include<iostream>
#include<sstream>
#include<vector>
#include<map>
#include<fstream>

using namespace std;

// Utility Functions

unsigned int clip(unsigned int x, int left, int right) {
    return (x >> right) & ((1 << (left - right + 1)) - 1);
}

void trim(string &s) {
    s.erase(0, s.find_first_not_of(" \t\n"));
    s.erase(s.find_last_not_of(" \t\n") + 1);
}

// Template for converting commands of different formats to machine code

class Instruction { // Parent class for all instructions

    public:
        static map<string, Instruction*> commandLookup; // See below
        unsigned int opcode;
        virtual unsigned int toMachineCode() = 0; // Convert command to machine code
        virtual void setOperands(vector<unsigned int> operands) = 0; // Set operands of the command passed as a vector containing registers first and then immediate values
};

class R : public Instruction { // R-Format Instructions

    unsigned int rd, funct3, rs1, rs2, funct7;

    public:

        R(unsigned int opcode, unsigned int funct3, unsigned int funct7) {
            this->opcode = opcode;
            this->funct3 = funct3;
            this->funct7 = funct7;
        }

        unsigned int toMachineCode() {
            return (funct7 << 25) | (rs2 << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | opcode;
        }

        void setOperands(vector<unsigned int> operands) {
            rd = operands[0];
            rs1 = operands[1];
            rs2 = operands[2];
        }
};

class I : public Instruction { // I-Format Instructions

    unsigned int rd, funct3, rs1, imm;

    public:
        
        I(unsigned int opcode, unsigned int funct3) {
            this->opcode = opcode;
            this->funct3 = funct3;
            imm = 0;
        }

        I(unsigned int opcode, unsigned int funct3, unsigned int funct7) {
            this->opcode = opcode;
            this->funct3 = funct3;
            this->imm = funct7 << 5;
        }

        unsigned int toMachineCode() {
            return (imm << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | opcode;
        }
        
        void setOperands(vector<unsigned int> operands) {
            rd = operands[0];
            rs1 = operands[1];
            imm = operands[2];
        }
};

class S : public Instruction { // S-Format Instructions

    unsigned int funct3, rs1, rs2, imm;

    public:

        S(unsigned int opcode, unsigned int funct3) {
            this->opcode = opcode;
            this->funct3 = funct3;
        }

        unsigned int toMachineCode() {
            unsigned int imm1 = clip(imm, 4, 0);
            unsigned int imm2 = clip(imm, 11, 5);
            return (imm2 << 25) | (rs2 << 20) | (rs1 << 15) | (funct3 << 12) | (imm1 << 7) | opcode;
        }

        void setOperands(vector<unsigned int> operands) {
            rs2 = operands[0];
            rs1 = operands[1];
            imm = operands[2];
        }
};

class SB : public Instruction { // SB-Format Instructions

    unsigned int funct3, rs1, rs2, imm;

    public:

        SB(unsigned int opcode, unsigned int funct3) {
            this->opcode = opcode;
            this->funct3 = funct3;
        }

        unsigned int toMachineCode() {
            unsigned int imm1 = clip(imm, 3, 0) << 1 | clip(imm, 10, 10);
            unsigned int imm2 = clip(imm, 11, 11) << 6 | clip(imm, 9, 4);
            return (imm2 << 25) | (rs2 << 20) | (rs1 << 15) | (funct3 << 12) | (imm1 << 7) | opcode;
        }

        void setOperands(vector<unsigned int> operands) {
            rs1 = operands[0];
            rs2 = operands[1];
            imm = operands[2];
        }
};

class U : public Instruction { // U-Format Instructions

    unsigned int rd, imm;

    public:

        U(unsigned int opcode) {
            this->opcode = opcode;
        }

        unsigned int toMachineCode() {
            return (imm << 12) | (rd << 7) | opcode;
        }

        void setOperands(vector<unsigned int> operands) {
            rd = operands[0];
            imm = operands[1];
        }
};

class UJ : public Instruction { // UJ-Format Instructions

    unsigned int rd, imm;

    public:

        UJ(unsigned int opcode) {
            this->opcode = opcode;
        }

        unsigned int toMachineCode() {
            unsigned int imm1 = clip(imm, 19, 19) << 19 | clip(imm, 9, 0) << 9 | clip(imm, 10, 10) << 8 | clip(imm, 18, 11);
            return (imm1 << 12) | (rd << 7) | opcode;
        }

        void setOperands(vector<unsigned int> operands) {
            rd = operands[0];
            imm = operands[1];
        }
};

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

    {"jal",    new UJ(0x6f)}
};

class Assembler { // For parsing and converting assembly code to machine code

    class Command { // Each command is converted to this format after parsing
        public:
            string name;
            vector<unsigned int> operands;
    };

    unsigned int parseRegister(string reg) { // Convert register name to register number
        reg.erase(0, 1);
        return stoi(reg);
    }

    unsigned int toMachineCode(Command command) { // Convert command to machine code
        Instruction* instruction = Instruction::commandLookup[command.name];
        instruction->setOperands(command.operands);
        return instruction->toMachineCode();
    }

    public:

        static map<string, string> registerLookup;
};

map<string, string> Assembler::registerLookup = {

    {"zero", "x0"},
    {"ra", "x1"},
    {"sp", "x2"},
    {"gp", "x3"},
    {"tp", "x4"},
    {"t0", "x5"},
    {"t1", "x6"},
    {"t2", "x7"},
    {"s0", "x8"},
    {"s1", "x9"},
    {"a0", "x10"},
    {"a1", "x11"},
    {"a2", "x12"},
    {"a3", "x13"},
    {"a4", "x14"},
    {"a5", "x15"},
    {"a6", "x16"},
    {"a7", "x17"},
    {"s2", "x18"},
    {"s3", "x19"},
    {"s4", "x20"},
    {"s5", "x21"},
    {"s6", "x22"},
    {"s7", "x23"},
    {"s8", "x24"},
    {"s9", "x25"},
    {"s10", "x26"},
    {"s11", "x27"},
    {"t3", "x28"},
    {"t4", "x29"},
    {"t5", "x30"},
    {"t6", "x31"},
};

int main() { 

    return 0;
}
