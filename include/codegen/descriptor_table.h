#ifndef REGISTER_TABLE_H
#define REGISTER_TABLE_H

#include <map>
#include <set>
#include <string>

typedef enum regenum {
    NO_REGISTER = 0, // If no register is available.
    RCX,
    RDX,
    RSI,
    RDI,
    R8, // Ye new registers.
    R9,
    R10,
    R11,
    RBX, // Start of perserved registers.
    R12,
    R13,
    R14,
    R15, // End of perserved registers.
    RAX // RAX is last because it is used for return values and results.
} reg_t;

class RegisterTable {
public:
    RegisterTable();

    void updateRegisterValue(const reg_t reg, const std::string &newValue);

    reg_t getUnusedRegister() const;
    reg_t getRegisterWithValue(const std::string &value) const;
    bool isValueInRegister(const std::string &value) const;
    bool doesRegisterContainValue(const reg_t reg) const;
    std::string getRegisterValue(const reg_t reg) const;
private:
    std::map<reg_t, std::string> valueMap;
    std::map<std::string, reg_t> regMap;
};

// For this simple language, all variables are 8 bytes, excluding arrays.
class AddressTable {
public:
    AddressTable();

    void insertVariableIntoStack(
        const std::string &name, 
        const unsigned int size_bytes
    );
    unsigned int findVariableInStack(const std::string &name) const;
    bool isVaribleInStack(const std::string &name) const;
    const unsigned int getStackSize() const;
private:
    unsigned int stackPointer = 0;
    std::map<std::string, unsigned int> variableStackOffset;
};

class DataSection {
public:
    DataSection();

    void insert(const std::string &name);
    bool isVariableInDataSection(const std::string &name) const;
private:
    std::set<std::string> dataKeys;
};

#endif