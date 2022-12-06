#ifndef STRIP_PROFILE_H__
#define STRIP_PROFILE_H__

#include <optimizer/basic_block.h>
#include <optimizer/natural_loop.h>
#include <vector>

class StripProfile {
public:
    StripProfile(
        const NaturalLoop &loop,
        BBP bb, 
        const unsigned int factor, 
        std::vector<tac_line_t> &iteration,
        const bool vectorize,
        induction_variable_t &iterator
    );
    void unroll();
private:
    void insertVectorInstructions();

    tac_line_t getNextUseOfResult(std::vector<tac_line_t>::iterator i) const;

    bool arrayExpressionUsesIterator(
        const tac_line_t &arrOp,
        const tac_line_t &next_use
    ) const;

    bool isVariableDependentOnIndex(const std::string &variable) const;

    bool canSquashLoop() const;

    const NaturalLoop &loop;
    BBP block;
    unsigned int factor;
    std::vector<tac_line_t> &iteration;
    bool vectorize;
    induction_variable_t &iterator;

    std::vector<tac_line_t> vectorInsts;
};

#endif