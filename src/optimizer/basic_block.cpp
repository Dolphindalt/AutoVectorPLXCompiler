#include <optimizer/basic_block.h>

#include <algorithm>

std::set<TID> BasicBlock::varDefinitions;

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

    if (tac_line_t::has_result(instruction)) {
        this->varDefinitions.insert(instruction.bid);
    }

    this->instructions.push_back(instruction);
}

void BasicBlock::insertPredecessor(BBP block) {
    this->predecessors.push_back(block);
}

void BasicBlock::insertSuccessor(BBP block) {
    this->successors.push_back(block);
}

const std::vector<tac_line_t> &BasicBlock::getInstructions() {
    return this->instructions;
}

const std::vector<BBP> &BasicBlock::getSuccessors() {
    return this->successors;
}

const std::vector<BBP> &BasicBlock::getPredecessors() {
    return this->predecessors;
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

bool BasicBlock::blockEndsWithUnconditionalJump() const {
    return this->instructions.size() >= 1 && 
        this->instructions.back().operation == TAC_UNCOND_JMP;
}

std::set<TID> BasicBlock::getGenSet() const {
    return this->generated;
}

std::set<TID> BasicBlock::getKillSet() const {
    return this->killed;
}

void BasicBlock::computeGenAndKillSets() {
    for (
        auto t = this->getInstructions().begin();
        t != this->getInstructions().end();
        t++
    ) {
        const tac_line_t instruction = *t;
        if (tac_line_t::has_result(instruction)) {
            this->generated.insert(instruction.bid);
            this->killed = this->varDefinitions;
            this->killed.erase(instruction.bid);
        }
    }
}

std::string BasicBlock::to_string() const {
    std::string label = "\"Basic Block " + std::to_string(this->id) + "\\n";
    std::for_each(this->instructions.begin(), this->instructions.end(), 
        [&label](tac_line_t t) {
            label += TACGenerator::tacLineToString(t) + "\\n";
        }
    );
    label += "\"";
    return std::to_string(this->getID()) + "[label=" + label + ", shape=box]";
}

unsigned int BasicBlock::basicBlockIdGenerator = 0;