/**
 * The global table is responsible for tracking what is in global memory.
 * 
 * @file globals_table.h
 * @author Dalton Caron
 */
#ifndef GLOBALS_TABLE_H__
#define GLOBALS_TABLE_H__

#include <map>
#include <string>

/**
 * GlobalAttributes is a class that contains all information related to a 
 * particular global variable.
 */
class GlobalAttributes {
public:
    /** Default contructor. */
    GlobalAttributes();

    /**
     * Constructs a GlobalAttributes that denotes the size of the global.
     * @param sizeBytes Size of the global in bytes.
     */
    GlobalAttributes(const unsigned int sizeBytes);
private:
    unsigned int sizeBytes;
};

/**
 * GlobalTable stores additional information associated with global variables.
 */
class GlobalTable {
public:
    /** Default constructor. */
    GlobalTable();

    /**
     * Inserts a global variable of the specified name and size.
     * @param name Name of the global variable to insert.
     * @param size Size in bytes of the global variable to insert.
     */
    void insertGlobalVariable(const std::string &name, const unsigned int size);

    /**
     * Inserts a global array of the specified name and size.
     * @param name Name of the global array to insert.
     * @param size Size in bytes of the global array to insert.
     */
    void insertGlobalArray(const std::string &name, const unsigned int size);

    /**
     * @param name Variable to check if global.
     * @return True if variable is global else false.
     */
    bool isGlobal(const std::string &name) const;
private:
    std::map<std::string, GlobalAttributes> table;
};

#endif