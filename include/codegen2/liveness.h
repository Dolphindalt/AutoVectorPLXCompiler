/**
 * Contains classes and functions pertaining to liveness analysis for 
 * context sensitive code generation.
 * 
 * @file liveness.h
 * @author Dalton Caron
 */
#ifndef LIVENESS_H__
#define LIVENESS_H__

#include <map>
#include <3ac.h>
#include <optimizer/basic_block.h>

#define NEVER_USED (-1)

using STID = signed int;

/** Liveness holds liveness and next-use informaion for a single variable. */
class Liveness {
public:
    bool isLive() const { return this->live; }
    STID getNextUse() const { return this->next_use; }
    void setLive(const bool Live) { this->live = live; }
    void setNextUse(const STID nextUse) { this->next_use = nextUse; }

    std::string to_string() const { 
        return "(" + std::to_string(this->live) + ", " + 
            std::to_string(this->next_use) + ")";
    }

    Liveness(bool live, STID next_use) : live(live), next_use(next_use) {}
private:
    bool live;
    STID next_use;
};

/** The liveness map holds liveness information for a single instruction. */
class LivenessMap {
public:
    LivenessMap();

    bool isLive(const std::string &name) const;
    bool hasNextUse(const std::string &name) const;
    STID getNextUse(const std::string &name) const;
    Liveness &getEntry(const std::string &name);
    void putEntry(const std::string &name, const Liveness &liveness);

    std::string to_string() const;
private:
    std::map<std::string, Liveness> entries;
};

/**
 * The LivenessTable contains liveness information for all instructions 
 * contained within a single basic block. This information is used to help 
 * facilitate register allocation during context sensitive code generation.
 */
class LivenessTable {
public:
    LivenessTable() = delete;

    /**
     * Computes the liveness and next-use information for all instructions 
     * that need to utilize registers within the basic block.
     */
    LivenessTable(const BBP bb);

    /**
     * Fetches the LivenessMap associated with an instrution in the basic block.
     * 
     * @param tid The instruction ID to fetch information for.
     * @return The LivenessMap containing said information.
     */
    const LivenessMap& getLivenessAndNextUse(const STID tid) const;
private:
    /**
     * Algorithm to compute liveness:
     * 1. Set all user variables as live, temp variables as dead, and all having
     * no next use in a table.
     * 2. Iterate through all 3AC statements in the block in reverse, do
     * a. Look at liveness and next-use information in the table and attach 
     * them to the current instruction.
     * b. Mark the table entries for the operands as live and set the next-use 
     * to the current instruction number. 
     */
    void computeLiveness(const BBP bb);

    void defaultTable(
        std::map<std::string, Liveness> &table, 
        const BBP bb
    ) const;

    void tryInsertTableEntry(
        std::map<std::string, Liveness> &table, 
        const std::string variable
    ) const;

    Liveness getLivenessForVariable(const std::string &name) const;

    Liveness getUserVarDefaultLiveness(const std::string &name) const;

    Liveness getTempVarDefaultLiveness(const std::string &name) const;

    void attachLivenessAndNextUse(
        std::map<std::string, Liveness> &table, 
        const STID &tid, 
        const std::string &variable
    );

    void insert(
        const STID &tid, 
        const std::string &variable, 
        const Liveness &liveness
    );

    void updateOperand(
        const STID &tid,
        std::map<std::string, Liveness> &table, 
        const std::string &variable
    ) const;

    void updateResult(
        const STID &tid,
        std::map<std::string, Liveness> &table, 
        const std::string &variable
    ) const;

    std::map<TID, LivenessMap> table;
};

#endif