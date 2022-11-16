#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include <3ac.h>
#include <vector>

class Preprocessor {
public:
    Preprocessor(std::vector<tac_line_t> &instructions);
private:
    void preprocess(std::vector<tac_line_t> &instructions);

    /**
     * Normally, this is implemented by converting the 3AC statements into 
     * a DAG and then transforming the DAG back into 3AC, but I can remove 
     * the redundancy using rewrite rules on the 3AC, similar to how it 
     * would be done in machine dependent optimization.
     * 
     * Also, the rule is hardcoded, because there won't be as many as with
     * rewrite rules for machine dependent optimizations.
     */
    void applyRedundantRewriteRule(std::vector<tac_line_t> &instructions);
};

#endif