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
    static unsigned int functionCount;
    static TIDSet globalVarDefinitions;

    BasicBlock();
    BasicBlock(
        const unsigned int newMajorId, 
        const BBP copy
    );
    virtual ~BasicBlock();

    void insertInstruction(const tac_line_t instruction);
    void insertInstructions(
        std::vector<tac_line_t> instructions, const bool atEnd
    );
    void removeInstruction(const tac_line_t instruction);

    void insertPredecessor(BBP block);
    void insertPredecessors(std::vector<BBP> predecessors);
    void clearPredecessors();
    void removePredecessor(BBP block);

    void insertSuccessor(BBP block);
    void insertSuccessors(std::vector<BBP> successors);
    void clearSuccessors();
    void removeSuccessor(BBP block);

    std::vector<tac_line_t> &getInstructions();
    const std::vector<BBP> &getSuccessors();
    const std::vector<BBP> &getPredecessors();

    unsigned int getID() const;
    unsigned int getMinorId() const;
    void setMinorId(const unsigned int mid);
    bool getHasProcedureCall() const;
    bool getHasEnterProcedure() const;
    bool getHasExitProcedure() const;
    bool blockEndsWithUnconditionalJump() const;
    bool changesControlAtEnd() const;
    TIDSet getGenSet() const;
    TIDSet getKillSet() const;
    const std::map<std::string, std::vector<tac_line_t>> &getDefChain() const;
    const std::map<std::string, std::vector<tac_line_t>> &getUseChain() const;
    const tac_line_t &getFirstLabel() const;

    void computeGenAndKillSets();

    std::string to_string() const;
private:
    static unsigned int basicBlockIdGenerator;
    static unsigned int minorIdGenerator;

    unsigned int id;
    unsigned int minorId;
    bool hasProcedureCall;
    bool hasEnterProcedure;
    bool hasExitProcedure;
    bool controlChangesAtEnd;
    TIDSet localVariableDefinitions;

    std::vector<tac_line_t> instructions;
    std::vector<BBP> successors;
    std::vector<BBP> predecessors;

    TIDSet generated;
    TIDSet killed;

    std::set<std::string> variableAssignments;
    std::map<std::string, std::vector<tac_line_t>> defChain;
    std::map<std::string, std::vector<tac_line_t>> useChain;
};

#endif