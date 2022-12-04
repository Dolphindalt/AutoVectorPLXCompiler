#include <optimizer/basic_block.h>

#include <algorithm>

void BasicBlock::resetGlobalState() {
    BasicBlock::functionCount = 0;
    BasicBlock::globalVarDefinitions.clear();
    BasicBlock::basicBlockIdGenerator = 0;
    BasicBlock::minorIdGenerator = 0;
}

unsigned int BasicBlock::functionCount = 0;

std::set<tac_line_t, decltype(set_id_cmp)> BasicBlock::globalVarDefinitions;

BasicBlock::BasicBlock() 
    : id(BasicBlock::basicBlockIdGenerator++), minorId(0), 
    hasProcedureCall(false), hasEnterProcedure(false), hasExitProcedure(false),
    controlChangesAtEnd(false), 
    localVariableDefinitions(BasicBlock::globalVarDefinitions) {}

BasicBlock::BasicBlock(
    const unsigned int newMajorId, 
    const BBP copy
) : id(newMajorId), minorId(BasicBlock::minorIdGenerator++), 
    hasProcedureCall(copy->hasProcedureCall),
    hasEnterProcedure(copy->hasEnterProcedure), 
    hasExitProcedure(copy->hasExitProcedure), 
    controlChangesAtEnd(copy->controlChangesAtEnd), 
    localVariableDefinitions(copy->localVariableDefinitions),
    instructions(copy->instructions),
    successors(copy->successors),
    predecessors(copy->predecessors),
    generated(copy->generated),
    killed(copy->killed),
    variableAssignments(copy->variableAssignments),
    defChain(copy->defChain),
    useChain(copy->useChain) {
        for (tac_line_t &inst : this->instructions) {
            inst.bid = tac_line_t::bid_gen++;
        }
    }

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

    if (instruction.argument1 != "") {
        if (!this->useChain.count(instruction.argument1)) {
            this->useChain.insert(
                std::make_pair(instruction.argument1, std::vector<tac_line_t>())
            );
        }
        this->useChain.at(instruction.argument1).push_back(instruction);
    }

    if (instruction.argument2 != "") {
        if (!this->useChain.count(instruction.argument2)) {
            this->useChain.insert(
                std::make_pair(instruction.argument2, std::vector<tac_line_t>())
            );
        }
        this->useChain.at(instruction.argument2).push_back(instruction);
    }

    this->instructions.push_back(instruction);
}

void BasicBlock::insertInstructions(
    std::vector<tac_line_t> instructions, const bool atEnd
) {
    if (atEnd) {
        this->instructions.insert(this->instructions.end(),
            instructions.begin(), instructions.end());
    } else {
        instructions.insert(instructions.end(),
            this->instructions.begin(), this->instructions.end());
        this->instructions = instructions;
    }
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

void BasicBlock::insertPredecessors(std::vector<BBP> iPreds) {
    this->predecessors.insert(this->predecessors.end(),
            iPreds.begin(), iPreds.end());
}

void BasicBlock::clearPredecessors() {
    this->predecessors.clear();
}

void BasicBlock::removePredecessor(BBP block) {
    this->predecessors.erase(
        std::find(this->predecessors.begin(), this->predecessors.end(), block)
    );
}

void BasicBlock::insertSuccessor(BBP block) {
    this->successors.push_back(block);
}

void BasicBlock::insertSuccessors(std::vector<BBP> iSuccs) {
    this->successors.insert(this->successors.end(),
            iSuccs.begin(), iSuccs.end());
}

void BasicBlock::clearSuccessors() {
    this->successors.clear();
}

void BasicBlock::removeSuccessor(BBP block) {
    this->successors.erase(
        std::find(this->successors.begin(), this->successors.end(), block)
    );
}

std::vector<tac_line_t> &BasicBlock::getInstructions() {
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

void BasicBlock::setMinorId(const unsigned int mid) {
    this->minorId = mid;
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

std::string BasicBlock::id_to_string() const {
    return std::to_string(this->id) + "." + std::to_string(this->minorId);
}

std::string BasicBlock::to_string() const {
    std::string label = "\"Basic Block " + this->id_to_string() + "\\n";
    std::for_each(this->instructions.begin(), this->instructions.end(), 
        [&label](tac_line_t t) {
            label += TACGenerator::tacLineToString(t) + "\\n";
        }
    );
    label += "\"";
    return this->id_to_string() + "[label=" + label + ", shape=box]";
}

unsigned int BasicBlock::basicBlockIdGenerator = 0;

unsigned int BasicBlock::minorIdGenerator = 1;