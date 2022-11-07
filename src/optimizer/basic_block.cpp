#include <optimizer/basic_block.h>

BasicBlock::BasicBlock() : id(BasicBlock::basicBlockIdGenerator++) {}

BasicBlock::~BasicBlock() {}

void BasicBlock::insertInstruction(const tac_line_t instruction) {
    this->instructions.push_back(instruction);
}

std::vector<tac_line_t> &BasicBlock::getInstructions() {
    return this->instructions;
}

unsigned int BasicBlock::getID() const {
    return this->id;
}

std::string BasicBlock::to_string() const {
    std::string result = "Basic Block " + std::to_string(this->id) + "\n";
    for (
        auto i = this->instructions.begin(); i < this->instructions.end(); i++
    ) {
        result += TACGenerator::tacLineToString(*i) + "\n";
    }
    return result;
}

unsigned int BasicBlock::basicBlockIdGenerator = 0;