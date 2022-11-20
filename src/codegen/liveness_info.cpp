#include <codegen/liveness_info.h>

LivenessInfoTable::LivenessInfoTable() {}

liveness_info_t LivenessInfoTable::get(const TID instructionID) const {
    return this->livenessMap.at(instructionID);
}

void LivenessInfoTable::insert(const TID tid, const liveness_info_t &info) {
    this->livenessMap.insert(std::make_pair(tid, info));
}