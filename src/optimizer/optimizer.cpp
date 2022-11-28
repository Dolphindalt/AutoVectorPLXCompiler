#include <optimizer/optimizer.h>

#include <cstdio>

Optimizer::Optimizer(std::vector<tac_line_t> &instructions) 
: preprocessor(Preprocessor(instructions)), blocker(Blocker(instructions)), 
    graphs(this->blocker.getBlockSet()) {}

BlockSet &Optimizer::getBlocks() {
    return this->blocker.getBlockSet();
}
