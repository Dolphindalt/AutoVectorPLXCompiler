#include "3ac.h"

#include <logging.h>

unsigned int tac_line_t::bid_gen = 0;

bool tac_line_t::transfers_control(const tac_line_t &line) {
    switch (line.operation) {
        case TAC_UNCOND_JMP:
        case TAC_JMP_E:
        case TAC_JMP_L:
        case TAC_JMP_G:
        case TAC_JMP_GE:
        case TAC_JMP_LE:
        case TAC_JMP_NE:
        case TAC_JMP_ZERO:
        // Yes, function calls count as a jump.
        case TAC_CALL:
            return true;
        default:
            break;
    }
    return false;
}

bool tac_line_t::is_comparision(const tac_line_t &line) {
    switch (line.operation) {
        case TAC_LESS_THAN:
        case TAC_LE_THAN:
        case TAC_GREATER_THAN:
        case TAC_GE_THAN:
        case TAC_NOT_EQUALS:
        case TAC_EQUALS:
            return true;
        default:
            break;
    }
    return false;
}

bool tac_line_t::has_result(const tac_line_t &line) {
    return line.result != "";
}

bool tac_line_t::is_conditional_jump(const tac_line &line) {
    switch (line.operation) {
        case TAC_JMP_E ... TAC_JMP_ZERO:
            return true;
        default:
            break;
    }
    return false;
}

bool tac_line_t::is_procedure_call(const tac_line &line) {
    return line.operation == TAC_CALL || line.operation == TAC_RETVAL;
}

bool tac_line_t::is_binary_operation(const tac_line &line) {
    // Certain comparison operations are binary, but they store no result,
    // so we no longer consider them binary.
    if (line.result == "") {
        return false;
    }

    switch (line.operation) {
        case TAC_ASSIGN ... TAC_ARRAY_INDEX:
            return true;
        default:
            break;
    }
    return false;
}

bool tac_line_t::is_label(const std::string &label) {
    return label.size() >= 2 && label.at(0) == '$' && label.at(1) == 'L';
}

std::string tac_line_t::extract_label(const std::string &label) {
    ASSERT(tac_line_t::is_label(label));
    return label.substr(2, label.size() - 2);
}

bool tac_line_t::is_user_defined_var(const std::string &var) {
    return !(
        var.size() >= 2 && var.at(0) == '$' && var.at(1) == 't'
    );
}

std::string TACGenerator::tacLineToString(const tac_line_t &tac) {
    std::string result = std::to_string(tac.bid) + 
        ": " + tacOpToStringMap.at(tac.operation);
    if (tac.result != "") {
        result += " " + tac.result;
    }
    if (tac.argument1 != "") {
        result += " " + tac.argument1;
    }
    if (tac.argument2 != "") {
        result += " " + tac.argument2;
    }
    return result;
}

TACGenerator::TACGenerator() : tempCounter(0), labelCounter(0) {}

TACGenerator::~TACGenerator() {}

tac_line_t TACGenerator::makeQuad(
    std::shared_ptr<SymbolTable> table,
    const tac_op_t operation,
    const std::string &address_a,
    const std::string &address_b
) {
    tac_line_t line;
    line.table = table;
    line.operation = operation;
    line.argument1 = "";
    line.argument2 = "";
    line.result = "";

    switch (operation) {
        // Nullary instructions.
        case TAC_NOP:
            // Do nothing.
            break;
        case TAC_ENTER_PROC:
            // Function prologue marker.
            // Name of function provided.
            line.argument1 = address_a;
            break;
        case TAC_EXIT_PROC:
            // Function epilogue marker.
            // Name of function provided.
            line.argument1 = address_a;
            break;
        // Unary operations.
        case TAC_NEGATE:
            line.argument1 = address_a;
            line.result = address_a;
            break;
        case TAC_UNCOND_JMP:
            // Target label.
            line.argument1 = address_a;
            break;
        case TAC_READ:
            line.argument1 = address_a;
            break;
        case TAC_WRITE:
            line.argument1 = address_a;
            break;
        case TAC_LABEL:
            // Check if existing label is provided; generate if not.
            if (address_a == "") {
                line.argument1 = this->newLabel();
            } else {
                line.argument1 = address_a;
            }
            break;
        case TAC_CALL:
            // Target label.
            // All functions are made into labels with their name appended
            // to the label identifier. The calling of the procedure needs
            // this modification also or it won't jump to the right label.
            line.argument1 = "$L" + address_a;
            break;
        case TAC_JMP_E ... TAC_JMP_ZERO:
            // Target label.
            line.argument1 = address_a;
            break;
        case TAC_RETVAL:
            // Store return address in argument 1.
            line.argument1 = address_a;
            break;
        case TAC_PROC_PARAM:
            // Store parameter name in argument 1.
            line.argument1 = address_a;
            break;
        // Binary operations.
        case TAC_ASSIGN ... TAC_ARRAY_INDEX:
            // An operand is always stored in the first argument.
            line.argument1 = address_a;
            // Is the operation a binary assignment in the form a = b?
            if (operation == TAC_ASSIGN) {
                // If so, address b is the operand and a is the result.
                // a = b
                line.result = address_a;
                line.argument1 = address_b;
            } else {
                // Otherwise, result stores the result of the binary operation.
                // tn = a + b
                line.result = this->newTemp();
                line.argument2 = address_b;
            }
            break;
        default:
            ERROR_LOG(
                "tried to generate an invalid 3AC operation %d",
                operation
            );
            break;
    }

    return line;
}

std::string TACGenerator::newTemp() {
    return "$t" + std::to_string(this->tempCounter++);
}

std::string TACGenerator::newLabel() {
    return "$L" + std::string("NO") + std::to_string(this->labelCounter++);
}

std::string TACGenerator::customLabel(std::string name) const {
    return "$L" + name;
}