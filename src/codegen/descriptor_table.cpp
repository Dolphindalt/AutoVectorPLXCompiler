#include <codegen/descriptor_table.h>

#include <logging.h>

RegisterTable::RegisterTable() {
    for (auto i : registers) {
        this->addressMap.insert(std::make_pair(i, false));
    }
    for (auto i : vectorRegisters) {
        this->addressMap.insert(std::make_pair(i, false));
    }
}

void RegisterTable::updateRegisterValue(
    const reg_t &reg, 
    const std::string &newValue
) {
    this->valueMap.insert(std::make_pair(reg, newValue));
    this->regMap.insert(std::make_pair(newValue, reg));
}

reg_t RegisterTable::getUsedRegister(const register_type_t type) const {
    switch (type) {
        case NORMAL:
            return "r8";
        case AVX:
            return "ymm15";
        default:
            break;
    }

    ERROR_LOG("invalid register type");
    exit(EXIT_FAILURE);
}

reg_t RegisterTable::getUnusedRegister(const register_type_t type) const {
    std::set<reg_t> registers = this->selectRegisters(type);
    for (const reg_t &reg : registers) {
        if (!this->doesRegisterContainValue(reg)) {
            return reg;
        }
    }
    return NO_REGISTER;
}

reg_t RegisterTable::getRegisterWithValue(const std::string &value) const {
    return this->regMap.at(value);
}

bool RegisterTable::isValueInRegister(const std::string &value) const {
    return this->regMap.count(value) > 0;
}

bool RegisterTable::doesRegisterContainValue(const reg_t &reg) const {
    return this->valueMap.count(reg) > 0;
}

std::string RegisterTable::getRegisterValue(const reg_t &reg) const {
    return this->valueMap.at(reg);
}

void RegisterTable::updateRegisterContentType(const reg_t reg, bool isAddr) {
    this->addressMap[reg] = isAddr;
}

bool RegisterTable::isRegisterHoldingAddress(const reg_t reg) const {
    return this->addressMap.at(reg);
}

bool RegisterTable::isRegisterUnused(const reg_t &reg) const {
    return this->valueMap.count(reg) == 0;
}

std::set<reg_t> RegisterTable::selectRegisters(
    const register_type_t type
) const {
    switch (type) {
        case NORMAL:
            return registers;
        case AVX:
            return vectorRegisters;
        default:
            break;
    }
    ERROR_LOG("invalid register type");
    exit(EXIT_FAILURE);
}

AddressTable::AddressTable() : stackPointer(0) {}

unsigned int AddressTable::insertVariableIntoStack(
    const std::string &name,
    const unsigned int size_bytes
) {
    const unsigned int offset = this->stackPointer;
    this->variableStackOffset.insert(std::make_pair(name, offset));
    this->stackPointer += size_bytes;
    INFO_LOG("Inserting variable %s into the stack at %d", name.c_str(), offset);
    return offset;
}

unsigned int AddressTable::findVariableInStack(const std::string &name) const {
    return this->variableStackOffset.at(name);
}

bool AddressTable::isVaribleInStack(const std::string &name) const {
    return this->variableStackOffset.count(name) > 0;
}

const unsigned int AddressTable::getStackSize() const {
    return this->stackPointer;
}

DataSection::DataSection() {}

void DataSection::insert(const std::string &name, unsigned int sizeBytes) {
    this->dataKeys.insert(std::make_pair(name, sizeBytes));
}

bool DataSection::isVariableInDataSection(const std::string &name) const {
    return this->dataKeys.count(name) > 0;
}

const std::map<std::string, unsigned int> &DataSection::getDataObjects() const {
    return this->dataKeys;
}
