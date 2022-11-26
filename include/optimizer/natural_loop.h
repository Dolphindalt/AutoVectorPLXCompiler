#ifndef NATURAL_LOOP_H__
#define NATURAL_LOOP_H__

#include <optimizer/basic_block.h>
#include <optimizer/reach.h>
#include <optimizer/dominator.h>

#include <functional>
#include <string>

using address = std::string;

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
        BBP header, BBP footer, const Reach *reach, const Dominator *dom
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
     */
    void findInductionVariables();

    /** 
     * A simple loop has its header as a predecessor and successor of the 
     * footer. If the loop is an outer loop in a loop nesting, then the header 
     * has the footer as a predecessor and the header has the inner loop header
     * as a successor.
     */
    bool isSimpleLoop() const;

    bool isInvariant(const std::string &value) const;

    bool isInductionVariable(const std::string &value) const;

    bool isSimpleInductionVariable(const std::string &value) const;

    const BBP getHeader() const;
    const BBP getFooter() const;
private:
    BBP findNextDommed(BBP bb) const;

    BBP header;
    BBP footer;
    const Reach *reach;
    const Dominator *dom;

    std::set<std::string> invariants;
    std::set<std::string> simpleInductionVariables;
    std::set<std::string> inductionVariables;
};

#endif