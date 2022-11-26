#include <optimizer/natural_loop.h>

#include <algorithm>
#include <assertions.h>
#include <logging.h>

NaturalLoop::NaturalLoop(
    BBP header, BBP footer, const Reach *reach, const Dominator *dom
) 
: header(header), footer(footer), reach(reach), dom(dom) {
    this->findInvariants();
    this->findInductionVariables();
}

void NaturalLoop::forEachBBInBody(std::function<void(BBP)> action) {
    // Get the first block in the loop body.
    BBP current = this->footer;
    while (current != nullptr) {
        action(current);
        current = this->findNextDommed(current);
    }
}

/**
 * A 3AC statement is a loop invariant if its operands
 * a. Are constant. OR
 * b. Are defined outside the loop. OR
 * c. Are defined by some invariant in the same loop.
 */
void NaturalLoop::findInvariants() {
    this->invariants.clear();
    this->instructionsWithInvariants.clear();

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
                definedOutside,
                line.table
            );

            bool operand2Conditions = this->isOperandInvariant(
                line.argument2, 
                this->getFooter(),
                invariants,
                definedOutside,
                line.table
            );

            if (operand1Conditions) {
                invariants.insert(line.argument1);
            }

            if (operand2Conditions) {
                invariants.insert(line.argument2);
            }

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
        this->invariants.insert(inv);
    }

    for (const tac_line_t &instruction : this->getFooter()->getInstructions()) {
        if (tac_line_t::has_result(instruction) && 
            invariants.count(instruction.result) != 0) {
                this->instructionsWithInvariants.insert(instruction);
            }
    }

    INFO_LOG("Instructions with invariants: ");
    for (auto inv : this->instructionsWithInvariants) {
        INFO_LOG("%s", TACGenerator::tacLineToString(inv).c_str());
    }

}

void NaturalLoop::findInductionVariables() {
    this->inductionVariables.clear();

    // Find variables in the simple form X := X + C where C is a constant.
    this->forEachBBInBody([this](BBP bb) {
        for (const tac_line_t &inst : bb->getInstructions()) {
            if (inst.operation == TAC_ADD || inst.operation == TAC_SUB) {
                // We got the right form, do the variables match?
                // Recall that all constants will be invariant.
                // X := X op C
                if (inst.result == inst.argument1 && 
                    this->isInvariant(inst.argument2)) {
                        // X is an induction variable.
                        this->simpleInductionVariables.insert(inst.result);
                        this->inductionVariables.insert(inst.result);
                    }
                // X := C op X
                if (inst.result == inst.argument2 && 
                    this->isInvariant(inst.argument1)) {
                        this->simpleInductionVariables.insert(inst.result);
                        this->inductionVariables.insert(inst.result);
                    }
            }
        }
    });

    INFO_LOG("Found induction variables: ");
    for (auto str : this->inductionVariables) {
        INFO_LOG("%s", str.c_str());
    }
}

bool NaturalLoop::isSimpleLoop() const {
    for (const tac_line_t &instruction : this->getFooter()->getInstructions()) {
        if (tac_line_t::is_conditional_jump(instruction)) {
            return false;
        } else if (tac_line_t::is_procedure_call(instruction)) {
            return false;
        } else if (tac_line_t::is_read_or_write(instruction)) {
            return false;
        }
    }
    return true;
}

bool NaturalLoop::isInvariant(const std::string &value) const {
    return this->invariants.count(value) > 0;
}

const BBP NaturalLoop::getHeader() const {
    return this->header;
}

const BBP NaturalLoop::getFooter() const {
    return this->footer;
}

bool NaturalLoop::isOperandInvariant(
    const std::string &operand,
    const BBP bb,
    const std::set<std::string> &invariants,
    const std::set<std::string> &outsideDeclarations,
    const std::shared_ptr<SymbolTable> &table
) const {
    unsigned int level;
    st_entry_t entry;
    bool success = table->lookup(operand, &level, &entry);

    bool isConstant;

    if (success) {
        isConstant = entry.entry_type == ST_LITERAL || 
            (entry.entry_type == ST_VARIABLE && entry.variable.isConstant);
    }

    return operand != "" &&
        (bb->isVariableConstantInBB(operand) ||
        outsideDeclarations.count(operand) == 1 ||
        invariants.count(operand) == 1 ||
        isConstant);
}

BBP NaturalLoop::findNextDommed(BBP bb) const {
    if (this->header == bb) {
        return nullptr;
    }

    for (auto cand : bb->getPredecessors()) {
        if (this->dom->dominates(cand, this->header) && cand != this->header) {
            return cand;
        }
    }

    // No dominance.
    return nullptr;
}
