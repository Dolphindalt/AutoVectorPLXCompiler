#ifndef SYMBOL_TABLE_H__
#define SYMBOL_TABLE_H__

#define MAXIMUM_ARGUMENTS 64
#define LITERAL_BYTE_LEN 8

#include <lexer.h>
#include <string>
#include <memory>
#include <map>

typedef std::string address;

typedef enum st_entry_type {
    ST_VARIABLE,
    ST_LITERAL,
    ST_FUNCTION
} st_entry_type_t;

typedef enum type { 
    UNKNOWN = 0,
    INT, 
    FLOAT,
    VOID,
    NO_TYPE
} type_t;

static std::map<type_t, std::string> typeToStringMap = {
    {UNKNOWN, "UNKNOWN"},
    {INT, "INT"},
    {FLOAT, "FLOAT"},
    {VOID, "VOID"},
    {NO_TYPE, "NO_TYPE"}
};

inline std::string typeToString(const type_t t) { 
    return typeToStringMap.at(t); 
};

typedef struct symbol_table_entry {
    st_entry_type_t entry_type;
    token_t token;

    union {
        bool isConstant;
        bool isAssigned;
        bool isArray;
        uint64_t arraySize;
        type_t type;
    } variable;

    union {
        char value[LITERAL_BYTE_LEN];
        type_t type;
    } literal;

    union {
        uint8_t argumentsLength;
        type_t argumentTypes[MAXIMUM_ARGUMENTS];
        type_t returnType;
    } procedure;
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