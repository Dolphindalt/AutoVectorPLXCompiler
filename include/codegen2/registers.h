#ifndef REGISTERS_H__
#define REGISTERS_H__

#include <string>
#include <set>
#include <map>
#include <vector>
#include <codegen2/liveness.h>
#include <memory>

class Register;

using RegPtr = std::shared_ptr<Register>;

typedef enum register_type {
    GPR,    // General purpose registers (ax, eax, rax).
    AVX     // AVX registers (xmm, ymm).
} register_type_t;

class Register {
public:
    Register() = delete;
    Register(const std::string name);

    std::string getName() const;
    std::string getNameAsMemory() const;

    bool operator<(const Register &rhs) const {
        return this->getName() < rhs.getName();
    }

    bool operator==(const Register &rhs) const {
        return this->getName() == rhs.getName();
    }
private:
    std::string name;
};

class RegisterAllocationTable;

class Registers {
public:
    Registers() = delete;
private:
    static std::set<RegPtr> &selectRegisters(const register_type_t &type);

    static std::set<RegPtr> generalPurposeRegisters;
    static std::set<RegPtr> vectorRegisters;

    friend class RegisterAllocationTable;
};

class RegisterAllocationTable {
public:
    RegisterAllocationTable();

    void setRegisterValue(RegPtr reg, const std::string &value);

    bool atLeastOneRegisterUnused(const register_type_t &type) const;

    RegPtr getUnusedRegister(const register_type_t &type) const;

    RegPtr getARegisterInUse(const register_type_t &type) const;

    std::string getVariableInRegister(RegPtr reg) const;

    void freeRegister(RegPtr reg);

    void clear();

    std::set<RegPtr> getAllRegistersInUse();

    std::string to_string() const;
private:
    std::map<RegPtr, std::string> registerTable;
};

#endif