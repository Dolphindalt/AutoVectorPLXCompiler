#include <optimizer/preprocessing.h>

Preprocessor::Preprocessor(std::vector<tac_line_t> &instructions) 
: instructions(instructions) {
    this->preprocess();
}

void Preprocessor::preprocess() {
    this->applyTwoInstructionRewrite();
}

void Preprocessor::applyTwoInstructionRewrite() {
    for (size_t i = 1; i < this->instructions.size(); i++) {
        tac_line_t &i1 = this->instructions.at(i-1);
        tac_line_t &i2 = this->instructions.at(i);

        bool didRemove = this->applyRedundantRewriteRule(i1, i2);

        if (didRemove) {
            this->instructions.erase(this->instructions.begin() + i);
            i--;
        }

        this->convertLoopOperation(i1, i2);
    }
}

bool Preprocessor::applyRedundantRewriteRule(tac_line_t &i1, tac_line_t &i2) {
    if (tac_line_t::is_binary_operation(i1) && i2.operation == TAC_ASSIGN) {
        // Looking for instructions in this form:
        // $1 = x op y
        // x = $1
        // To convert into:
        // x = x op y

        // Both are temporary as they start with $.
        if (i2.argument1 == i1.result && i1.result.at(0) == '$') {

            // x = x op y
            if (i1.argument1 == i2.result) {
                i1.result = i1.argument1;
            }

            // x = y op x
            if (i1.argument2 == i2.result) {
                i1.result = i1.argument2;
            }

            return true;
        }
    }
    return false;
}

void Preprocessor::convertLoopOperation(tac_line_t &i1, tac_line_t &i2) {
    if (tac_line_t::is_comparision(i1) && i2.operation == TAC_JMP_ZERO) {
        // The first instruction no longer stores the result.
        // The result is contained within the special registers.
        i1.result = "";
        // Now the jump must change depending on the previous instruction.
        switch (i1.operation) {
            case TAC_EQUALS:
                i2.operation = TAC_JMP_E;
                break;
            case TAC_NOT_EQUALS:
                i2.operation = TAC_JMP_NE;
                break;
            case TAC_GREATER_THAN:
                i2.operation = TAC_JMP_L;
                break;
            case TAC_LESS_THAN:
                i2.operation = TAC_JMP_G;
                break;
            case TAC_LE_THAN:
                i2.operation = TAC_JMP_GE;
                break;
            case TAC_GE_THAN:
                i2.operation = TAC_JMP_LE;
                break;
            default:
                break;
        }
    }
}