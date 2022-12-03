#include <optimizer/graphs.h>

#include <algorithm>

Graphs::Graphs(BlockSet &blocks) {
    this->constructCFGs(blocks);
}

CFG &Graphs::getEntry() {
    return this->entry;
}

std::vector<CFG> &Graphs::getProcedures() {
    return this->procedures;
}

void Graphs::constructCFGs(BlockSet &blocks) {
    BBP entryBlock = *blocks.begin();
    std::string entry = "entry";
    this->entry = CFG(blocks, entry, entryBlock);

    std::for_each(++blocks.begin(), blocks.end(), [&blocks, this](BBP block) {
        if (block->getHasEnterProcedure()) {
            // From the structure of the 3AC.
            std::string proc_name = block->getInstructions().at(0).argument1;
            this->procedures.push_back(CFG(blocks, proc_name, block));
        }
    });
}
