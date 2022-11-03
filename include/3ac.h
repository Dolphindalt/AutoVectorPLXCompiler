#ifndef THREEAC_H__
#define THREEAC_H__

#include <string>
#include <vector>

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
    TAC_ARRAY_INDEX
} tac_op_t;

typedef struct tac_line {
    tac_op_t operation;
    std::string argument1;
    std::string argument2;
    std::string result;
} tac_line_t;

class TACGenerator {
public:
    TACGenerator();
    virtual ~TACGenerator();

    static std::string tacToString(const tac_line_t &tac);

    tac_line_t makeQuad(
        const tac_op_t operation,
        const std::string &address_a="",
        const std::string &address_b=""
    );

    std::string newLabel();

    std::string customLabel(std::string name) const;

    std::string to_string() const;
private:
    std::string newTemp();

    std::vector<tac_line_t> code;

    unsigned int tempCounter;
    unsigned int labelCounter;
};

#endif