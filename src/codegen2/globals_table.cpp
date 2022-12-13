#include <codegen2/globals_table.h>

GlobalAttributes::GlobalAttributes() {}

GlobalAttributes::GlobalAttributes(const unsigned int sizeBytes) 
: sizeBytes(sizeBytes) {}

GlobalTable::GlobalTable() {}

void GlobalTable::insertGlobalVariable(
    const std::string &name,
    const unsigned int size
) {
    this->table[name] = GlobalAttributes(size);
}

void GlobalTable::insertGlobalArray(
    const std::string &name, 
    const unsigned int size
) {
    this->table[name] = GlobalAttributes(size);
}

bool GlobalTable::isGlobal(const std::string &name) const {
    return this->table.count(name) > 0;
}
