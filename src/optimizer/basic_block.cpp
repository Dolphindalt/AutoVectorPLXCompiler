#include <optimizer/basic_block.h>

#include <algorithm>

BasicBlock::BasicBlock() 
    : id(BasicBlock::basicBlockIdGenerator++), hasProcedureCall(false),
    hasEnterProcedure(false), hasExitProcedure(false) {}

BasicBlock::~BasicBlock() {}

void BasicBlock::insertInstruction(const tac_line_t instruction) {
    switch (instruction.operation) {
        case TAC_CALL:
            this->hasProcedureCall = true;
            break;
        case TAC_ENTER_PROC:
            this->hasEnterProcedure = true;
            break;
        case TAC_EXIT_PROC:
            this->hasExitProcedure = true;
            break;
        default:
            break;
    }

    this->instructions.push_back(instruction);
}

void BasicBlock::insertPredecessor(BBP block) {
    this->predecessors.insert(block);
}

void BasicBlock::insertSuccessor(BBP block) {
    this->successors.insert(block);
}

std::vector<tac_line_t> &BasicBlock::getInstructions() {
    return this->instructions;
}

unsigned int BasicBlock::getID() const {
    return this->id;
}

bool BasicBlock::getHasProcedureCall() const {
    return this->hasProcedureCall;
}

bool BasicBlock::getHasEnterProcedure() const {
    return this->hasEnterProcedure;
}

bool BasicBlock::getHasExitProcedure() const {
    return this->hasExitProcedure;
}

std::string BasicBlock::to_string() const {
    std::string result = "Basic Block " + std::to_string(this->id) + "\n";
    
    result += "Predecessors: ";
    std::for_each(this->predecessors.begin(), this->predecessors.end(), 
        [&result](BBP bb) {
            result += std::to_string(bb->getID()) + " "; 
        }
    );
    result += "\n";

    std::for_each(this->instructions.begin(), this->instructions.end(), 
        [&result](tac_line_t t) {
            result += TACGenerator::tacLineToString(t) + "\n";
        }
    );
    return result;
}

unsigned int BasicBlock::basicBlockIdGenerator = 0;