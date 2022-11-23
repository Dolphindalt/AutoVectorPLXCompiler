#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include <3ac.h>
#include <vector>

class Preprocessor {
public:
    Preprocessor(std::vector<tac_line_t> &instructions);
private:
    void preprocess();

    void applyTwoInstructionRewrite();

    /**
     * Normally, this is implemented by converting the 3AC statements into 
     * a DAG and then transforming the DAG back into 3AC, but I can remove 
     * the redundancy using rewrite rules on the 3AC, similar to how it 
     * would be done in machine dependent optimization.
     * 
     * Also, the rule is hardcoded, because there won't be as many as with
     * rewrite rules for machine dependent optimizations.
     * 
     * @return True if the second instruction should be removed.
     */
    bool applyRedundantRewriteRule(tac_line_t &i1, tac_line_t &i2);

    /**
     * All loop headers are generated into the following form:
     * $0 = op1 cond op2
     * jump if $0 is zero
     * This can be converted into a more compact form where all cond operaions 
     * are the compare (cmp) operation and then the jump type determines the 
     * comparison performed.
     * 
     * cmp op1 op2 for each condition implies a jump:
     * = then je
     * # then jne
     * < then jg
     * > then jl
     * <= then jge
     * >= then jle
     * 
     * This happens inline.
     */
    void convertLoopOperation(tac_line_t &i1, tac_line_t &i2);

    std::vector<tac_line_t> &instructions;
};

#endif