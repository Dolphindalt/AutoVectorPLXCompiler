#ifndef BASIC_BLOCK_H__
#define BASIC_BLOCK_H__

#include <3ac.h>

#include <memory>
#include <set>

class BasicBlock;

using BBP = std::shared_ptr<BasicBlock>;

class BasicBlock {
public:
    BasicBlock();
    virtual ~BasicBlock();

    void insertInstruction(const tac_line_t instruction);
    void insertPredecessor(BBP block);
    void insertSuccessor(BBP block);

    std::vector<tac_line_t> &getInstructions(); 

    unsigned int getID() const;
    bool getHasProcedureCall() const;
    bool getHasEnterProcedure() const;
    bool getHasExitProcedure() const;
    std::string to_string() const;
private:
    static unsigned int basicBlockIdGenerator;

    unsigned int id;
    bool hasProcedureCall;
    bool hasEnterProcedure;
    bool hasExitProcedure;

    std::vector<tac_line_t> instructions;
    std::set<BBP> successors;
    std::set<BBP> predecessors;
};

#endif