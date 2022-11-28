#ifndef OPTIMIZER_H__
#define OPTIMIZER_H__

#include <vector>
#include <3ac.h>

#include <optimizer/blocker.h>
#include <optimizer/graphs.h>
#include <optimizer/preprocessing.h>

class Optimizer {
public:
    Optimizer(std::vector<tac_line_t> &instructions);

    BlockSet &getBlocks();
private:
    Preprocessor preprocessor;
    Blocker blocker;
    Graphs graphs;
};

#endif