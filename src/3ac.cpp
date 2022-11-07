#include "3ac.h"

#include <logging.h>

bool tac_line::transfers_control(const tac_line &line) {
    switch (line.operation) {
        case TAC_UNCOND_JMP:
        case TAC_JMP_E:
        case TAC_JMP_L:
        case TAC_JMP_G:
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

std::string TACGenerator::tacLineToString(const tac_line_t &tac) {
    std::string result = tacOpToStringMap.at(tac.operation);
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
    const tac_op_t operation,
    const std::string &address_a,
    const std::string &address_b
) {
    tac_line_t line;
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
            break;
        case TAC_EXIT_PROC:
            // Function epilogue marker.
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
                // If so, address b stores the result.
                // a = b
                line.result = address_b;
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
    return "$" + std::to_string(this->tempCounter++);
}

std::string TACGenerator::newLabel() {
    return "$L" + std::to_string(this->labelCounter++);
}

std::string TACGenerator::customLabel(std::string name) const {
    return "$L" + name;
}