// This File Converts the .asm file to .bin file
// converts MIPS instruction into 32 bit binary format
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <bitset>
#include <string>
#include <iomanip>

using namespace std;

// Constants for instruction formats
const int REGISTER_BITS = 5;
const int IMMEDIATE_BITS = 16;
const int ADDRESS_BITS = 26;

// Mapping MIPS registers to binary codes
unordered_map<string, string> mips_registers = {
    {"$zero", "00000"}, {"$at", "00001"}, {"$v0", "00010"}, {"$v1", "00011"},
    {"$a0", "00100"}, {"$a1", "00101"}, {"$a2", "00110"}, {"$a3", "00111"},
    {"$t0", "01000"}, {"$t1", "01001"}, {"$t2", "01010"}, {"$t3", "01011"},
    {"$t4", "01100"}, {"$t5", "01101"}, {"$t6", "01110"}, {"$t7", "01111"},
    {"$s0", "10000"}, {"$s1", "10001"}, {"$s2", "10010"}, {"$s3", "10011"},
    {"$s4", "10100"}, {"$s5", "10101"}, {"$s6", "10110"}, {"$s7", "10111"},
    {"$t8", "11000"}, {"$t9", "11001"}, {"$k0", "11010"}, {"$k1", "11011"},
    {"$gp", "11100"}, {"$sp", "11101"}, {"$fp", "11110"}, {"$ra", "11111"}
};

// MIPS instructions with their types, opcodes, and function codes
unordered_map<string, unordered_map<string, string>> mips_instructions = {
    {"add",    {{"type", "R"}, {"opcode", "000000"}, {"funct", "100000"}}},
    {"sub",    {{"type", "R"}, {"opcode", "000000"}, {"funct", "100010"}}},
    {"and",    {{"type", "R"}, {"opcode", "000000"}, {"funct", "100100"}}},
    {"or",     {{"type", "R"}, {"opcode", "000000"}, {"funct", "100101"}}},
    {"sll",    {{"type", "R"}, {"opcode", "000000"}, {"funct", "000000"}}},
    {"srl",    {{"type", "R"}, {"opcode", "000000"}, {"funct", "000010"}}},
    {"slt",    {{"type", "R"}, {"opcode", "000000"}, {"funct", "101010"}}},
    {"jr",     {{"type", "R"}, {"opcode", "000000"}, {"funct", "001000"}}},
    {"syscall",{{"type", "R"}, {"opcode", "000000"}, {"funct", "001100"}}},
    {"move",   {{"type", "R"}, {"opcode", "000000"}, {"funct", "100000"}}},
    {"addi",   {{"type", "I"}, {"opcode", "001000"}}},
    {"li",     {{"type", "I"}, {"opcode", "001000"}}},
    {"la",     {{"type", "I"}, {"opcode", "001000"}}},
    {"andi",   {{"type", "I"}, {"opcode", "001100"}}},
    {"ori",    {{"type", "I"}, {"opcode", "001101"}}},
    {"lw",     {{"type", "I"}, {"opcode", "100011"}}},
    {"sw",     {{"type", "I"}, {"opcode", "101011"}}},
    {"beq",    {{"type", "I"}, {"opcode", "000100"}}},
    {"bne",    {{"type", "I"}, {"opcode", "000101"}}},
    {"j",      {{"type", "J"}, {"opcode", "000010"}}},
    {"jal",    {{"type", "J"}, {"opcode", "000011"}}}
};

// Register memory storage
unordered_map<string, string> register_memory;

// Label addresses for branching
unordered_map<string, int> labelAddressMap;

// Initialize register memory
void initializeRegisterMemory() {
    for (const auto& reg : mips_registers) {
        register_memory[reg.second] = string(32, '0');
    }
}

// Convert immediate value to binary
string getImmediateBinary(int value) {
    return bitset<IMMEDIATE_BITS>(value).to_string();
}

// Convert address value to binary
string getAddressBinary(int value) {
    return bitset<ADDRESS_BITS>(value).to_string();
}

// Convert R-type instruction to binary
string convertRType(const string &opcode, const string &rs, const string &rt, const string &rd, int shamt = 0) {
    return mips_instructions[opcode]["opcode"]+ mips_registers[rs]+ mips_registers[rt] + mips_registers[rd] + bitset<5>(shamt).to_string() + mips_instructions[opcode]["funct"];
}

// Convert I-type instruction to binary
string convertIType(const string &opcode, const string &rs, const string &rt, int immediate) {
    return mips_instructions[opcode]["opcode"] + mips_registers[rs] + mips_registers[rt] + getImmediateBinary(immediate);
}

// Convert J-type instruction to binary
string convertJType(const string &opcode, int address) {
    return mips_instructions[opcode]["opcode"] + getAddressBinary(address);
}

// Store value in register
void storeInRegister(const string &reg, const string &value) {
    register_memory[mips_registers[reg]] = value;
}
// Memory storage for the `.data` section
unordered_map<string, pair<int, vector<int>>> memoryAllocation;
int dataMemoryAddress = 0x10010000;  // Starting address for data memory
unordered_map<int, string> varToRegMap;
// Parse the `.data` section
void parseDataSection(ifstream &input) {
    int i=4;
    string line;
    while (getline(input, line)) {
        if (line.find(".text") != string::npos) break;

        istringstream iss(line);
        string label, directive, valueStr;
        
        if (iss >> label >> directive) {
            label.pop_back();  // Remove ':' from the label

            if (directive == ".word") {
                int value;
                iss >> value;
                varToRegMap[dataMemoryAddress]="$s"+to_string(i++);
                memoryAllocation[label] = {dataMemoryAddress, {value}};
                dataMemoryAddress += 4;
            } else if (directive == ".float") {
                float value;
                iss >> value;
                int intValue = *reinterpret_cast<int*>(&value);
                memoryAllocation[label] = {dataMemoryAddress, {intValue}};
                dataMemoryAddress += 4;
            } else if (directive == ".asciiz") {
                string str;
                getline(iss, str);
                str = str.substr(1, str.size() - 2);  // Remove quotation marks
                vector<int> charValues;
                for (char c : str) {
                    charValues.push_back(static_cast<int>(c));
                }
                charValues.push_back(0);  // Null terminator
                memoryAllocation[label] = {dataMemoryAddress, charValues};
                dataMemoryAddress += ((charValues.size() + 3) / 4) * 4;  // Align to word boundary
            }
        }
    }

    // Store data addresses in $1 register
    for (const auto& entry : memoryAllocation) {
        storeInRegister("$1", bitset<32>(entry.second.first).to_string());
    }
}

// Parse the `.text` section and convert to binary
void parseTextSection(ifstream &input, ofstream &output) {
    string line;
    int instructionAddress = 0x00400000;  // Starting address for text segment
    bool inText = false;
    // First pass: store labels with their addresses
    while (getline(input, line)) {
        if(!inText && line.find(".text") != string::npos) {
            inText = true;
        }
        if (inText && line.find(":") != string::npos) {
            size_t pos = line.find(":");
            string label = line.substr(0, pos);
            labelAddressMap[label] = instructionAddress;
        } else if (inText && !line.empty() && line[0] != '#' && line[0] != '.') {  // Ignore empty lines and comments
            instructionAddress += 4;  // Each instruction is 4 bytes
        }
    }

    // Reset file to beginning for second pass
    input.clear();
    input.seekg(0, ios::beg);

    // Find .text section
    while (getline(input, line) && line.find(".text") == string::npos);

    // Second pass: parse and translate instructions
    instructionAddress = 0x00400000;
    while (getline(input, line)) {
        istringstream iss(line);
        string opcode, rd, rs, rt, label;
        int immediate, shamt = 0;

        if (line.empty() || line[0] == '#') continue;  // Skip empty lines and comments
        if (line.find(":") != string::npos) continue;  // Skip label-only lines

        iss >> opcode;
        
        if (mips_instructions[opcode]["type"] == "R") {
            if (opcode == "jr") {
                iss >> rs;
                output << convertRType(opcode, rs, "$zero", "$zero") << endl;
            } else if (opcode == "syscall") {
                output << convertRType(opcode, "$zero", "$zero", "$zero") << endl;
            } else if (opcode == "move") {
                iss >> rd >> rs;
                rd=rd.substr(0,rd.size()-1);
                output << convertRType("add", rs, "$zero", rd) << endl;
                storeInRegister(rd, register_memory[mips_registers[rs]]);
            } else if (opcode == "sll" || opcode == "srl") {
                iss >> rd >> rt >> shamt;
                output << convertRType(opcode, "$zero", rt, rd, shamt) << endl;
                int value = stoi(register_memory[mips_registers[rt]], nullptr, 2);
                value = (opcode == "sll") ? (value << shamt) : (value >> shamt);
                storeInRegister(rd, bitset<32>(value).to_string());
            } else {
                iss >> rd >> rs >> rt;
                rs=rs.substr(0,rs.size()-1);
                rd=rd.substr(0,rd.size()-1);
                cout<< opcode << " " << rd << " " << rs<< " " << rt <<endl;
                output << convertRType(opcode, rs, rt, rd) << endl;
                // Perform operation and store result
                int value1 = stoi(register_memory[mips_registers[rs]], nullptr, 2);
                int value2 = stoi(register_memory[mips_registers[rt]], nullptr, 2);
                int result;
                if (opcode == "add") result = value1 + value2;
                else if (opcode == "sub") result = value1 - value2;
                else if (opcode == "and") result = value1 & value2;
                else if (opcode == "or") result = value1 | value2;
                else if (opcode == "slt") result = (value1 < value2) ? 1 : 0;
                storeInRegister(rd, bitset<32>(result).to_string());
            }
        } else if (mips_instructions[opcode]["type"] == "I") {
            if (opcode == "lw") {
                iss >> rt >> label;
                rt=rt.substr(0,rt.size()-1);
                // Assume address of label is in $1
                if (label.find('(') == std::string::npos) {
                    storeInRegister(rt,bitset<32>(memoryAllocation[label].second[0]).to_string());
                    output << convertIType(opcode, varToRegMap[memoryAllocation[label].first], rt, 0) << endl;
                } else if (isdigit(label[0])) {
                    size_t openParen = label.find('(');
                    size_t closeParen = label.find(')');
                    if (openParen != std::string::npos && closeParen != std::string::npos) {
                        string offset = label.substr(0, openParen);
                        string base = label.substr(openParen + 1, closeParen - openParen - 1);
                        output << convertIType(opcode, base, rt, stoi(offset)) << endl;
                    }
                }
                
                // // Load word from memory to register
                // int address = stoi(register_memory[mips_registers["$1"]], nullptr, 2);
                // int value = memoryAllocation[label].second[0];
                // storeInRegister(rt, bitset<32>(value).to_string());
             
            } else if (opcode == "beq" || opcode == "bne") {
                iss >> rs >> rt >> label;
                rs=rs.substr(0,rs.size()-1);
                rt=rt.substr(0,rt.size()-1);
                int labelOffset = (labelAddressMap[label] - instructionAddress + 4) / 4;
                output << convertIType(opcode, rs, rt, labelOffset) << endl;
            } else if (opcode == "li") {
                iss >> rt >> immediate;
                output << convertIType("addi", "$zero", rt, immediate) << endl;
                storeInRegister(rt, bitset<32>(immediate).to_string());
            } else if (opcode == "la") {
                iss >> rt >> label;
                int address = memoryAllocation[label].first;
                output << convertIType("addi", "$zero", rt, address) << endl;
                storeInRegister(rt, bitset<32>(address).to_string());
            } else {
                iss >> rt >> rs >> immediate;
                output << convertIType(opcode, rs, rt, immediate) << endl;
                int value = stoi(register_memory[mips_registers[rs]], nullptr, 2);
                int result;
                if (opcode == "addi") result = value + immediate;
                else if (opcode == "andi") result = value & immediate;
                else if (opcode == "ori") result = value | immediate;
                storeInRegister(rt, bitset<32>(result).to_string());
            }
        } else if (mips_instructions[opcode]["type"] == "J") {
            iss >> label;
            int address = labelAddressMap[label] / 4;
            output << convertJType(opcode, address) << endl;
        }
        
        instructionAddress += 4;
    }
}

int main() {
    ifstream inputFile("input.asm");
    ofstream outputFile("output.bin");

    if (!inputFile.is_open() || !outputFile.is_open()) {
        cerr << "Error opening file!" << endl;
        return 1;
    }

    initializeRegisterMemory();
    parseDataSection(inputFile);

    // Reset file pointer to beginning of file
    inputFile.clear();
    inputFile.seekg(0, ios::beg);

    parseTextSection(inputFile, outputFile);

    inputFile.close();
    outputFile.close();

    cout << "Data Section:" << endl;
    for (const auto &entry : memoryAllocation) {
        cout << entry.first << " (Address: 0x" << hex << entry.second.first << "): ";
        for (const int &value : entry.second.second) {
            cout << dec << value << " ";
        }
        cout << endl;
    }

    cout << "\nText Section Labels:" << endl;
    for (const auto &entry : labelAddressMap) {
        cout << entry.first << ": 0x" << hex << entry.second << endl;
    }

    return 0;
}