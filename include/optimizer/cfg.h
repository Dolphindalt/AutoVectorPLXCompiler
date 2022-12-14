/**
 * CFG stands for control flow graph and is a graph where the nodes are basic 
 * blocks and the directed edges are the transfer of control from one basic 
 * block to another.
 * 
 * @file cfg.h
 * @author Dalton Caron
 */
#ifndef CFG_H__
#define CFG_H__

#include <optimizer/basic_block.h>
#include <optimizer/block_types.h>
#include <optimizer/dominator.h>
#include <optimizer/natural_loop.h>
#include <optimizer/reach.h>

#include <string>
#include <functional>
#include <constants.h>

/**
 * The Control Flow Graph is a graph with basic block nodes and directed 
 * edges that indicate control changing from one basic block to another 
 * basic block.
 */
class CFG {
public:
    /** Default constructor to initialize uncomputed graphs. */
    CFG();

    /**
     * Computes the control flow graph.
     * @param allBlocks The blocks to be contained within the CFG.
     * @param name A name to identify the CFG when printing.
     * @param firstBlock The entry point to the CFG.
    */
    CFG(BlockSet &allBlocks, std::string &name, BBP firstBlock);

    /**
     * Performs a post order traversal on the CFG, performing an action on each 
     * basic block encountered.
     * @param action The function to perform on the basic blocks.
     */
    void performPostorderTraversal(std::function<void(BBP block)> action) const;

    /** @return The entry point basic block to the CFG. */
    BBP getEntryBlock() const;

    /** @return The name of the CFG. */
    const std::string getName() const;

    /** @return A Graphviz dot representation of the graph in stirng form. */
    std::string to_graph() const;
private:
    void computePostorderTraversalInternal(
        BBP node, std::set<BBP> &visited, std::function<void(BBP block)> action
    ) const;

    std::set<std::pair<BBP, BBP>> computeBackwardsEdges();

    std::vector<NaturalLoop> computeNaturalLoops(
        std::set<std::pair<BBP, BBP>> backedges,
        BlockSet &allBlocks
    ) const;

    std::string name;
    BBP entryBlock;
    Dominator dominator;
    Reach reach;
};

/**
 * Represents a post order traversal of a control flow graph.
 */
class PostOrderView {
public:
    PostOrderView(CFG *cfg);

    bool comparator(BBP lhs, BBP rhs) const; 

    std::vector<BBP> &getPO();
private:
    unsigned int idGenerator = 0;
    std::map<BBP, unsigned int> poIndexMap;
    std::vector<BBP> postorder;
};

#endif