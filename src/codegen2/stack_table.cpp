#include <codegen2/stack_table.h>

#include <assertions.h>

StackTable::StackTable() : baseAddress(0), stackSize(0) {}

StackAddr StackTable::allocate(const std::string &variable, unsigned int size) {
    StackAddr ret = this->stackSize;
    this->varsInStack.insert(std::make_pair(variable, ret));
    this->stackSize += size;
    return ret;
}

void StackTable::resetToPreviousBaseAddress() {
    ASSERT(!this->prevBaseAddresses.empty());
    this->baseAddress = this->prevBaseAddresses.top();
    this->stackSize = this->baseAddress;
    this->prevBaseAddresses.pop();
    this->clearVarsInStackToBaseAddress();
}

void StackTable::newBaseAddress() {
    this->baseAddress = this->stackSize;
    this->prevBaseAddresses.push(this->baseAddress);
}

bool StackTable::inGlobalScope() const {
    return this->baseAddress == 0;
}

bool StackTable::inStack(const std::string &variable) const {
    return this->varsInStack.count(variable) > 0;
}

StackAddr StackTable::getAddress(const std::string &variable) const {
    return this->varsInStack.at(variable);
}

void StackTable::clearVarsInStackToBaseAddress() {
    for (
        auto p = this->varsInStack.begin(); p != this->varsInStack.end(); p++
    ) {
        if (p->second > this->baseAddress) {
            this->varsInStack.erase(p);
        }
    }
}
