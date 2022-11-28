#include <optimizer/basic_block.h>

#include <algorithm>

unsigned int BasicBlock::functionCount = 0;

std::set<tac_line_t, decltype(set_id_cmp)> BasicBlock::globalVarDefinitions;

BasicBlock::BasicBlock() 
    : id(BasicBlock::basicBlockIdGenerator++), minorId(0), 
    hasProcedureCall(false), hasEnterProcedure(false), hasExitProcedure(false),
    controlChangesAtEnd(false), 
    localVariableDefinitions(BasicBlock::globalVarDefinitions) {}

BasicBlock::~BasicBlock() {}

void BasicBlock::insertInstruction(const tac_line_t instruction) {
    this->controlChangesAtEnd = false;
    if (tac_line_t::transfers_control(instruction)) {
        this->controlChangesAtEnd = true;
    }

    switch (instruction.operation) {
        case TAC_CALL:
            this->hasProcedureCall = true;
            break;
        case TAC_ENTER_PROC:
            this->hasEnterProcedure = true;
            BasicBlock::functionCount++;
            break;
        case TAC_EXIT_PROC:
            this->hasExitProcedure = true;
            break;
        default:
            break;
    }

    if (tac_line_t::has_result(instruction)) {
        this->globalVarDefinitions.insert(instruction);
        this->localVariableDefinitions.insert(instruction);
        this->variableAssignments.insert(instruction.result);
        if (!this->defChain.count(instruction.result)) {
            this->defChain.insert(
                std::make_pair(instruction.result, std::vector<tac_line_t>())
            );
        }
        this->defChain.at(instruction.result).push_back(instruction);
    }

    this->instructions.push_back(instruction);
}

void BasicBlock::removeInstruction(const tac_line_t instruction) {
    this->generated.erase(instruction);

    this->instructions.erase(std::remove(
        this->instructions.begin(), this->instructions.end(), instruction
    ), this->instructions.end());
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

unsigned int BasicBlock::getMinorId() const {
    return this->minorId;
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

bool BasicBlock::changesControlAtEnd() const {
    return this->controlChangesAtEnd;
}

std::set<tac_line_t, decltype(set_id_cmp)> BasicBlock::getGenSet() const {
    return this->generated;
}

std::set<tac_line_t, decltype(set_id_cmp)> BasicBlock::getKillSet() const {
    return this->killed;
}

const std::map<std::string, std::vector<tac_line_t>> &BasicBlock::getDefChain() const {
    return this->defChain;
}

const tac_line_t &BasicBlock::getFirstLabel() const {
    for (const tac_line_t &inst : this->instructions) {
        if (inst.operation == TAC_LABEL) {
            return inst;
        }
    }
    ERROR_LOGV("Failed to find label when expected");
    exit(EXIT_FAILURE);
}

void BasicBlock::computeGenAndKillSets() {
    this->killed = TIDSet(this->localVariableDefinitions);
    for (
        auto t = this->getInstructions().begin();
        t != this->getInstructions().end();
        t++
    ) {
        const tac_line_t instruction = *t;
        if (tac_line_t::has_result(instruction)) {
            this->generated.insert(instruction);
            this->killed.erase(instruction);
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