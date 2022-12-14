/**
 * The address table is responsible for keep track of where variables are 
 * located, be it in memory or in registers.
 * 
 * @file address_table.h
 * @author Dalton Caron
 */
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

/**
 * Represents the location of a value in memory or in a register.
 */
class Location {
public:
    /** A prefix that identifies large immediate values in memory. */
    static std::string largeImmediatesPrefix;

    /**
     * Constructs a large immediate name from the name of an existing immediate 
     * value
     * @param previousName The name/value of an existing immediate.
     * @return The large immediate value name.
     */
    static std::string getLargeImmediateName(const std::string &previousName);

    /** Default constructor for stack initialization. */
    Location();

    /** 
     * Constructs a location to contain data of the provided type.
     * @param location_type_t The type of location where the data is stored. 
     */
    Location(const location_type_t &type);

    /** @return Gets the type of location where the data is stored. */
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

    /**
     * Looks up the location of a variable and finds where it is.
     * @param variable Variable to look up.
     * @return A Location object denoting the location of the variable.
     */
    Location &getLocation(const std::string &variable);

    bool isInRegister(const std::string &variable) const;

    RegPtr getRegister(const std::string &variable);

    /**
     * Associates a variable with the location provided.
     * 
     * @param variable The variable to associate with.
     * @param location The location to associate with.
     */
    void insert(const std::string &variable, const Location &location);

    /**
     * Associates a variable with an immediate value if it is a literal.
     * 
     * @param variable Variable to attempt to associate with.
     * @param table The symbol table used to determine if the variable is a
     * constant immediate value.
     */
    void insertIfLiteral(
        const std::string &variable, 
        const std::shared_ptr<SymbolTable> &table
    );

    /**
     * Checks if a variable has a recorded location.
     * @param variable Varible to check.
     * @return True if the variable has a location, else false.
    */
    bool contains(const std::string &variable) const;

    /**
     * Fetches variables and locations of variables that are located within a
     * register.
     * 
     * @return Pairs of variables and locations that are in use in a register.
     */
    std::vector<std::pair<std::string, Location>> 
    getValueAndLocationInRegisters();

    /**
     * Clears all variables and locations that are associated with a register.
     */
    void clearRegisters();

    /** @return A string representation of the address table. */
    std::string to_string() const;
private:
    std::map<std::string, Location> table;
};

#endif