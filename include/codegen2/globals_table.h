#ifndef GLOBALS_TABLE_H__
#define GLOBALS_TABLE_H__

#include <map>
#include <string>

class GlobalAttributes {
public:
    GlobalAttributes();
    GlobalAttributes(const unsigned int sizeBytes);
private:
    unsigned int sizeBytes;
};

class GlobalTable {
public:
    GlobalTable();

    void insertGlobalVariable(const std::string &name, const unsigned int size);
    void insertGlobalArray(const std::string &name, const unsigned int size);
    bool isGlobal(const std::string &name) const;
private:
    std::map<std::string, GlobalAttributes> table;
};

#endif