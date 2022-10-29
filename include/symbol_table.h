#ifndef SYMBOL_TABLE_H__
#define SYMBOL_TABLE_H__

#include <lexer.h>
#include <string>
#include <memory>
#include <map>

typedef std::string address;

typedef enum status { ALIVE, DEAD } status_t;

#define NEVER_USED (-1)

typedef struct symbol_table_entry {
    // Parser information.
    token_t token;
    bool isConstant = false;
    bool isAssigned = false;
    // Code generator information.
    status_t liveness;
    int lastUse = NEVER_USED;
} st_entry_t;

class SymbolTable {
public:
    SymbolTable();
    SymbolTable(std::shared_ptr<SymbolTable> enclosingScope);

    virtual ~SymbolTable();

    void insert(const address &name, const st_entry_t &object);

    bool lookup(
        const address &name, 
        unsigned int *out_level, 
        st_entry_t *out_entry
    ) const;

    bool isGlobalScope() const;

    unsigned int getLevel() const { return this->level; };
private:
    std::map<address, st_entry_t> symbolTable;
    std::shared_ptr<SymbolTable> enclosingScope = nullptr;
    unsigned int level = 0;
};

#endif