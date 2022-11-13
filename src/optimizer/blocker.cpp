#include <optimizer/blocker.h>

#include <logging.h>
#include <3ac.h>
#include <map>
#include <memory.h>
#include <functional>

Blocker::Blocker(const std::vector<tac_line_t> &instructions) {
    this->basicBlocks = this->computeBasicBlocks(instructions, firstBlock);
    this->computeControlFlowInformation();
}

/**
 * 1. Identify the leaders in the code. Leaders are instructions that come under 
 * any of the following 3 categories:
 * 
 * a. It is the first instruction. The first instruction is a leader.
 * 
 * b. The target of a conditional or an unconditional goto/jump instruction is 
 * a leader.
 * 
 * c. The instruction that immediately follows a conditional or an unconditional
 * goto/jump instruction is a leader.
 * 
 * 2. Starting from a leader, the set of all following instructions until and 
 * not including the next leader is the basic block corresponding to the 
 * starting leader. Thus every basic block has a leader.
 */
BlockSet Blocker::computeBasicBlocks(
    const std::vector<tac_line_t> &instructions,
    BBP &firstBlock 
) {
    if (instructions.size() == 0) {
        return BlockSet();
    }

    BlockSet resultSet;

    BBP block = std::make_shared<BasicBlock>();
    firstBlock = block;
    tac_line_t leader = *instructions.begin();
    block->insertInstruction(leader);
    bool followsJump = false;

    for (auto i = instructions.begin() + 1; i != instructions.end(); i++) {
        const tac_line_t current_instruction = *i;

        // If the current instruction is a leader, it starts a new basic block.
        if (this->isInstructionLeader(current_instruction, followsJump)) {
            resultSet.insert(block);
            block = std::make_shared<BasicBlock>();
        }

        block->insertInstruction(current_instruction);

        // Store information about relationships between instructions and 
        // basic blocks.
        if (current_instruction.operation == TAC_LABEL) {
            this->labelLocationInBlock
                .insert(std::make_pair(current_instruction.argument1, block));
        }

        // Instructions that transfer control will flag the next instruction
        // to be a leader.
        followsJump = false;
        if (tac_line_t::transfers_control(current_instruction) 
            || current_instruction.operation == TAC_EXIT_PROC) {
                followsJump = true;
        }
    }
    
    resultSet.insert(block);

    block = nullptr;

    return resultSet;
}

/**
 * This function establishes the successors and predecessors of each block. A
 * block that is not the boundary between the entry code and a procedure 
 * declaration are a successor/predecessor pair. Any block that jumps to 
 * another shares a successor/predecssor pair.
 */
void Blocker::computeControlFlowInformation() {
    auto i = this->basicBlocks.begin();
    BBP savedPreviousBlock = nullptr;
    BBP previousBlock = *i;
    for (++i; i != this->basicBlocks.end(); i++) {

        BBP currentBlock = *i;

        // Deals with blocks being neighbors or cut up by procedure
        // declarations.
        if (currentBlock->getHasEnterProcedure()) {
            savedPreviousBlock = previousBlock;
            previousBlock = *i;
        } else if (currentBlock->getHasExitProcedure()) {
            previousBlock->insertSuccessor(currentBlock);
            currentBlock->insertPredecessor(previousBlock);
            previousBlock = savedPreviousBlock;
            savedPreviousBlock = nullptr;
        } else {
            previousBlock->insertSuccessor(currentBlock);
            currentBlock->insertPredecessor(previousBlock);
            previousBlock = currentBlock;
        }

        // Now to deal with jumps.
        for (
            auto t = currentBlock->getInstructions().begin();
            t != currentBlock->getInstructions().end();
            t++
        ) {
            const tac_line_t &tac = *t;
            // Function calls do transfer control but are ignored for this
            // analysis.
            if (
                tac_line_t::transfers_control(tac) && tac.operation != TAC_CALL
            ) {
                BBP controlGoesTo = this->labelLocationInBlock.at(tac.argument1);
                controlGoesTo->insertSuccessor(currentBlock);
                currentBlock->insertPredecessor(controlGoesTo);
                controlGoesTo = nullptr;
            }
        }

    }

    previousBlock = nullptr;
    savedPreviousBlock = nullptr;
}

std::string Blocker::to_string() {
    std::string result = "";
    std::for_each(this->basicBlocks.begin(), this->basicBlocks.end(),
        [&result](BBP block) {
            result += block->to_string();
        }
    );
    return result;
}

bool Blocker::isInstructionLeader(
    const tac_line_t &line, const bool followsJump
) const {
    return followsJump || line.operation == TAC_LABEL || 
        line.operation == TAC_ENTER_PROC || line.operation == TAC_EXIT_PROC;
}