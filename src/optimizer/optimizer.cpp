#include <optimizer/optimizer.h>

Optimizer::Optimizer(std::vector<tac_line_t> &instructions) {
    std::map<std::string, unsigned int> labelToBlockMap;
    this->populateGroups(
        instructions.begin(), instructions.end(), labelToBlockMap
    );
    
    this->replaceLabelsWithTargetBlocks(labelToBlockMap);
}

Optimizer::~Optimizer() {}

std::string Optimizer::to_string() const {
    std::string result = "";
    
    for (auto g = this->groups.begin(); g != this->groups.end(); g++) {
        result += g->to_string();
    }

    return result;
}

void Optimizer::populateGroups(
    std::vector<tac_line_t>::iterator start,
    const std::vector<tac_line_t>::iterator end,
    std::map<std::string, unsigned int> &labelToBlockMap
) {

    Group group;

    // The first instruction is always a leader.
    const tac_line_t &first_instruction = *start;

    BBP incumbentBlock = std::make_shared<BasicBlock>();
    this->allBasicBlocks.push_back(incumbentBlock);

    incumbentBlock->insertInstruction(first_instruction);
    start++;

    // If this is a function and not the entry point, the first instruction 
    // will be a label.
    if (first_instruction.operation == TAC_LABEL) {
        labelToBlockMap[first_instruction.argument1] = incumbentBlock->getID();
        group.setName(first_instruction.argument1);
    }

    bool previousWasJump = false;

    for (; start != end; start++) {

        const tac_line_t &instruction = *start;

        // If the instruction starts a procedure, we will skip the procedure
        // and continue forward. The procedure will be read as its own group.
        // The TAC_ENTER_PROC and TAC_EXIT_PROC are markers to show the 
        // declaration of the procedure. Now that groups denote procedures,
        // we can discard these markers.
        if (instruction.operation == TAC_ENTER_PROC) {

            // Discard the TAC_ENTER_PROC line.
            start++;     

            // Mark the start of the procedure.
            std::vector<tac_line_t>::iterator procStart = start;

            // Skip all the procedure contents.
            while (start->operation != TAC_EXIT_PROC) {
                start++;
            }

            // Mark the end of the procedure.
            std::vector<tac_line_t>::iterator procEnd = start;

            // Read the procedure contents as a group.
            this->populateGroups(procStart, procEnd, labelToBlockMap);

            // Continue skips the TAC_EXIT_PROC.
            continue;
        }

        // An instruction that is a label is a leader and an instruction that
        // follow a unconditional or conditional jump is a leader.
        if (instruction.operation == TAC_LABEL || previousWasJump) {
            previousWasJump = false;
            // The current instruction is a leader.
            group.insertBasicBlock(incumbentBlock);

            incumbentBlock = std::make_shared<BasicBlock>();
            this->allBasicBlocks.push_back(incumbentBlock);

            if (instruction.operation == TAC_LABEL) {
                labelToBlockMap[instruction.argument1] = incumbentBlock->getID();
            }
        }
        incumbentBlock->insertInstruction(instruction);

        // If this instruction is a jump, then the next instruction is the new
        // basic block leader.
        previousWasJump = tac_line_t::transfers_control(instruction);

    }
    group.insertBasicBlock(incumbentBlock);

    this->groups.push_back(group);

    incumbentBlock = nullptr;
}

void Optimizer::replaceLabelsWithTargetBlocks(
    const std::map<std::string, unsigned int> &labelToBlockMap
) {
    for (
        auto b = this->allBasicBlocks.begin(); 
        b != this->allBasicBlocks.end(); 
        b++
    ) {
        BBP bb = (*b);

        for (
            auto i = bb->getInstructions().begin();
            i != bb->getInstructions().end();
            i++
        ) {
            // If the instructions transfers control, we want to change the 
            // label into the block that gets the control.
            tac_line_t &instruction = (*i);
            if (tac_line_t::transfers_control(instruction)) {
                instruction.argument1 = 
                    std::to_string(labelToBlockMap.at(instruction.argument1));
            }
        }

        bb = nullptr;
    }
}
