#include <codegen2/registers.h>

#include <logging.h>

Register::Register(const std::string name) : name(name) {}

std::string Register::getName() const {
    return "\%" + this->name;
}

std::string Register::getNameAsMemory() const {
    return "(\%" + this->name + ")";
}

std::set<RegPtr> &Registers::selectRegisters(const register_type_t &type) {
    switch (type) {
        case GPR:
            return Registers::generalPurposeRegisters;
        case AVX:
            return Registers::vectorRegisters;
        default:
            break;
    }
    ERROR_LOGV("Invalid registers set selected");
    exit(EXIT_FAILURE);
}

std::set<RegPtr> Registers::generalPurposeRegisters = {
    std::make_shared<Register>("r15"),
    std::make_shared<Register>("r14"),
    std::make_shared<Register>("r13"),
    std::make_shared<Register>("r12"),
    std::make_shared<Register>("r11"),
    std::make_shared<Register>("r10"),
    std::make_shared<Register>("r9"),
    std::make_shared<Register>("r8"),
    std::make_shared<Register>("rdi"),
    std::make_shared<Register>("rsi"),
    std::make_shared<Register>("rdx"),
    std::make_shared<Register>("rcx"),
    std::make_shared<Register>("rax")
};

std::set<RegPtr> Registers::vectorRegisters = {
    std::make_shared<Register>("ymm15"),
    std::make_shared<Register>("ymm14"),
    std::make_shared<Register>("ymm13"),
    std::make_shared<Register>("ymm12"),
    std::make_shared<Register>("ymm11"),
    std::make_shared<Register>("ymm10"),
    std::make_shared<Register>("ymm9"),
    std::make_shared<Register>("ymm8"),
    std::make_shared<Register>("ymm7"),
    std::make_shared<Register>("ymm6"),
    std::make_shared<Register>("ymm5"),
    std::make_shared<Register>("ymm4"),
    std::make_shared<Register>("ymm3"),
    std::make_shared<Register>("ymm2"),
    std::make_shared<Register>("ymm1"),
    std::make_shared<Register>("ymm0")
};

RegisterAllocationTable::RegisterAllocationTable() {}

void RegisterAllocationTable::setRegisterValue(
    RegPtr reg, 
    const std::string &value
) {
    this->registerTable[reg] = value;
}

bool RegisterAllocationTable::atLeastOneRegisterUnused(
    const register_type_t &type
) const {
    return Registers::selectRegisters(type).size() 
        != this->registerTable.size();
}

RegPtr RegisterAllocationTable::getUnusedRegister(
    const register_type_t &type
) const {
    for (auto reg : Registers::selectRegisters(type)) {
        if (this->registerTable.count(reg) == 0) {
            return reg;
        }
    }
    return nullptr;
}

RegPtr RegisterAllocationTable::getARegisterInUse(
    const register_type_t &type
) const {
    for (auto reg : Registers::selectRegisters(type)) {
        if (this->registerTable.count(reg) != 0) {
            return reg;
        }
    }
    return nullptr;
}

std::string RegisterAllocationTable::getVariableInRegister(RegPtr reg) const {
    return this->registerTable.at(reg);
}

void RegisterAllocationTable::freeRegister(RegPtr reg) {
    this->registerTable.erase(reg);
}

void RegisterAllocationTable::clear() {
    this->registerTable.clear();
}

std::set<RegPtr> RegisterAllocationTable::getAllRegistersInUse() {
    std::set<RegPtr> usedRegs;
    for (const auto &p : this->registerTable) {
        usedRegs.insert(p.first);
    }
    return usedRegs;
}

std::string RegisterAllocationTable::to_string() const {
    std::string result = "";
    for (auto p : this->registerTable) {
        result += "(" + p.first->getName() + ", " + p.second + ") "; 
    }
    return result;
}