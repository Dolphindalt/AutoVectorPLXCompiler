#ifndef BLOCKER_H__
#define BLOCKER_H__

#include <vector>
#include <set>
#include <functional>

#include <optimizer/basic_block.h>
#include <optimizer/block_types.h>

class Blocker {
public:
    Blocker(const std::vector<tac_line_t> &instructions);

    BlockSet computeBasicBlocks(
        const std::vector<tac_line_t> &instructions,
        BBP &firstBlock 
    );

    void computeControlFlowInformation();

    BlockSet &getBlockSet();

    std::string to_string();
private:
    bool isInstructionLeader(
        const tac_line_t &line, const bool followsJump
    ) const;

    BBP firstBlock;
    std::map<std::string, BBP> labelLocationInBlock;
    BlockSet basicBlocks;
};

#endif