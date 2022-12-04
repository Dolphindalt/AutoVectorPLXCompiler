#include <optimizer/strip_profile.h>

#include <functional>

StripProfile::StripProfile(
    BBP bb, 
    const unsigned int factor, 
    std::vector<tac_line_t> &iteration,
    const bool vectorize
) : block(bb), factor(factor), iteration(iteration), vectorize(vectorize) {
    if (this->vectorize) {
        this->insertVectorInstructions();
    }
}

void StripProfile::insertVectorInstructions() {
    /**
     * If no statements in the loop are dependent upon the i iterator 
     * (excluding those that are indices to arrays), then
     * the loop can be collapsed such that all operations are performed in 
     * parallel in a single iteration.
     */

    /**
     * Keep in mind that there is an assumption that the loop can be executed 
     * in parallel, so arrays will never contain any data dependencies. This 
     * could be relaxed to support a wider range of loops.
     */

    /**
     * In normal code generation, we do not differentiate between load and 
     * stores, but in vector land we must do so. We need to look ahead to see
     * if array indexing is used as an operand or a result and then generate
     * the appropriate insrtuction.
     * 
     * A variable will be loaded if it is the result of an array index and 
     * its next use is as an operand.
     * 
     * A variable will be stored if it is the result of an array index and 
     * its next use is as a result.
     */
    
    // Do a single pass to mark all array variables.
    std::map<std::string, tac_line_t> arrayVarInfo;

    std::function<bool(const std::string &)> isArrayVar = 
        [&arrayVarInfo](const std::string &var) {
            return arrayVarInfo.count(var) > 0;
        };

    std::function<tac_line_t(const tac_line_t &old, tac_op_t op)> makeInstCpyN = 
        [](const tac_line_t &old, tac_op_t op) {
            tac_line_t load;
            load.operation = op;
            load.argument1 = old.argument1;
            load.argument2 = old.argument2;
            load.result = old.result;
            load.table = old.table;
            return load;
        };

    for (auto i = this->iteration.begin(); i != this->iteration.end(); i++) {
        const tac_line_t inst = *i;

        // Mainly convert instructions in these forms:
        // c[i] = a[i] + b[i]
        // c[i] = a[i] + c
        switch (inst.operation) {
            case TAC_ARRAY_INDEX:
                // We will generate indexes as load and stores when they are
                // used.
                arrayVarInfo.insert(std::make_pair(inst.result, inst));
                this->iteration.erase(i);
                i--;
                break;
            case TAC_ADD:
            case TAC_SUB: {
                if (isArrayVar(inst.argument1) || isArrayVar(inst.argument2)) {
                    this->iteration.erase(i);
                    i--;

                    tac_line_t newInst = makeInstCpyN(
                        inst, (inst.operation == TAC_ADD) ? TAC_VADD : TAC_VSUB
                    );

                    if (isArrayVar(newInst.argument1)) {
                        const tac_line_t arg1Load = makeInstCpyN(
                            arrayVarInfo.at(newInst.argument1), TAC_VLOAD
                        );
                        this->vectorInsts.push_back(arg1Load);
                    }

                    if (isArrayVar(newInst.argument2)) {
                        const tac_line_t arg2Load = makeInstCpyN(
                            arrayVarInfo.at(newInst.argument2), TAC_VLOAD
                        );
                        this->vectorInsts.push_back(arg2Load);
                    }

                    this->vectorInsts.push_back(newInst);


                    // Any array computations stored to a temp variable 
                    // will be aliased later on or stored later.
                    arrayVarInfo.insert(
                        std::make_pair(newInst.result, newInst)
                    );
                }

                break;
            }
            case TAC_ASSIGN: {
                // This is an alias, c = $1, $1 = a + b.
                // It can also be a copy assignment c = a where a is loaded.
                if (isArrayVar(inst.argument1)) {
                    this->vectorInsts
                        .push_back(makeInstCpyN(inst, TAC_VASSIGN));
                    this->iteration.erase(i);
                    i--;
                }
            }
            default:
                break;
        }
    }

    if (this->canSquashLoop()) {
        tac_line_t &iterator = this->iteration.at(0);

        st_entry_t lit_info;
        iterator.table->lookupOrInsertIntConstant(this->factor, &lit_info);
        ASSERT(lit_info.entry_type == ST_LITERAL);
        ASSERT(lit_info.literal.type == INT);
        const std::string newItrIncr = std::to_string(this->factor);

        if (iterator.is_operand_constant(iterator.argument1)) {
            iterator.argument1 = newItrIncr;
        } else if (iterator.is_operand_constant(iterator.argument2)) {
            iterator.argument2 = newItrIncr;
        }
        this->iteration.clear();
        this->vectorInsts.push_back(iterator);
    }

}

bool StripProfile::canSquashLoop() const {
    // It is just the iterator.
    return this->iteration.size() == 1;
}

void StripProfile::unroll() {
    const tac_line_t lastInstructionJmp = this->block->getInstructions().back();
    ASSERT(lastInstructionJmp.operation == TAC_UNCOND_JMP);

    this->block->getInstructions().clear();

    // Insert vector instructions.
    this->block->insertInstructions(this->vectorInsts, false);

    // Insert serial instructions.
    for (unsigned int i = 0; i < this->factor; i++) {
        this->block->insertInstructions(this->iteration, false);
    }

    this->block->insertInstruction(lastInstructionJmp);
}