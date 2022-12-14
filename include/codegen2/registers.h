/**
 * This file contains classes and functions related to x86_64 registers, 
 * including the register class itself and the register allocation table.
 * 
 * @file registers.h
 * @author Dalton Caron
 */
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

// Denotes the type of registers being operated upon.
// The distinction is required as many registers are instruction set extensions.
typedef enum register_type {
    GPR,    // General purpose registers (ax, eax, rax).
    AVX     // AVX registers (xmm, ymm).
} register_type_t;

/**
 * Represents a Register in the x86_64 architecture.
 */
class Register {
public:
    Register() = delete;
    Register(const std::string name);

    std::string getName() const;
    std::string getLowerName() const;
    std::string getNameAsMemory() const;

    bool operator<(const Register &rhs) const {
        return this->getName() < rhs.getName();
    }

    bool operator==(const Register &rhs) const {
        return this->getName() == rhs.getName();
    }
private:
    std::string name = "";
};

class RegisterAllocationTable;

/**
 * Registers is a static class that provides access to the various x86_64 
 * registers and collections of said registers.
 */
class Registers {
public:
    Registers() = delete;
private:
    static std::set<RegPtr> &selectRegisters(const register_type_t &type);

    static std::set<RegPtr> generalPurposeRegisters;
    static std::set<RegPtr> vectorRegisters;

    friend class RegisterAllocationTable;
};

/**
 * The RegisterAllocationTable is responsible for tracking what variables and 
 * values are located within what registers as well as what registers are in 
 * use or are currently unused.
 */
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