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

        bool didRemove = this->applyRedundantRewriteRule(i1, i2) ||
            this->removeSingleAssignments(i2);

        if (didRemove) {
            this->instructions.erase(this->instructions.begin() + i);
            i--;
        }
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

bool Preprocessor::removeSingleAssignments(tac_line_t &i2) {
    return i2.operation == TAC_ASSIGN && 
        i2.argument1 == "" && 
        i2.argument2 == "";
}