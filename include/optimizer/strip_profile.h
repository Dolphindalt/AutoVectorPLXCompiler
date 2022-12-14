/**
 * This module is in charge of stripping loops and inserting vectorized code 
 * into stripped loops.
 * 
 * @file strip_profile.h
 * @author Dalton Caron
*/
#ifndef STRIP_PROFILE_H__
#define STRIP_PROFILE_H__

#include <optimizer/basic_block.h>
#include <optimizer/natural_loop.h>
#include <vector>

/** Represents the body of a loop that is to be strip mined. */
class StripProfile {
public:
    /**
     * Constructs a loop stripping profile from the following.
     * 
     * @param loop The natural loop to potentially unroll.
     * @param bb The basic block that represents the loop body.
     * @param factor The factor to unroll the loop to, i.e. the new iterator 
     * increment.
     * @param iteration All instructions that represent a single loop iteration 
     * within the loop.
     * @param vectorize True if the loop should be vectorized, else false.
     * @param iterator The loop iterator. 
     */
    StripProfile(
        const NaturalLoop &loop,
        BBP bb, 
        const unsigned int factor, 
        std::vector<tac_line_t> &iteration,
        const bool vectorize,
        induction_variable_t &iterator
    );

    /** Performs loop unrolling. */
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