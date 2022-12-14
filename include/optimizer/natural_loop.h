/**
 * Contains the class that represents a natural loop. A natural loop is a 
 * back edge in the CFG where the loop footer dominates the loop header and 
 * the loop only has a single entrance.
 * 
 * @file natural_loop.h
 * @author Dalton Caron
 */
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

    /** Default contructor for initialization purposes. */
    induction_variable() {}

    /**
     * Constructs an induction variable representation.
     * @param is_simple True if in the form X := X + C, else false.
     * @param inductionVar The name of the induction variable.
     * @param constant The name of the constant variable.
     * @param previous The previous induction variable. Is only populated when 
     * the variable is not simple. Variables that are not simple are linked 
     * lists of induction variable objects.
     */
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
    /**
     * Construct a representation of a natural loop that includes the back edge 
     * and other important meta data computed prior.
     * 
     * @param header Loop header.
     * @param footer Loop footer.
     * @param reach Reaching definition analysis results.
     * @param dom Dominator tree of the CFG the loop is member of.
     * @param allBlocks A reference to the collection of all basic blocks.
     */
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
    void forEachBBInBody(std::function<void(BBP)> action) const;

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
    bool isSimpleLoop();

    /** 
     * @param value The variable to check.
     * @return True if a variable is loop invariant, else false. 
     */
    bool isInvariant(const std::string &value) const;

    /** 
     * @param value The variable to check.
     * @return True if a variable is an induction variable, else false. 
     */
    bool isInductionVariable(const std::string &value) const;

    /** 
     * @param value The variable to check.
     * @return True if a variable is a simple induction variable, else false. 
     */
    bool isSimpleInductionVariable(const std::string &value) const;

    /**
     * @param variable The variable to check.
     * @return True if the variable is never defined in the loop, else false.
     */
    bool isNeverDefinedInLoop(const std::string &variable) const;

    /**
     * Returns the loop exit. Assumes the loop only has one exit.
     * @return The loop exit.
     */
    BBP getExit() const;

    /** @return The loop header. */
    const BBP getHeader() const;

    /** @return The loop footer. */
    const BBP getFooter() const;

    /** @return A back edge tuple representation of the loop in string form. */
    const std::string to_string() const;
private:
    void forEachBBInBodyInternal(
        BBP current, 
        std::function<void(BBP)> action,
        std::set<BBP> &visited
    ) const;

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