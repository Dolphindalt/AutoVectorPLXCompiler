#include <optimizer/loop_vectorizer.h>

#include <assertions.h>

#define FAIL_MESSAGE "Failed to vectorize loop: "

LoopVectorizer::LoopVectorizer(NaturalLoop &loop) : loop(loop) {
    this->canVectorize = this->checkCanLoopBeVectorized();
    if (this->canVectorize) {
        INFO_LOG("Can vectorize loop %s", loop.to_string().c_str());
    } else {
        WARNING_LOG("Cannot vectorized loop %s", loop.to_string().c_str());
    }
}

void LoopVectorizer::vectorize() {
    // Strip mine the loop.
}

bool LoopVectorizer::checkCanLoopBeVectorized() {
    // Loop must be a simple loop.
    if (!loop.isSimpleLoop()) {
        WARNING_LOG(FAIL_MESSAGE "Loop is not simple");
        return false;
    }

    bool foundIterator = loop.identifyLoopIterator(index);
    if (!foundIterator) {
        WARNING_LOG(FAIL_MESSAGE "Failed to determine loop iterator");
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
                inst.argument2, index, dependsOnIterator, vector_out
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