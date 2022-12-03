#ifndef NATURAL_LOOP_H__
#define NATURAL_LOOP_H__

#include <optimizer/basic_block.h>
#include <optimizer/reach.h>
#include <optimizer/dominator.h>
#include <optimizer/block_types.h>

#include <functional>
#include <string>

using address = std::string;

/**
 * Represents induction variables in the two forms
 * X := X + C if is simple
 * or
 * W := A * X or W := X + B if not simple
 */
typedef struct induction_variable {
public:
    bool is_simple;
    std::string inductionVar;
    std::string constant;
    struct induction_variable *previousInductionVar;

    induction_variable() {}

    induction_variable(
        bool is_simple, 
        std::string inductionVar, 
        std::string constant,
        struct induction_variable *previous
    ) : is_simple(is_simple), inductionVar(inductionVar), constant(constant), 
        previousInductionVar(previous) {}
    
    inline bool operator==(const induction_variable &rhs) const {
        return this->inductionVar == rhs.inductionVar;
    }
} induction_variable_t;

/**
 * A natural loop is defined as the smallest set of nodes in which the back 
 * edge is included with no predecessors outside of the set except for
 * the predecessor of the header.
 * 
 * For this project, we are only interested in loops that have no control flow 
 * structures and no procedure calls. This would imply then that the loops of
 * interest contain only a header and footer node, as there are no control 
 * flow statements or procedure calls that could create intermediate basic 
 * blocks. 
 */
class NaturalLoop {
public:
    NaturalLoop(
        BBP header, 
        BBP footer, 
        const Reach *reach, 
        const Dominator *dom,
        BlockSet &allBlocks
    );

    /**
     * Because the footer dominates the header, the path to the header will
     * be dominated all the way until the footer. This allows for a traversal
     * of the loop using the dominance and DFS.
     */
    void forEachBBInBody(std::function<void(BBP)> action);

    /**
     * A 3AC statement is a loop invariant if its operands
     * a. Are constant. OR
     * b. Are defined outside the loop. OR
     * c. Are defined by some invariant in the same loop.
     */
    void findInvariants();

    /**
     * A basic induction variable take on the form 
     * X := X + C
     * X := X - C
     * An induction variable is a linear function of some induction variable.
     * More complex induction variables take on the following form
     * W := A * X + B
     * This is split into two definitions.
     * W := A * X
     * W := X + B
     */
    void findInductionVariables();

    /**
     * The loop iterator is a simple induction variable (in the form X := X + C) 
     * where the constant is 1 AND the variable is used within the loop header
     * to compute the condition. If multiple variables satisy these conditions 
     * or a loop iterator could not be found, this function returns false.
     * 
     * @param varNameOut The name of the iterator variable, if found.
     * @return False if multiple iterators exist or an iterator is not found.
     */
    bool identifyLoopIterator(induction_variable_t &varNameOut) const;

    /**
     * This function duplicates the loop after the exit of the exit of the 
     * loop. Natural loops are expected to only have one exit at which is 
     * the predecessor of the header block that does not dominate the header.
     * The copy of the loop will become the new exit to the loop header and 
     * the previous exit block becomes the exit block of the copy loop.
     * 
     * This will only work on loops where the footer dominates every block to
     * the header.
     */
    void duplicateLoopAfterThisLoop();

    /** 
     * A simple loop has its header as a predecessor and successor of the 
     * footer. If the loop is an outer loop in a loop nesting, then the header 
     * has the footer as a predecessor and the header has the inner loop header
     * as a successor.
     * 
     * This simplifies the kinds of loops that will be considered for 
     * optimizations by eliminating loops with conditional statements and 
     * loops that require intraprocedural analysis.
     */
    bool isSimpleLoop() const;

    bool isInvariant(const std::string &value) const;

    bool isInductionVariable(const std::string &value) const;

    bool isSimpleInductionVariable(const std::string &value) const;

    BBP getExit() const;

    const BBP getHeader() const;
    const BBP getFooter() const;

    const std::string to_string() const;
private:
    BBP findNextDommed(BBP bb) const;

    BBP header;
    BBP footer;
    const Reach *reach;
    const Dominator *dom;
    BlockSet &allBlocks;

    std::set<std::string> invariants;
    std::map<std::string, induction_variable_t> simpleInductionVariables;
    std::map<std::string, induction_variable_t> inductionVariables;
};

#endif