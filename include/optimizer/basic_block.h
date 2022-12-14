/**
 * This file contains the functionality associated with individual basic blocks.
 * 
 * A basic block is the smallest unit of code that is interrupted without 
 * interruption by control flow changing during execution.
 * 
 * @file basic_block.h
 * @author Dalton Caron
 */
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

/**
 * Represents a unit of code that is executed without interruption.
 */
class BasicBlock {
public:
    /** 
     * Resets basic block global state for when basic blocks must be recomputed. 
     */
    static void resetGlobalState();

    /** Number of functions that exist in the global basic block collection. */
    static unsigned int functionCount;

    /** A set of all variables defined in basic blocks. */
    static TIDSet globalVarDefinitions;

    /** Constructs a basic block with a unique major ID. */
    BasicBlock();

    /**
     * BasicBlock copy constructor.
     * @param newMajorId The new major ID of the basic block.
     * @param copy The block to copy.
    */
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
    bool isNeverDefined(const std::string &variable) const;

    void computeGenAndKillSets();

    std::string id_to_string() const;
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