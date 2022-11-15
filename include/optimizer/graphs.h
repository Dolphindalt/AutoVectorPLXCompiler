#ifndef GRAPHS_H__
#define GRAPHS_H__

#include <optimizer/cfg.h>
#include <optimizer/block_types.h>

class Graphs {
public:
    Graphs(BlockSet &blocks);

    const std::vector<CFG> &getAllGraphs();
    CFG &getEntry();
    std::vector<CFG> &getProcedures();
private:
    void constructCFGs(BlockSet &blocks);

    CFG entry;
    std::vector<CFG> procedures;
};

#endif