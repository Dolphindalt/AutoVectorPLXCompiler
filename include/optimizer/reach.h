/**
 * This module performs reaching definition analysis. Reaching definition 
 * analysis determines what variable definitions reach which basic blocks.
 * 
 * @file reach.h
 * @author Dalton Caron
*/
#ifndef REACH_H__
#define REACH_H__

#include <optimizer/basic_block.h>
#include <3ac.h>
#include <map>
#include <string>

class CFG;

/**
 * Computes and stores the results of the reaching definition analysis.
 */
class Reach {
public:
    /** Default constructor for stack initialization. */
    Reach();

    /**
     * Performs the reaching definition analysis on the provided CFG.
     * 
     * @param cfg CFG to perform the reaching analysis upon.
    */
    Reach(CFG *cfg);

    /**
     * Returns a set of variables that are flowing into the provided basic 
     * block.
     * 
     * @param bb Basic block to get variable flow to.
     * @return The variables flowing into the block bb.
    */
    std::set<std::string> getVariablesIntoBlock(const BBP bb) const;

    /**
     * Returns a set of variables that are flowing out of the provided basic 
     * block.
     * 
     * @param bb Basic block to get variable flow from.
     * @return The variables flowing out of the block bb.
    */
    std::set<std::string> getVariablesOutOfBlock(const BBP bb) const;

    /** @return A table of input and output flows for blocks in the CFG. */
    std::string to_string() const;
private:
    void worklistReaching();

    // All varibles that are available when coming out of a block.
    std::map<BBP, std::set<std::string>> out;
    // All variables that are available when coming into a block.
    std::map<BBP, std::set<std::string>> in;
    CFG *cfg;
};

#endif