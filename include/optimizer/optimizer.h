#ifndef OPTIMIZER_H__
#define OPTIMIZER_H__

#include <vector>
#include <3ac.h>

#include <optimizer/blocker.h>
#include <optimizer/graphs.h>

class Optimizer {
public:
    Optimizer(const std::vector<tac_line_t> &instructions);
private:
    Blocker blocker;
    Graphs graphs;
};

#endif