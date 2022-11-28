#ifndef LIVENESS_INFO_H__
#define LIVENESS_INFO_H__

#include <map>
#include <3ac.h>
#include <symbol_table.h>
#include <optimizer/basic_block.h>

typedef struct liveness_pair {
    liveness_t liveness = CG_DEAD;
    int next_use = NO_NEXT_USE;
} liveness_pair_t;

typedef struct liveness_info {
    liveness_pair_t result;
    liveness_pair_t operand1;
    liveness_pair_t operand2;
} liveness_info_t;

class LivenessInfoTable {
public:
    LivenessInfoTable() {};
    LivenessInfoTable(BBP bb);

    liveness_info_t get(const TID instructionID) const;
    void insert(const TID instructionID, const liveness_info_t &info);
private:
    void populateSymbolTableDefaults(BBP &bb);
    void initVariableInTable(const std::string &variable);
    void computeLiveness();

    BBP bb;
    SymbolTable sym;
    std::map<TID, liveness_info_t> livenessMap;
};

#endif