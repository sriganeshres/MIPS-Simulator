// This File Converts the .asm file to .bin file
// converts MIPS instruction into 32 bit binary format
#include <bits/stdc++.h>

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

unordered_map<string, string> reverse_mips_registers = {
    {"00000", "$zero"}, {"00001", "$at"}, {"00010", "$v0"}, {"00011", "$v1"},
    {"00100", "$a0"}, {"00101", "$a1"}, {"00110", "$a2"}, {"00111", "$a3"},
    {"01000", "$t0"}, {"01001", "$t1"}, {"01010", "$t2"}, {"01011", "$t3"},
    {"01100", "$t4"}, {"01101", "$t5"}, {"01110", "$t6"}, {"01111", "$t7"},
    {"10000", "$s0"}, {"10001", "$s1"}, {"10010", "$s2"}, {"10011", "$s3"},
    {"10100", "$s4"}, {"10101", "$s5"}, {"10110", "$s6"}, {"10111", "$s7"},
    {"11000", "$t8"}, {"11001", "$t9"}, {"11010", "$k0"}, {"11011", "$k1"},
    {"11100", "$gp"}, {"11101", "$sp"}, {"11110", "$fp"}, {"11111", "$ra"}
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

bool foundMain = false;

// Register memory storage
unordered_map<string, string> register_memory;

// Label addresses for branching
unordered_map<string, int> labelAddressMap;

// Instruction Mapping
unordered_map<int, string> instructionMemory;

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

unordered_map<string, pair<int, vector<int>>> memoryAllocation = {};
vector<int> atRegister;
// Store value in register
void storeInRegister(const string &reg, const string &value) {
    register_memory[mips_registers[reg]] = value;
}
// Memory storage for the `.data` section
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
                varToRegMap[dataMemoryAddress]="$at";
                memoryAllocation[label] = {dataMemoryAddress, {value}};
                atRegister.push_back(value);
                dataMemoryAddress += 4;
            }
        }
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
            if(label == "main") {
                foundMain = true;
            }
        } else if (inText && !line.empty() && line[0] != '#' && line[0] != '.' && line[0] != '\n') {  // Ignore empty lines and comments
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
            } else if (opcode == "sll" || opcode == "srl") {
                iss >> rd >> rt >> shamt;
                output << convertRType(opcode, "$zero", rt, rd, shamt) << endl;
            } else {
                iss >> rd >> rs >> rt;
                rs=rs.substr(0,rs.size()-1);
                rd=rd.substr(0,rd.size()-1);
                output << convertRType(opcode, rs, rt, rd) << endl;
                // Perform operation and store result
            }
        } else if (mips_instructions[opcode]["type"] == "I") {
            if (opcode == "lw") {
                iss >> rt >> label;
                rt=rt.substr(0,rt.size()-1);
                // Assume address of label is in $1
                output << convertIType(opcode, varToRegMap[memoryAllocation[label].first], rt, memoryAllocation[label].first - 0x10010000) << endl; 
            } else if (opcode == "beq" || opcode == "bne") {
                iss >> rs >> rt >> label;
                rs=rs.substr(0,rs.size()-1);
                rt=rt.substr(0,rt.size()-1);
                int labelOffset = (labelAddressMap[label] - instructionAddress) / 4;
                if (foundMain) labelOffset += 1;
                output << convertIType(opcode, rs, rt, labelOffset) << endl;
            } else if (opcode == "li") {
                iss >> rt >> immediate;
                rt=rt.substr(0,rt.size()-1);
                output << convertIType("addi", "$zero", rt, immediate) << endl;
            } else {
                iss >> rt >> rs >> immediate;
                rs=rs.substr(0,rs.size()-1);
                rt=rt.substr(0,rt.size()-1);
                output << convertIType(opcode, rs, rt, immediate) << endl;
            }
        } else if (mips_instructions[opcode]["type"] == "J") {
            iss >> label;
            int address = labelAddressMap[label];
            output << convertJType(opcode, address) << endl;
        }
        
        instructionAddress += 4;
    }
}

void executeRType(const string &instruction, int& PC) {
    string rs = instruction.substr(6, 5);
    string rt = instruction.substr(11, 5);
    string rd = instruction.substr(16, 5);
    string shamt = instruction.substr(21, 5);
    string funct = instruction.substr(26, 6);

    int value1 = stoi(register_memory[rs], nullptr, 2);
    int value2 = stoi(register_memory[rt], nullptr, 2);
    int result = 0;
    if (funct == "100000") {  // add
        result = value1 + value2;
    } else if (funct == "100010") {  // sub
        result = value1 - value2;
    } else if (funct == "100100") {  // and
        result = value1 & value2;
    } else if (funct == "100101") {  // or
        result = value1 | value2;
    } else if (funct == "101010") {  // slt
        result = (value1 < value2) ? 1 : 0;
    } else if (funct == "000000" || funct == "000010") {  // sll/srl
        int shamtVal = stoi(shamt, nullptr, 2);
        result = (funct == "000000") ? (value2 << shamtVal) : (value2 >> shamtVal);
    }

    storeInRegister(reverse_mips_registers[rd], bitset<32>(result).to_string());
    PC += 4;  // Always increment PC for R-type instructions
}

void executeIType(const string &instruction, int& PC) {
    string opcode = instruction.substr(0, 6);
    string rs = instruction.substr(6, 5);
    string rt = instruction.substr(11, 5);
    int immediate = stoi(instruction.substr(16, 16), nullptr, 2);
    int value = stoi(register_memory[rs], nullptr, 2);
    int result = 0;

    if (opcode == "001000") {  // addi
        result = value + immediate;
        storeInRegister(reverse_mips_registers[rt], bitset<32>(result).to_string());
        PC += 4;
    } else if (opcode == "001100") {  // andi
        result = value & immediate;
        storeInRegister(reverse_mips_registers[rt], bitset<32>(result).to_string());
        PC += 4;
    } else if (opcode == "001101") {  // ori
        result = value | immediate;
        storeInRegister(reverse_mips_registers[rt], bitset<32>(result).to_string());
        PC += 4;
    } else if (opcode == "000101") {  // bne
        int val = stoi(register_memory[rt], nullptr, 2);
        if (value != val) {
            PC += immediate * 4;
        } else {
            PC += 4;
        }
    } else if (opcode == "000100") {  // beq
        int val = stoi(register_memory[rt], nullptr, 2);
        if (value == val) {
            PC += immediate * 4;
        } else {
            PC += 4;
        }
    } else if (opcode == "100011") { // lw
        result = atRegister[immediate / 4];
        storeInRegister(reverse_mips_registers[rt], bitset<32>(result).to_string());
        PC += 4;
    }
}

void executeJType(const string &instruction, int& PC) {
    string opcode = instruction.substr(0, 6);
    int address = stoi(instruction.substr(6, 26), nullptr, 2);
    if (opcode == "000010") {  // j
        PC = address;
    } else if (opcode == "000011") {  // jal
        storeInRegister("$ra", bitset<32>(PC + 4).to_string());  // Save return address
        PC = address;
    }
}

void processInstruction(const string& line, int& PC) {
    if (line.length() != 32) {
        cerr << "Invalid line" << endl;
        return;
    }
    string subStr = line.substr(0, 6);
    storeInRegister("$at", bitset<32>(0x10010000).to_string());
    if (subStr == "000000") {
        executeRType(line, PC);
    } else if (subStr == "000010" || subStr == "000011") {
        executeJType(line, PC);
    } else {
        executeIType(line, PC);
    }
}

void Processor(const string& binaryFile) {
    ifstream binaryReadFile(binaryFile);

    if (!binaryReadFile.is_open()) {
        cerr << "Error opening file!" << endl;
        return;
    }

    string line;
    int PC = 0x00400000;
    while (getline(binaryReadFile, line)) {
        if (line.length() != 32) {
            cerr << "Invalid instruction length" << endl;
            continue;
        }
        instructionMemory[PC] = line;
        PC += 4;
    }
    PC = 0x00400000;
    binaryReadFile.close();

    while(instructionMemory.find(PC) != instructionMemory.end()) {
        string currentInstruction = instructionMemory[PC];
        processInstruction(currentInstruction, PC);
    }

    vector<pair<string, string>> registerVals;
    for (const auto& ele : register_memory) {
        registerVals.push_back({ele.first, ele.second});
    }
    sort(registerVals.begin(), registerVals.end(), [&](const pair<string, string> &a, const pair<string, string> &b) {
        return a.first < b.first;
    });
    for(const auto& ele: registerVals) {
        cout << ele.first << " " << ele.second << endl;
    }
}

int main(int argc, char** argv) {
    if(argc < 2) {
        cerr << "Usage: ./Parser <input_file>.asm";
        return 1;
    }
    ifstream inputFile(argv[1]);
    ofstream outputFile("output.bin");

    if (!inputFile.is_open() || !outputFile.is_open()) {
        cerr << "Error opening file!" << endl;
        return 1;
    }

    initializeRegisterMemory();
    parseDataSection(inputFile);
    cout << "Output is in output.bin file" << endl;

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

    string wantToExecute;
    cout << "Want to execute\nTo execute type yes\n";
    cin >> wantToExecute;
    if(wantToExecute == "yes") {
        Processor("output.bin");
    }

    return 0;
}