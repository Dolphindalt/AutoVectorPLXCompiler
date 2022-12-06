#include <optimizer/natural_loop.h>

#include <algorithm>
#include <assertions.h>
#include <logging.h>

NaturalLoop::NaturalLoop(
    BBP header, 
    BBP footer, 
    const Reach *reach, 
    const Dominator *dom,
    BlockSet &allBlocks
) 
: header(header), footer(footer), reach(reach), dom(dom), allBlocks(allBlocks) {
    this->findInvariants();
    this->findInductionVariables();
}

void NaturalLoop::forEachBBInBody(std::function<void(BBP)> action) const {
    // Get the first block in the loop body.
    BBP current = this->footer;
    std::set<BBP> visited;
    this->forEachBBInBodyInternal(current, action, visited);
}

/**
 * A 3AC statement is a loop invariant if its operands
 * a. Are constant. OR
 * b. Are defined outside the loop. OR
 * c. Are defined by some invariant in the same loop.
 * 
 * Revised this definition below.
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
                        induction_variable_t var(
                            true, inst.result, inst.argument2, nullptr
                        );
                        this->simpleInductionVariables.insert(
                            std::make_pair(inst.result, var));
                        this->inductionVariables
                            .insert(std::make_pair(inst.result, var));
                    }
                // X := C op X
                if (inst.result == inst.argument2 && 
                    this->isInvariant(inst.argument1)) {
                        induction_variable_t var(
                            true, inst.result, inst.argument1, nullptr
                        );
                        this->simpleInductionVariables.insert(
                            std::make_pair(inst.result, var));
                        this->inductionVariables
                            .insert(std::make_pair(inst.result, var));
                    }
            }
        }
    });

    INFO_LOG("Found simple induction variables: ");
    for (auto str : this->inductionVariables) {
        INFO_LOG("%s", str.first.c_str());
    }

    // It turns out that the simple induction variables are all I need.
    // Wait, but to get the distance vectors, I need all induction variables
    // on the iterator.
    // Find more complex induction variables in the form W := A * X + B, where
    // A and B are constants or loop invariants.
    // Luckily, 3AC only allows for one operation, so we will be looking for
    // W := X + B and W := A * X independently.
    std::map<std::string, induction_variable_t> old = this->inductionVariables;

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
                                induction_variable_t var = induction_variable_t(
                                    false, inst.result, inst.argument2, 
                                    &this->inductionVariables.at(inst.argument1)
                                );
                                this->inductionVariables
                                    .insert(std::make_pair(inst.result, var));
                            }
                        // W := A op X
                        if (this->isInductionVariable(inst.argument2) &&
                            this->isInvariant(inst.argument1)) {
                                induction_variable_t var = induction_variable_t(
                                    false, inst.result, inst.argument1, 
                                    &this->inductionVariables.at(inst.argument2)
                                );
                                this->inductionVariables
                                    .insert(std::make_pair(inst.result, var));
                            }
                    }
                
                // For assignments in the form A = X, where X is an induction
                // variable, A is also an induction variable.
                if (inst.operation == TAC_ASSIGN) {
                    if (this->isInductionVariable(inst.argument1)) {
                        induction_variable_t &old = this->inductionVariables
                            .at(inst.argument1);
                        induction_variable_t var = induction_variable_t(
                            old.is_simple, inst.result, old.constant, &old
                        );
                        this->inductionVariables.insert(
                            std::make_pair(inst.result, var)
                        );
                    }
                }
                
            }
        });

        if (old != this->inductionVariables) {
            changed = true;
            old = this->inductionVariables;
        }
    }

    INFO_LOG("Found induction variables: ");
    for (auto str : this->inductionVariables) {
        INFO_LOG("%s", str.first.c_str());
    }
}

bool NaturalLoop::identifyLoopIterator(induction_variable_t &varNameOut) const {
    // TODO: This needs to be made more sophisticated to detect more a wider
    // range of loop classes. We will detect candidate induction variables 
    // that are DIRECT operands in the loop header.

    bool foundOne = false;

    for (const tac_line_t &inst : this->header->getInstructions()) {
        if (tac_line_t::is_comparision(inst)) {

            if (this->isSimpleInductionVariable(inst.argument1)) {
                if (!foundOne) {
                    foundOne = true;
                    varNameOut = this->inductionVariables.at(inst.argument1);
                } else return false;
            }

            if (this->isSimpleInductionVariable(inst.argument2)) {
                if (!foundOne) {
                    foundOne = true;
                    varNameOut = this->inductionVariables.at(inst.argument2);
                } else return false;
            }

        }
    }

    return foundOne;
}

void NaturalLoop::duplicateLoopAfterThisLoop() {
    BBP exit = this->getExit();
    // The new blocks occur after the footer but before the exit, so they
    // will have the same major id as the footer block.
    const unsigned int newBlocksMajorId = this->getFooter()->getID();

    // Copying of the loop blocks.
    // The header must be made the predecessor to the footer and the 
    // header made the successor to the footer.
    //
    // LHead -> ... -> LFoot -> LExit
    //
    // LHead -> NewHead -> ... -> NewFoot -> LExit
    std::vector<BBP> copyLoop;

    BBP footerCopy = std::make_shared<BasicBlock>(
        newBlocksMajorId, this->getFooter()
    );
    footerCopy->clearPredecessors();
    footerCopy->clearSuccessors();

    // NewFoot -> LExit
    footerCopy->insertSuccessor(exit);
    exit->clearPredecessors();
    exit->insertPredecessor(footerCopy);

    copyLoop.push_back(footerCopy);

    BBP headerCopy = std::make_shared<BasicBlock>(
        newBlocksMajorId, this->getHeader()
    );
    headerCopy->clearPredecessors();
    headerCopy->clearSuccessors();
    
    // Loop backedge from header to footer.
    headerCopy->insertPredecessor(footerCopy);
    footerCopy->insertSuccessor(headerCopy);

    // ... -> NewFoot
    BBP successor = footerCopy;
    this->forEachBBInBody(
        [&copyLoop, &successor, this, newBlocksMajorId](BBP body) {
            // We deal with the footer outside the loop.
            if (body == this->getFooter()) {
                return;
            }

            // Invariant: Footer dominates header implies
            // Invariant: Loops are traversed in backwards order implies
            // Invariant: Each block succeeds/preceeds the other.
            BBP bodyCopy = std::make_shared<BasicBlock>(
                newBlocksMajorId, body
            );

            bodyCopy->clearPredecessors();
            bodyCopy->clearSuccessors();

            bodyCopy->insertSuccessor(successor);
            successor->insertPredecessor(bodyCopy);

            successor = bodyCopy;

            copyLoop.push_back(bodyCopy);
        }
    );

    BBP firstInLoopBody = copyLoop.back();
    // LHead -> NewHead -> ...

    this->getHeader()->removeSuccessor(exit);
    this->getHeader()->insertSuccessor(headerCopy);

    firstInLoopBody->insertPredecessor(headerCopy);
    firstInLoopBody->removeSuccessor(exit);
    headerCopy->insertSuccessor(firstInLoopBody);

    copyLoop.push_back(headerCopy);

    headerCopy->insertSuccessor(exit);
    exit->insertPredecessor(headerCopy);

    // Now TAC instructions and labels need to be made unique and reordered.
    for (BBP &bb : copyLoop) {
        for (tac_line_t &inst : bb->getInstructions()) {
            if (tac_line_t::is_conditional_jump(inst) 
                || inst.operation == TAC_UNCOND_JMP
                || inst.operation == TAC_LABEL) {
                    // Now it is copied :)
                    inst.argument1 += "C";
            }
        }
    }

    // The original header needs to jump to the new header
    // in the conditional.
    ASSERT(tac_line_t::is_conditional_jump(
        this->getHeader()->getInstructions().back()));

    this->getHeader()->getInstructions().back().argument1 = 
        headerCopy->getFirstLabel().argument1;

    // The new header needs to jump to the old exit in the conditional.
    ASSERT(tac_line_t::is_conditional_jump(
        headerCopy->getInstructions().back()));

    headerCopy->getInstructions().back().argument1 = 
        exit->getFirstLabel().argument1;
    
    // The blocks are backwards, so we swap their minor IDs.
    for (size_t i = 0; i < copyLoop.size() / 2; i++) {
        const size_t ridx = copyLoop.size() - 1 - i;
        const unsigned int temp = copyLoop.at(i)->getMinorId();
        copyLoop.at(i)->setMinorId(copyLoop.at(ridx)->getMinorId());
        copyLoop.at(ridx)->setMinorId(temp);
    }

    // We need to insert the new blocks.
    for (auto bb : copyLoop) {
        this->allBlocks.insert(bb);
    }

}

bool NaturalLoop::isSimpleLoop() {
    bool retValue = true;

    this->forEachBBInBody([&retValue](BBP bb) {
        for (const tac_line_t &instruction : bb->getInstructions()) {
            if (tac_line_t::is_conditional_jump(instruction)) {
                retValue = false;
            } else if (tac_line_t::is_procedure_call(instruction)) {
                retValue = false;
            }
        }
    });
    
    return retValue;
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

bool NaturalLoop::isNeverDefinedInLoop(const std::string &variable) const {
    bool assigned = false;
    this->forEachBBInBody([&assigned, &variable](BBP bb) {
        assigned = assigned || !bb->isNeverDefined(variable);
    });
    return assigned;
}

BBP NaturalLoop::getExit() const {
    for (const BBP &bbp : this->getHeader()->getSuccessors()) {
        if (!this->dom->dominates(bbp, this->getHeader())) {
            return bbp;
        }
    }
    return nullptr;
}

const BBP NaturalLoop::getHeader() const {
    return this->header;
}

const BBP NaturalLoop::getFooter() const {
    return this->footer;
}

const std::string NaturalLoop::to_string() const {
    return "(" + std::to_string(this->getHeader()->getID()) + ", " + 
        std::to_string(this->getFooter()->getID()) + ")";
}

void NaturalLoop::forEachBBInBodyInternal(
    BBP current, 
    std::function<void(BBP)> action,
    std::set<BBP> &visited
) const {
    if (current == this->header || visited.count(current) > 0) {
        return;
    }

    action(current);
    visited.insert(current);

    for (BBP pred : current->getPredecessors()) {
        this->forEachBBInBodyInternal(pred, action, visited);
    }
}
