/**
 * This file is responsibe for hosting the multipurpose symbol table that is 
 * utilized throughout the compilation process.
 * 
 * @file symbol_table.h
 * @author Dalton Caron
 */
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

/** The type of entry that may be stored in the symbol table. */
typedef enum st_entry_type {
    ST_VARIABLE,
    ST_LITERAL,
    ST_FUNCTION,
    ST_CODE_GEN
} st_entry_type_t;

/** The data types of variables and literals in the language. */
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

/**
 * Liveness denotes if a variable is dead or live.
 * Live variables are used again while dead variables are never used.
 */
typedef enum liveness {
    CG_DEAD,
    CG_LIVE
} liveness_t;

/** 
 * Represents the value of a literal from the source code.
 */
typedef union literal_value {
    int64_t int_value;
    _Float64 float_value;
} literal_value_t;

/**
 * The symbol table entry is a multipurpose data structure for associating 
 * a symbol to compile-time information.
 */
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

/**
 * The SymbolTable is used to associate a variable or literal with information 
 * at compile-time. The SymbolTable is utilized in most phases of the compiler. 
 */
class SymbolTable {
public:
    /** Contructs a SymbolTable in which the table is global in scope. */
    SymbolTable();

    /**
     * Constructs a SymbolTable that is one scope deeper than the enclosing 
     * scope.  
     * @param enclosingScope The scope one layer higher than this table's scope.
     */
    SymbolTable(std::shared_ptr<SymbolTable> enclosingScope);

    virtual ~SymbolTable();

    /**
     * Creates a binding in the symbol table between a variable and a symbol 
     * table entry object.
     * @param name Variable to bind with data.
     * @param object Data to bind with variable.
     */
    void insert(const address &name, const st_entry_t &object);

    /**
     * Finds the symbol table entry for a provided variable. The scope depth 
     * of the variable is also provided.
     * 
     * @param name The name of the variable to lookup.
     * @param out_level An integer to store the scope depth in.
     * @param out_entry A symbol table entry to store the found information in.
     */
    bool lookup(
        const address &name, 
        unsigned int *out_level, 
        st_entry_t *out_entry
    ) const;

    /**
     * Special function that looks up the a value assuming it is constant. If 
     * the constant value is not found, the value is inserted into the table 
     * and the new entry is returned.
     * 
     * @param value Literal value to find or insert.
     * @param out_entry A symbol table entry to store the found information in.
     */
    void lookupOrInsertIntConstant(
        const int64_t value,
        st_entry_t *out_entry
    );

    /**
     * Returns the size of the type in bytes associated with a particular 
     * symbol table entry.
     * @return Size of the type.
     */
    unsigned int getTypeSizeBytes(const st_entry_t &entry) const;

    /**
     * A scope is global if it is at scope level zero. This means it is the 
     * root scope and cannot be a nested scope. 
     * 
     * @return True if scope is global, else false.
     */
    bool isGlobalScope() const;

    /** @return The scope depth level. Higher is deeper and zero is global. */
    unsigned int getLevel() const { return this->level; };
private:
    std::map<address, st_entry_t> symbolTable;
    std::shared_ptr<SymbolTable> enclosingScope = nullptr;
    unsigned int level = 0;
};

#endif