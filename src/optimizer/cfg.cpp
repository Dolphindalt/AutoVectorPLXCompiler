#include <optimizer/cfg.h>

CFGBuilder::CFGBuilder() {}

CFGBuilder::~CFGBuilder() {
    this->entryBlock = nullptr;
}

void CFGBuilder::setName(const std::string &name) {
    this->name = name;
}

void CFGBuilder::insert(BBP block) {
    if (entryBlock == nullptr) {
        this->entryBlock = block;
    }

    this->blocks.insert(block);
    block = nullptr;
}

CFG CFGBuilder::build() {
    return CFG(this->name, this->blocks, this->entryBlock);
}

CFG::CFG(std::string &name, BlockSet &blocks, BBP entry) : name(name), 
blocks(blocks), entryBlock(entry) {}