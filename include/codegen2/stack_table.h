#ifndef STACK_TABLE_H__
#define STACK_TABLE_H__

#include <stack>
#include <map>
#include <string>

using StackAddr = unsigned int;

class StackTable {
public:
    StackTable();

    StackAddr allocate(const std::string &variable, unsigned int size);

    void resetToPreviousBaseAddress();

    void newBaseAddress();

    bool inGlobalScope() const;

    bool inStack(const std::string &variable) const;

    StackAddr getAddress(const std::string &variable) const;
private:
    void clearVarsInStackToBaseAddress();

    StackAddr baseAddress;
    unsigned int stackSize;
    std::stack<StackAddr> prevBaseAddresses;
    std::map<std::string, unsigned int> varsInStack;
};

#endif