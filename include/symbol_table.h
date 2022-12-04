#ifndef SYMBOL_TABLE_H__
#define SYMBOL_TABLE_H__

#define MAX_ID_LEN 32
#define MAXIMUM_ARGUMENTS 16
#define VARIABLE_SIZE_BYTES 8

#include <lexer.h>
#include <string>
#include <memory>
#include <map>

#define NO_NEXT_USE (-1)

typedef std::string address;

typedef enum st_entry_type {
    ST_VARIABLE,
    ST_LITERAL,
    ST_FUNCTION,
    ST_CODE_GEN
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
    if (typeToStringMap.count(t) > 0) {
        return typeToStringMap.at(t);
    }
    return "INVALID TYPE";
};

typedef enum liveness {
    CG_DEAD,
    CG_LIVE
} liveness_t;

typedef union literal_value {
    int64_t int_value;
    _Float64 float_value;
} literal_value_t;

typedef struct symbol_table_entry {
    st_entry_type_t entry_type;
    token_t token;

    union {

        struct {
            bool isConstant;
            bool isAssigned;
            bool isArray;
            uint64_t arraySize;
            type_t type;
            literal_value_t value;
        } variable;

        struct {
            literal_value_t value;
            type_t type;
        } literal;

        struct {
            uint8_t argumentsLength;
            type_t argumentTypes[MAXIMUM_ARGUMENTS];
            type_t returnType;
            char argumentNames[MAXIMUM_ARGUMENTS][MAX_ID_LEN];
            char returnTypeName[MAX_ID_LEN];
        } procedure;

        struct {
            liveness_t liveness;
            // -1 implies no next use.
            // Any other number is the line used at.
            int next_use;
        } code_gen;

    };
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

    void lookupOrInsertIntConstant(
        const int64_t value,
        st_entry_t *out_entry
    );

    unsigned int getTypeSizeBytes(const st_entry_t &entry) const;

    bool isGlobalScope() const;

    unsigned int getLevel() const { return this->level; };
private:
    std::map<address, st_entry_t> symbolTable;
    std::shared_ptr<SymbolTable> enclosingScope = nullptr;
    unsigned int level = 0;
};

#endif