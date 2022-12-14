/**
 * This file is responible for building and maintaining the control flow graphs 
 * in memory.
 * 
 * @file graphs.h
 * @author Dalton Caron
 */
#ifndef GRAPHS_H__
#define GRAPHS_H__

#include <optimizer/cfg.h>
#include <optimizer/block_types.h>

/** Computes and holds all control flow graphs for the program. */
class Graphs {
public:
    /** 
     * Constructs all control flow graphs from the basic blocks provided. 
     * A CFG is constructed for the program entry point and all procedures. 
     * 
     * @param blocks The blocks to construct CFGs from.
     */
    Graphs(BlockSet &blocks);

    /** @return All control flow graphs. */
    const std::vector<CFG> &getAllGraphs();

    /** @return The CFG representing the program entry point. */
    CFG &getEntry();

    /** @return The CFGs representing all program procedures. */
    std::vector<CFG> &getProcedures();
private:
    void constructCFGs(BlockSet &blocks);

    CFG entry;
    std::vector<CFG> procedures;
};

#endif