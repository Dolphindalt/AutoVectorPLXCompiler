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

void SymbolTable::lookupOrInsertIntConstant(
    const int64_t value,
    st_entry_t *out_entry
) {
    const address name = std::to_string(value);
    if (this->symbolTable.count(name) > 0) {
        *out_entry = this->symbolTable.at(name);
    } else {
        out_entry->entry_type = ST_LITERAL;
        out_entry->literal.type = INT;
        out_entry->literal.value.int_value = value;
        this->insert(name, *out_entry);
    }
}

unsigned int SymbolTable::getTypeSizeBytes(const st_entry_t &entry) const {
    const st_entry_type_t type = entry.entry_type;
    unsigned int sizeBytes = 0;
    switch (type) {
        case ST_VARIABLE:
            if (entry.variable.isArray) {
                sizeBytes = entry.variable.arraySize * VARIABLE_SIZE_BYTES;
            } else {
                sizeBytes = VARIABLE_SIZE_BYTES;
            }
            break;
        case ST_LITERAL:
            // Literals cannot be arrays, yet.
            sizeBytes = VARIABLE_SIZE_BYTES;
            break;
        default:
            ERROR_LOG("invalid symbol table entry for type %d", type);
            exit(EXIT_FAILURE);
            break;
    }
    return sizeBytes;
}

bool SymbolTable::isGlobalScope() const {
    return this->level == 0;
}