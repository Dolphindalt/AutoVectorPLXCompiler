#ifndef BASIC_BLOCK_H__
#define BASIC_BLOCK_H__

#include <3ac.h>

#include <memory>
#include <set>

class BasicBlock;

using BBP = std::shared_ptr<BasicBlock>;

auto inline set_id_cmp = [](tac_line_t a, tac_line_t b) { 
    return a.bid < b.bid; 
};

using TIDSet = std::set<tac_line_t, decltype(set_id_cmp)>;

class BasicBlock {
public:
    static TIDSet varDefinitions;

    BasicBlock();
    virtual ~BasicBlock();

    void insertInstruction(const tac_line_t instruction);
    void removeInstruction(const tac_line_t instruction);
    void insertPredecessor(BBP block);
    void insertSuccessor(BBP block);

    const std::vector<tac_line_t> &getInstructions(); 
    const std::vector<BBP> &getSuccessors();
    const std::vector<BBP> &getPredecessors();

    unsigned int getID() const;
    bool getHasProcedureCall() const;
    bool getHasEnterProcedure() const;
    bool getHasExitProcedure() const;
    bool blockEndsWithUnconditionalJump() const;
    TIDSet getGenSet() const;
    TIDSet getKillSet() const;
    bool isVariableConstantInBB(const std::string &varName) const;

    void computeGenAndKillSets();

    std::string to_string() const;
private:
    static unsigned int basicBlockIdGenerator;

    unsigned int id;
    bool hasProcedureCall;
    bool hasEnterProcedure;
    bool hasExitProcedure;

    std::vector<tac_line_t> instructions;
    std::vector<BBP> successors;
    std::vector<BBP> predecessors;

    TIDSet generated;
    TIDSet killed;

    std::set<std::string> variableAssignments;
};

#endif