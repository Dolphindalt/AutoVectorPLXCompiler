#include <optimizer/natural_loop.h>

#include <algorithm>
#include <assertions.h>
#include <logging.h>

NaturalLoop::NaturalLoop(BBP header, BBP footer, const Reach *reach) 
: header(header), footer(footer), reach(reach) {
    this->findInvariants();
}

/**
 * A 3AC statement is a loop invariant if its operands
 * a. Are constant. OR
 * b. Are defined outside the loop. OR
 * c. Are defined by some invariant in the same loop.
 */
void NaturalLoop::findInvariants() {
    std::set<std::string> definedOutside 
        = this->reach->getVariablesIntoBlock(this->getHeader());

    std::set<std::string> usedAfterLoopBody
        = this->reach->getVariablesOutOfBlock(this->getFooter());

    // I love the syntax of the C++ STL!!! (not)
    std::set<std::string> constantsInside;
    std::set_difference(
        definedOutside.begin(), definedOutside.end(),
        usedAfterLoopBody.begin(), usedAfterLoopBody.end(),
        std::inserter(constantsInside, constantsInside.begin())
    );

    std::set<std::string> invariants = constantsInside;
    std::set<std::string> old = invariants;

    bool changed = true;
    while (changed) {
        changed = false;

        for (const tac_line_t &line : this->getFooter()->getInstructions()) {
            // Ignore labels and unconditional jumps.
            if (!tac_line_t::has_result(line)) {
                continue;
            }
            // Consider each operand.

            bool operand1Conditions = this->isOperandInvariant(
                line.argument1, 
                this->getFooter(),
                invariants,
                definedOutside
            );

            bool operand2Conditions = this->isOperandInvariant(
                line.argument2, 
                this->getFooter(),
                invariants,
                definedOutside
            );

            if (operand1Conditions && operand2Conditions) {
                invariants.insert(line.result);
            }
        }

        if (invariants != old) {
            changed = true;
            old = invariants;
        }
    }

    INFO_LOG("Found invariants: ");
    for (auto inv : invariants) {
        INFO_LOG("%s", inv.c_str());
    }

    for (const tac_line_t &instruction : this->getFooter()->getInstructions()) {
        if (tac_line_t::has_result(instruction) && 
            invariants.count(instruction.result) != 0) {
                this->invariants.insert(instruction);
            }
    }

    INFO_LOG("Instructions with invariants: ");
    for (auto inv : this->invariants) {
        INFO_LOG("%s", TACGenerator::tacLineToString(inv).c_str());
    }

}

bool NaturalLoop::isOperandInvariant(
    const std::string &operand,
    const BBP bb,
    const std::set<std::string> &invariants,
    const std::set<std::string> &outsideDeclarations
) const {
    return operand != "" &&
        (bb->isVariableConstantInBB(operand) ||
        outsideDeclarations.count(operand) == 1 ||
        invariants.count(operand) == 1);
}

bool NaturalLoop::isSimpleLoop() const {
    for (const tac_line_t &instruction : this->getFooter()->getInstructions()) {
        if (tac_line_t::is_conditional_jump(instruction)) {
            return false;
        } else if (tac_line_t::is_procedure_call(instruction)) {
            return false;
        }
    }
    return true;
}

const BBP NaturalLoop::getHeader() const {
    return this->header;
}

const BBP NaturalLoop::getFooter() const {
    return this->footer;
}
