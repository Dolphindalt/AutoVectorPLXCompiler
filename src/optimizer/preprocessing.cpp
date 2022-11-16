#include <optimizer/preprocessing.h>

Preprocessor::Preprocessor(std::vector<tac_line_t> &instructions) {
    this->preprocess(instructions);
}

void Preprocessor::preprocess(std::vector<tac_line_t> &instructions) {
    this->applyRedundantRewriteRule(instructions);
}

void Preprocessor::applyRedundantRewriteRule(
    std::vector<tac_line_t> &instructions
) {
    // My two instructions peephole.
    for (size_t i = 1; i < instructions.size(); i++) {
        tac_line_t &i1 = instructions.at(i-1);
        tac_line_t &i2 = instructions.at(i);

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

                instructions.erase(instructions.begin() + i);
                i--;
            }
        }
    }
}