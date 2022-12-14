/**
 * The stack table file contains the StackTable class and is responsible for 
 * tracking what is on the stack at all times.
 * 
 * @file stack_table.h
 * @author Dalton Caron
 */
#ifndef STACK_TABLE_H__
#define STACK_TABLE_H__

#include <stack>
#include <map>
#include <string>

using StackAddr = unsigned int;

/**
 * The stack table simulates the program stack that is a feature of the 
 * x86_64 runtime. 
 */
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