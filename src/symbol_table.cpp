#include <symbol_table.h>

#include <assertions.h>

SymbolTable::SymbolTable() : enclosingScope(nullptr), level(0) {
    
}

SymbolTable::SymbolTable(std::shared_ptr<SymbolTable> enclosingScope) 
: enclosingScope(enclosingScope), level(this->enclosingScope->level + 1) {
    
}

SymbolTable::~SymbolTable() {
    this->enclosingScope = nullptr;
}

void SymbolTable::insert(
    const address &name, 
    const st_entry_t &object
) {
    this->symbolTable.insert(std::make_pair(name, object));
}

bool SymbolTable::lookup(
    const address &name,
    unsigned int *out_level,
    st_entry_t *out_entry
) const {
    if (this->symbolTable.count(name) > 0) {
        *out_entry = this->symbolTable.at(name);
        *out_level = this->level;
        return true;
    }

    if (this->enclosingScope != nullptr) {
        return this->enclosingScope->lookup(name, out_level, out_entry);
    }

    return false;
}

bool SymbolTable::isGlobalScope() const {
    return this->level == 0;
}