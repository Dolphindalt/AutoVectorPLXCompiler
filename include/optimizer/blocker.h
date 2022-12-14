/**
 * Provides the Blocker class which is responsible for converting a sequence 
 * of three address code into basic blocks.
 * 
 * @file blocker.h
 * @author Dalton Caron
 */
#ifndef BLOCKER_H__
#define BLOCKER_H__

#include <vector>
#include <set>
#include <functional>

#include <optimizer/basic_block.h>
#include <optimizer/block_types.h>

/**
 * Responsible for grouping three address code into a sequence of basic blocks.
 */
class Blocker {
public:
    /**
     * Initializes the blocker object with the provided three address codes.
     * @param instrucions A sequence of three address codes to be blocked.
     */
    Blocker(const std::vector<tac_line_t> &instructions);

    BlockSet &getBlockSet();

    std::string to_string();
private:
    /**
     * Converts the three address codes into a set of basic blocks ordered by 
     * the occurrence of the blocks in the source code.
     * 
     * 1. Identify the leaders in the code. Leaders are instructions that come 
     * under any of the following 3 categories:
     * 
     * a. It is the first instruction. The first instruction is a leader.
     * 
     * b. The target of a conditional or an unconditional goto/jump instruction 
     * is a leader.
     * 
     * c. The instruction that immediately follows a conditional or an 
     * unconditional goto/jump instruction is a leader.
     * 
     * 2. Starting from a leader, the set of all following instructions until 
     * and not including the next leader is the basic block corresponding to the 
     * starting leader. Thus every basic block has a leader.
     * 
     * @param instructions The sequence of three address codes to be blocked.
     * @param firstBlock The first block, provided interally or externally.
     */
    BlockSet computeBasicBlocks(
        const std::vector<tac_line_t> &instructions,
        BBP &firstBlock 
    );

    /**
     * Computes the predecessors and successors of each basic block computed.
     */
    void computeControlFlowInformation();

    /**
     * @param line The instruction to evaluate.
     * @param followsJump True if the instruction is after a jump, else false.
     * @return True if the instruction is a leader, else false.
     */
    bool isInstructionLeader(
        const tac_line_t &line, const bool followsJump
    ) const;

    BBP firstBlock;
    std::map<std::string, BBP> labelLocationInBlock;
    BlockSet basicBlocks;
};

#endif