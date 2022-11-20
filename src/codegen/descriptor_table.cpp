#include <codegen/descriptor_table.h>

RegisterTable::RegisterTable() {}

void RegisterTable::updateRegisterValue(
    const reg_t reg, 
    const std::string &newValue
) {
    this->valueMap.insert(std::make_pair(reg, newValue));
    this->regMap.insert(std::make_pair(newValue, reg));
}

reg_t RegisterTable::getUnusedRegister() const {
    for (unsigned int i = RBX; i < RAX; i++) {
        if (!this->doesRegisterContainValue((reg_t)i)) {
            return (reg_t) i;
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

bool RegisterTable::doesRegisterContainValue(const reg_t reg) const {
    return this->valueMap.count(reg) > 0;
}

std::string RegisterTable::getRegisterValue(const reg_t reg) const {
    return this->valueMap.at(reg);
}

AddressTable::AddressTable() : stackPointer(0) {}

void AddressTable::insertVariableIntoStack(
    const std::string &name,
    const unsigned int size_bytes
) {
    this->stackPointer += size_bytes;
    this->variableStackOffset.insert(std::make_pair(name, size_bytes));
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

void DataSection::insert(const std::string &name) {
    this->dataKeys.insert(name);
}

bool DataSection::isVariableInDataSection(const std::string &name) const {
    return this->dataKeys.count(name) > 0;
}