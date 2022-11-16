#ifndef NATURAL_LOOP_H__
#define NATURAL_LOOP_H__

#include <optimizer/basic_block.h>
#include <optimizer/reach.h>

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
    NaturalLoop(BBP header, BBP footer, const Reach *reach);

    void findInvariants();

    bool isOperandInvariant(
        const std::string &operand,
        const BBP bb,
        const std::set<std::string> &invariants,
        const std::set<std::string> &outsideDeclarations
    ) const;

    /** 
     * A simple loop has its header as a predecessor and successor of the 
     * footer. If the loop is an outer loop in a loop nesting, then the header 
     * has the footer as a predecessor and the header has the inner loop header
     * as a successor.
     */
    bool isSimpleLoop() const;

    const BBP getHeader() const;
    const BBP getFooter() const;
private:

    BBP header;
    BBP footer;
    const Reach *reach;

    std::set<tac_line_t> invariants;
};

#endif