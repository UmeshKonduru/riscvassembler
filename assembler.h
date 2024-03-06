#include <map>
#include <vector>

using namespace std;

class Assembler {

    private:

    // Class Declarations

    class Command { // Each command is converted to this format after parsing
        public:
        string name;
        vector<string> operands; // Contains operands in the order they appear in the command
        unsigned int pc;
    };

    static map<string, string> registerLookup; // Contains the register number corresponding to the register name
    static map<string, unsigned int> sizeLookup; // Contains the size of each data type

    static void clean(string &line); // Removes comments and trailing spaces from the line
    static vector<string> extractLabels(string &line); // Extracts the labels from the line

    map<string, unsigned int> addressLookup; // Contains the address of each label
    map<string, unsigned int> labelLookup; // Contains the program counter value of each labelLookup

    vector<string> commands; // Contains the commands of the text section
    vector<Command> parsedCommands; // Contains the parsed commands
    vector<pair<unsigned int, unsigned int>> machineCode; // Contains the machine code of the text section

    unsigned int toMachineCode(Command command); // Converts the command to machine code
    void parseCommands(); // Parses the commands to the Command format

    void parseData(unsigned int &address, string line); // Parses a single line of the data segment and adds it to the machine code
    void parseText(); // Parses the text section
    void parse(string path); // Parses the entire file

    public:

    Assembler(string path);
    static unsigned int parseRegister(string reg); // Returns the number corresponding to the register
    unsigned int parseImmediate(string imm); // Returns the number corresponding to the immediate value
};

class Instruction { // Base class for all instruction formats

    public:
    static map<string, Instruction*> commandLookup; // Contains the instruction object corresponding to the command name
    unsigned int opcode;
    virtual unsigned int toMachineCode() = 0;
    virtual void setOperands(vector<string> &operands, unsigned int pc, Assembler &assembler) = 0;
    
    Instruction(unsigned int opcode);
};