/**
 * Representation of three address code for use in the compiler optimization. 
 * Three address code is a simple intermediate representation that represents 
 * all operations with an operation type, a result, and two operands. 
 * 
 * @file 3ac.h
 * @author Dalton Caron
 */
#ifndef THREEAC_H__
#define THREEAC_H__

#include <string>
#include <vector>
#include <map>
#include <symbol_table.h>
#include <memory>

// The types of operations a three address code may represent.
typedef enum tac_op {
    // Nullary instructions.
    // No operation instruction.
    TAC_NOP = 0,
    // Function prologue and epilogue markers.
    TAC_ENTER_PROC,
    TAC_EXIT_PROC,
    // Unary operations.
    TAC_NEGATE,
    TAC_UNCOND_JMP,
    // Special case inlined functions.
    TAC_READ,
    TAC_WRITE,
    // Label.
    TAC_LABEL,
    // Call procedure by label name.
    TAC_CALL,
    // Jumps to labels.
    TAC_JMP_E,
    TAC_JMP_L,
    TAC_JMP_G,
    TAC_JMP_LE,
    TAC_JMP_GE,
    TAC_JMP_NE,
    TAC_JMP_ZERO,
    // Return a single value.
    TAC_RETVAL,
    // Procedure parameter.
    TAC_PROC_PARAM,
    // Binary operations.
    TAC_ASSIGN,
    TAC_ADD,
    TAC_SUB,
    TAC_DIV,
    TAC_MULT,
    TAC_LESS_THAN,
    TAC_GREATER_THAN,
    TAC_GE_THAN,
    TAC_LE_THAN,
    TAC_EQUALS,
    TAC_NOT_EQUALS,
    TAC_ARRAY_INDEX,
    // Vector instructions.
    TAC_VADD,
    TAC_VSUB,
    TAC_VASSIGN,
    TAC_VLOAD,
    TAC_VSTORE
} tac_op_t;

// A map for converting operation types into a string.
static std::map<tac_op_t, std::string> tacOpToStringMap = {
    {TAC_NOP, "TAC_NO_OP"},
    {TAC_ENTER_PROC, "TAC_ENTER_PROC"},
    {TAC_EXIT_PROC, "TAC_EXIT_PROC"},
    {TAC_NEGATE, "TAC_NEGATE"},
    {TAC_UNCOND_JMP, "TAC_UNCONDITIONAL_JUMP"},
    {TAC_READ, "TAC_READ_INPUT"},
    {TAC_WRITE, "TAC_WRITE_OUTPUT"},
    {TAC_LABEL, "TAC_LABEL"},
    {TAC_CALL, "TAC_CALL"},
    {TAC_JMP_E, "TAC_JUMP_EQUALS"},
    {TAC_JMP_L, "TAC_JUMP_LESS_THAN"},
    {TAC_JMP_G, "TAC_JUMP_GREATER_THAN"},
    {TAC_JMP_LE, "TAC_JUMP_LESS_THAN_EQUAL_TO"},
    {TAC_JMP_GE, "TAC_JUMP_GREATER_THAN_EQUAL_TO"},
    {TAC_JMP_NE, "TAC_JUMP_NOT_EQUALS"},
    {TAC_JMP_ZERO, "TAC_JUMP_ZERO"},
    {TAC_RETVAL, "TAC_RETURN_VALUE"},
    {TAC_PROC_PARAM, "TAC_PROCEDURE_PARAMETER"},
    {TAC_ASSIGN, "TAC_ASSIGNMENT"},
    {TAC_ADD, "TAC_ADD"},
    {TAC_SUB, "TAC_SUBTRACT"},
    {TAC_MULT, "TAC_MULTIPLY"},
    {TAC_DIV, "TAC_DIVIDE"},
    {TAC_LESS_THAN, "TAC_LESS_THAN"},
    {TAC_GREATER_THAN, "TAC_GREATER_THAN"},
    {TAC_GE_THAN, "TAC_GREATER_THAN_OR_EQUALS"},
    {TAC_LE_THAN, "TAC_LESS_THAN_OR_EQUALS"},
    {TAC_EQUALS, "TAC_EQUALS"},
    {TAC_NOT_EQUALS, "TAC_NOT_EQUALS"},
    {TAC_ARRAY_INDEX, "TAC_ARRAY_INDEX"},
    {TAC_VADD, "TAC_VADD"},
    {TAC_VSUB, "TAC_VSUB"},
    {TAC_VASSIGN, "TAC_VASSIGN"},
    {TAC_VLOAD, "TAC_VLOAD"},
    {TAC_VSTORE, "TAC_VSTORE"}
};

/** Three address code ID. */
typedef unsigned int TID;

/** Represents a single three address code instruction. */
typedef struct tac_line {
public:
    static unsigned int bid_gen;

    /**
     * @param line The current instruction.
     * @return True if control is transfered, else false.
    */
    static bool transfers_control(const tac_line &line);

    /**
     * @param line The current instruction.
     * @return True if the operation is a comparison, else false.
     */
    static bool is_comparision(const tac_line &line);

    /**
     * @param line The current instruction.
     * @return True if the operation has a result, else false.
     */
    static bool has_result(const tac_line &line);

    /**
     * @param line The current instruction.
     * @return True if the operation is a conditional jump, else false.
     */
    static bool is_conditional_jump(const tac_line &line);

    /**
     * @param line The current instruction.
     * @return True if the operation is a procedure call, else false.
     */
    static bool is_procedure_call(const tac_line &line);

    /**
     * @param line The current instruction.
     * @return True if the operation is binary, else false.
     */
    static bool is_binary_operation(const tac_line &line);

    /**
     * @param line The current instruction.
     * @return True if the operation is a label, else false.
     */
    static bool is_label(const std::string &label);

    /**
     * Three address code labels are marked with a prefix to identify them 
     * during optimization. This function returns a label with the prefix 
     * removed.
     * @param label A label from a three address code label.
     * @return The extracted label.
     */
    static std::string extract_label(const std::string &label);

    /**
     * Determines if a variable is defined by the user.
     * @param var Varible to check.
     * @return True if the variable is user defined, else false.
     */
    static bool is_user_defined_var(const std::string &var);

    /**
     * Read and write are the language supported IO operations. 
     * @param line The current instruction.
     * @return True if operation is read/write, else false.
     */
    static bool is_read_or_write(const tac_line &line);

    /**
     * Uses the current instructions symbol table to determine if the provided 
     * variable is a constant within the scope of this instruction.
     * @param value The variable to check for constantness.
     * @return True if the value is constant, else false.
     */
    bool is_operand_constant(const std::string &value) const;

    /**
     * Simple three address code expressions have at least one operand and are 
     * simple expressions, not labels or function operations.
     * @return True if simple, else false.
     */
    bool is_simple() const;

    /**
     * Generates and sets a new unique ID for the three address code. Used when 
     * three address codes are duplicated but must remain distinct from 
     * each other.
     */
    void new_id();
    
    inline bool operator==(tac_line const &rhs) const {
        return this->bid == rhs.bid;
    }

    inline bool operator<(tac_line const &rhs) const {
        return this->bid < rhs.bid;
    }

    TID bid;
    tac_op_t operation;
    std::string argument1;
    std::string argument2;
    std::string result;
    std::shared_ptr<SymbolTable> table;
public:
    /**
     * Constructs an uninitialized three address code instruction with a 
     * unique ID.
     */
    tac_line() { this->bid = bid_gen++; };
} tac_line_t;

/**
 * TACGenerator is responsible for generating three address code.
 */
class TACGenerator {
public:
    /**
     * Converts a three address code into a string representation.
     * @param tac The three address code instruction.
     * @return A human-readable string representation of the instruction.
     */
    static std::string tacLineToString(const tac_line_t &tac);

    TACGenerator();
    virtual ~TACGenerator();

    /**
     * Constructs a three address code representation for a specified operation 
     * with two optional operands within the scope of the provided symbol table.
     * @param table The symbol table in which the instruction is in scope.
     * @param operation The type of operation the code will represent.
     * @param address_a The first operand.
     * @param address_b The second operand.
     */
    tac_line_t makeQuad(
        std::shared_ptr<SymbolTable> table,
        const tac_op_t operation,
        const std::string &address_a="",
        const std::string &address_b=""
    );

    /**
     * Generates a new label for use in a three address code.
     * @return An automatically generated unique label.
     */
    std::string newLabel();

    /**
     * Provides a label with a custom name in the format a three address code 
     * label is supposed to be in.
     * @param name Custom label identifier.
     * @return Formatted label with the custom name. 
     */
    std::string customLabel(std::string name) const;
private:
    std::string newTemp();

    std::vector<tac_line_t> code;

    unsigned int tempCounter;
    unsigned int labelCounter;
};

#endif