#ifndef LIVENESS_INFO_H__
#define LIVENESS_INFO_H__

#include <map>
#include <3ac.h>
#include <symbol_table.h>

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
    LivenessInfoTable();

    liveness_info_t get(const TID instructionID) const;
    void insert(const TID instructionID, const liveness_info_t &info);
private:
    std::map<TID, liveness_info_t> livenessMap;
};

#endif