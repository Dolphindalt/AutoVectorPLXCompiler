#ifndef REGISTER_TABLE_H
#define REGISTER_TABLE_H

#define NO_REGISTER "no register"

#include <map>
#include <set>
#include <string>

using reg_t = std::string;

typedef enum register_type {
    NORMAL,
    AVX
} register_type_t;

static std::set<reg_t> registers = {
    "rcx",
    "rdx",
    "rsi",
    "rdi",
    "r8",
    "r9",
    "r10",
    "r11",
    "r12",
    "r13",
    "r14",
    "r15",
    "rax"
};

static std::set<reg_t> vectorRegisters = {
    "ymm0",
    "ymm1",
    "ymm2",
    "ymm3",
    "ymm4",
    "ymm5",
    "ymm6",
    "ymm7",
    "ymm8",
    "ymm9",
    "ymm10",
    "ymm11",
    "ymm12",
    "ymm13",
    "ymm14",
    "ymm15"
};

class RegisterTable {
public:
    RegisterTable();

    void updateRegisterValue(const reg_t &reg, const std::string &newValue);

    reg_t getUsedRegister(const register_type_t type) const;
    reg_t getUnusedRegister(const register_type_t type) const;
    reg_t getRegisterWithValue(const std::string &value) const;
    bool isValueInRegister(const std::string &value) const;
    bool doesRegisterContainValue(const reg_t &reg) const;
    std::string getRegisterValue(const reg_t &reg) const;
private:
    std::set<reg_t> selectRegisters(const register_type_t type) const;

    std::map<reg_t, std::string> valueMap;
    std::map<std::string, reg_t> regMap;
};

// For this simple language, all variables are 8 bytes, excluding arrays.
class AddressTable {
public:
    AddressTable();

    unsigned int insertVariableIntoStack(
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

#endif