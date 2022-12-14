/**
 * This is the main entry point into the optimizier compilation phase.
 * 
 * @file optimizer.h
 * @author Dalton Caron
 */
#ifndef OPTIMIZER_H__
#define OPTIMIZER_H__

#include <vector>
#include <3ac.h>

#include <optimizer/blocker.h>
#include <optimizer/graphs.h>
#include <optimizer/preprocessing.h>

/**
 * Represents the compiler optimizer entry point. Responsible for running 
 * the requested optimizations.
 */
class Optimizer {
public:
    /**
     * Performs specifically machine independent optimization on the sequence 
     * of instructions generated from the compiler frontend.
     */
    Optimizer(std::vector<tac_line_t> &instructions);

    /**
     * Gets the optimized code in the form of a sequnece of basic blocks.
     * @return The set of optimized blocks.
     */
    BlockSet &getBlocks();
private:
    Preprocessor preprocessor;
    Blocker blocker;
    Graphs graphs;
};

#endif