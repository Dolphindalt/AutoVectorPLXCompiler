#include <optimizer/loop_vectorizer.h>

#include <optimizer/strip_profile.h>
#include <assertions.h>

#define FAIL_MESSAGE "Failed to vectorize loop: "

LoopVectorizer::LoopVectorizer(NaturalLoop &loop) : loop(loop) {
    this->canVectorize = this->checkCanLoopBeVectorized();
}

void LoopVectorizer::vectorize() {
    if (this->canVectorize) {
        INFO_LOG("Can vectorize loop %s", loop.to_string().c_str());
    } else {
        WARNING_LOG("Cannot vectorize loop %s", loop.to_string().c_str());
        return;
    }

    if (!this->shouldVectorizeLoop()) {
        WARNING_LOG(
            "Declined to vectorize loop %s", loop.to_string().c_str()
        );
        return;
    }

    // Duplicate the loop after the current loop.
    loop.duplicateLoopAfterThisLoop();

    // Strip mine the loop.
    // By default, all variables are 8 bytes and fit into a 256 bit register.
    this->stripMineLoop(4);
}

void LoopVectorizer::stripMineLoop(const unsigned int unroll) {
    // Step 1: find the induction variable. We did that already.
    // We know it is a simple induction variable in the form i = i + 1.

    // Step 2: duplicate all instructions unroll times in each basic block.
    // We are confined to simple loops, so the only basic block will be the 
    // footer block, but this could be changed in the future.
    std::vector<tac_line_t> instructionGroup;
    this->loop.forEachBBInBody([this, &instructionGroup](BBP bb) {
        for (tac_line_t inst : bb->getInstructions()) {
            instructionGroup.push_back(inst);
        }

        if (this->loop.getFooter() == bb && bb->changesControlAtEnd()) {
            instructionGroup.pop_back();
        }
    });

    StripProfile profile(
        this->loop, 
        this->loop.getFooter(), 
        unroll, 
        instructionGroup, 
        true, 
        this->index
    );

    profile.unroll();
}

bool LoopVectorizer::isInstructionDependentOnIndex(
    const NaturalLoop &loop,
    const tac_line_t &inst,
    const induction_variable_t &index
) {
    return isVariableDependentOnIndex(loop, inst.argument2, index) || 
        isVariableDependentOnIndex(loop, inst.argument1, index);
}

bool LoopVectorizer::isVariableDependentOnIndex(
    const NaturalLoop &loop,
    const std::string &variable,
    const induction_variable_t &index
) {
    if (variable == "") {
        return false;
    }

    if (variable == index.inductionVar) {
        return true;
    }

    std::vector<BBP> loopBody;
    loop.forEachBBInBody([&loopBody](BBP bb) {
        loopBody.push_back(bb);
    });

    for (BBP bb : loopBody) {

        if (bb->getDefChain().count(variable) > 0) {

            const std::vector<tac_line_t> &definitions = 
                bb->getDefChain().at(variable);
            
            for (const tac_line_t &inst : definitions) {
                if (inst.result == index.inductionVar) {
                    return true;
                }
                return LoopVectorizer::isInstructionDependentOnIndex(
                    loop, inst, index);
            }

        }

    }

    return false;
}

bool LoopVectorizer::checkCanLoopBeVectorized() {
    // Loop must be a simple loop.
    if (!loop.isSimpleLoop()) {
        WARNING_LOG(FAIL_MESSAGE "Loop is not simple");
        return false;
    }

    bool foundIterator = loop.identifyLoopIterator(this->index);
    if (!foundIterator) {
        WARNING_LOG(FAIL_MESSAGE "Failed to determine loop iterator");
        return false;
    }

    // We will only vectorize loops of simple induction variables that 
    // increment by 1 (makes strip mining easier).
    if (this->index.constant != "1") {
        WARNING_LOG(FAIL_MESSAGE "Increment is not 1");
        return false;
    }

    bool success = this->computeDirectionVectors(directionVectors);
    if (!success) {
        WARNING_LOG(FAIL_MESSAGE "Failed to determine direction vector");
        return false;
    }

    for (const int &d : directionVectors) {
        if (d != DISTANCE_EQUAL) {
            WARNING_LOG(FAIL_MESSAGE "Data dependence");
            return false;
        }
    }

    return true;
}

bool LoopVectorizer::computeDirectionVectors(
    std::vector<int> &direction_vectors_out
) const {
    direction_vectors_out.clear();

    for (const tac_line_t &inst : loop.getFooter()->getInstructions()) {
        if (inst.operation == TAC_ARRAY_INDEX) {
            bool dependsOnIterator;
            int vector_out;
            bool foundVector = this->getDirectionVectorFromVariable(
                inst.argument2, 
                index.inductionVar, 
                dependsOnIterator, 
                vector_out
            );

            if (!foundVector) {
                return false;
            }

            if (dependsOnIterator) {
                direction_vectors_out.push_back(vector_out);
            }
        }
    }

    return true;
}

bool LoopVectorizer::getDirectionVectorFromVariable(
        const std::string &variable,
        const std::string &index,
        bool &dependsOnIterator,
        int &vector_out
) const {
    dependsOnIterator = false;
    bool result = this->getDistanceVectorFromVariable(
        variable, index, dependsOnIterator, vector_out
    );

    if (vector_out > 0) {
        vector_out = DISTANCE_MORE;
    } else if (vector_out < 0) {
        vector_out = DISTANCE_LESS;
    } else {
        vector_out = DISTANCE_EQUAL;
    }

    return result;
}

bool LoopVectorizer::getDistanceVectorFromVariable(
    const std::string &variable,
    const std::string &index,
    bool &containedIterator,
    int &vector_out
) const {
    const BBP body = loop.getFooter();
    // Variables in a basic block share the same scope.
    const std::shared_ptr<SymbolTable> sym = body->getInstructions().at(0).table;
    
    // Check if the variable is the index.
    if (variable == index) {
        containedIterator = true;
        vector_out = DISTANCE_EQUAL;
        return true;
    }

    unsigned int level;
    st_entry_t entry;
    sym->lookup(variable, &level, &entry);

    // Check if the variable is a constant.
    if (entry.entry_type == ST_LITERAL) {
        ASSERT(entry.literal.type == INT);
        vector_out = entry.literal.value.int_value;
        return true;
    }

    if (entry.entry_type == ST_VARIABLE && entry.variable.isConstant) {
        ASSERT(entry.literal.type == INT);
        vector_out = entry.variable.value.int_value;
        return true;
    }

    // Otherwise, we need to evaluate.
    ASSERT(entry.entry_type == ST_VARIABLE);
    // This variable can only be defined once in the loop body.
    const std::vector<tac_line_t> &insts = body->getDefChain().at(variable);
    if (insts.size() == 1) {
        const tac_line_t &inst = insts.at(0);
        // Only specific operations are supported.
        switch (inst.operation) {
            case TAC_ADD:
            case TAC_SUB:
            case TAC_DIV:
            case TAC_MULT: {
                int vectorOut1, vectorOut2;
                bool gotOp1 = this->getDistanceVectorFromVariable(
                    inst.argument1, index, containedIterator, vectorOut1
                );
                bool gotOp2 = this->getDistanceVectorFromVariable(
                    inst.argument1, index, containedIterator, vectorOut2
                );

                if ((!gotOp1) || (!gotOp2)) {
                    return false;
                }

                vector_out = vectorOut1 + vectorOut2;
                return true;
            }
            default:
                break;
        }
    }

    // Not constant and defined outside the loop, so we don't know.
    return false;
}

bool LoopVectorizer::shouldVectorizeLoop() const {
    unsigned int arrayWrites = 0;
    std::set<std::string> arrayVariables;
    this->loop.forEachBBInBody(
        [&arrayWrites, &arrayVariables, this](BBP bb) {
            for (const tac_line_t &inst : bb->getInstructions()) {
                if (
                    inst.operation == TAC_ARRAY_INDEX && 
                    LoopVectorizer::isInstructionDependentOnIndex(
                        loop, inst, index)
                    ) {
                        arrayVariables.insert(inst.result);
                } else if (arrayVariables.count(inst.result) > 0) {
                    arrayWrites++;
                }
            }
        }
    );

    bool shouldVectorize = arrayWrites > 0;

    return shouldVectorize;
}