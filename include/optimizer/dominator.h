/**
 * A set of utilities and classes for computing dominance between basic blocks. 
 * 
 * A node d dominates a node n if every node from the starting node to the 
 * node n must go through node d. A node dominates itself by default.
 * 
 * @file dominator.h
 * @author Dalton Caron
 */
#ifndef DOMINATOR_H__
#define DOMINATOR_H__

#include <optimizer/basic_block.h>

#include <map>
#include <vector>
#include <string>

class CFG;

class PostOrderView;

/**
 * Represents the Dominator Tree data structure used for computing dominance 
 * between basic blocks in the control flow graph.
 */
class Dominator {
public:
    /** Default contructor for intialization purposes. */
    Dominator();

    /** Contructs the dominator from a control flow graph. */
    Dominator(CFG *cfg);

    /**
     * Check the dominance of node a on node b.
     * 
     * @param a Node to dominate.
     * @param b Node to be dominated.
     * @return True if a dominates b else false.
     */
    bool dominates(const BBP a, const BBP b) const;

    /** @return A graphical representation of the dominator tree. */
    std::string to_graph();
private:
    BBP getNode(const BBP node) const;

    bool properlyDominates(const BBP a, const BBP b) const;

    void buildDominatorTree();

    BBP intersect(BBP b1, BBP b2, PostOrderView &po);

    CFG *cfg;
    std::map<BBP, BBP> iDoms;
};

#endif