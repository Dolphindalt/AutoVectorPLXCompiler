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

        this->forEachBBInBody([&invariants, &constantsInside](BBP bb) {
            for (const tac_line_t &line : bb->getInstructions()) {
            // Ignore labels and unconditional jumps.
            if (!tac_line_t::has_result(line)) {
                continue;
            }

            // We need to be careful as the invariant definition from the 
            // my textbook is not technically right. See
            // https://www.cs.cmu.edu/~aplatzer/course/Compilers11/17-loopinv.pdf 
            // for a better definition.
            if (line.is_operand_constant(line.argument1) ||
                constantsInside.count(line.argument1) > 0) {
                    invariants.insert(line.argument1);
            }

            if (line.is_operand_constant(line.argument2) ||
                constantsInside.count(line.argument2) > 0) {
                invariants.insert(line.argument2);
            }

            if (invariants.count(line.argument1) && 
                invariants.count(line.argument2)) {
                    invariants.insert(line.result);
                }

            }
        });

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

    // It turns out that the simple induction variables are all I need.
    // Wait, but to get the distance vectors, I need all induction variables
    // on the iterator.
    // Find more complex induction variables in the form W := A * X + B, where
    // A and B are constants or loop invariants.
    // Luckily, 3AC only allows for one operation, so we will be looking for
    // W := X + B and W := A * X independently.
    std::set<std::string> old = this->inductionVariables;

    bool changed = true;
    while (changed) {
        changed = false;

        // Find X within some expression.
        this->forEachBBInBody([this](BBP bb) {
            for (const tac_line_t &inst : bb->getInstructions()) {
                
                // Check for the addition and the multiplication.
                if (inst.operation == TAC_ADD || inst.operation == TAC_SUB ||
                    inst.operation == TAC_MULT || inst.operation == TAC_DIV) {

                        // Consider the two cases
                        // W := X op A
                        if (this->isInductionVariable(inst.argument1) &&
                            this->isInvariant(inst.argument2)) {
                                this->inductionVariables.insert(inst.result);
                            }
                        // W := A op X
                        if (this->isInductionVariable(inst.argument2) &&
                            this->isInvariant(inst.argument1)) {
                                this->inductionVariables.insert(inst.result);
                            }
                    }
                
                // For assignments in the form A = X, where X is an induction
                // variable, A is also an induction variable.
                if (inst.operation == TAC_ASSIGN) {
                    if (this->isInductionVariable(inst.argument1)) {
                        this->inductionVariables.insert(inst.result);
                    }
                }
                
            }
        });

        if (old != this->inductionVariables) {
            changed = true;
            old = this->inductionVariables;
        }
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

bool NaturalLoop::isInductionVariable(const std::string &value) const {
    return this->inductionVariables.count(value) > 0;
}

bool NaturalLoop::isSimpleInductionVariable(const std::string &value) const {
    return this->simpleInductionVariables.count(value) > 0;
}

const BBP NaturalLoop::getHeader() const {
    return this->header;
}

const BBP NaturalLoop::getFooter() const {
    return this->footer;
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
