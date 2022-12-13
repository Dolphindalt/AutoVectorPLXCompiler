#ifndef ADDRESS_TABLE_H
#define ADDRESS_TABLE_H

#include <map>
#include <string>
#include <vector>
#include <codegen2/registers.h>

typedef enum location_type {
    LT_DUMMY,
    LT_REGISTER,
    LT_MEMORY_GLOBAL,
    LT_MEMORY_STACK,
    LT_IMMEDIATE
} location_type_t;

class Location {
public:
    static std::string largeImmediatesPrefix;

    static std::string getLargeImmediateName(const std::string &previousName);

    Location();
    Location(const location_type_t &type);

    const location_type_t &getType() const;

    bool inMemory() const;
    bool inRegister() const;
    bool isImmediate() const;

    Location &setStack(const signed int offset);
    Location &setReg(const RegPtr reg);

    signed int getStackOffset() const;
    RegPtr getRegister() const;

    const std::string &getImmValueOrGlobal() const;
    Location setImmValueOrGlobal(const std::string &value);

    bool isRegAddress() const;
    Location setIsRegAddress(const bool value);

    std::string address(const bool forceRegValue=false) const;

    std::string to_string() const;
private:
    location_type_t type;

    signed int stackOffset = 0;
    RegPtr reg;
    std::string immValueOrGlobal;
    bool regIsAddress = false;

};

/**
 * The address table keeps track of where the most recent versions of variables 
 * are. Variables are either in memory or are in a register. If variables are 
 * in memory, they are either in global memory or in stack memory.
 */
class AddressTable {
public:
    AddressTable();

    Location &getLocation(const std::string &variable);

    Location &getLocationOrConstant(
        const std::string &variable,
        const std::shared_ptr<SymbolTable> &table
    );

    bool varIsInRegOrMem(const std::string &variable) const;

    bool isInRegister(const std::string &variable) const;

    RegPtr getRegister(const std::string &variable);

    void insert(const std::string &variable, const Location &location);

    void insertIfLiteral(
        const std::string &variable, 
        const std::shared_ptr<SymbolTable> &table
    );

    bool contains(const std::string &variable) const;

    std::vector<std::pair<std::string, Location>> 
    getValueAndLocationInRegisters();

    void clearRegisters();

    std::string to_string() const;
private:
    std::map<std::string, Location> table;
};

#endif