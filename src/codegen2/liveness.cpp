#include <codegen2/liveness.h>

LivenessMap::LivenessMap() {}

bool LivenessMap::isLive(const std::string &name) const {
    return this->entries.at(name).isLive();
}

bool LivenessMap::hasNextUse(const std::string &name) const {
    return this->entries.at(name).getNextUse() != NEVER_USED;
}

STID LivenessMap::getNextUse(const std::string &name) const {
    return this->entries.at(name).getNextUse();
}

Liveness &LivenessMap::getEntry(const std::string &name) {
    return this->entries.at(name);
}

void LivenessMap::putEntry(const std::string &name, const Liveness &liveness) {
    this->entries.insert(std::make_pair(name, liveness));
}

std::string LivenessMap::to_string() const {
    std::string result = "";
    for (auto p : this->entries) {
        result += p.first + p.second.to_string() + " ";
    }
    return result;
}

LivenessTable::LivenessTable(const BBP bb) {
    this->computeLiveness(bb);
}

const LivenessMap& LivenessTable::getLivenessAndNextUse(const STID tid) const {
    return this->table.at(tid);
}

void LivenessTable::computeLiveness(const BBP bb) {
    std::map<std::string, Liveness> table;

    this->defaultTable(table, bb);

    const std::vector<tac_line_t> &instructions = bb->getInstructions();
    for (auto i = instructions.rbegin(); i != instructions.rend(); i++) {
        if ((*i).is_simple()) {
            const tac_line_t &inst = *i;
            this->attachLivenessAndNextUse(table, inst.bid, inst.result);
            this->attachLivenessAndNextUse(table, inst.bid, inst.argument1);
            this->attachLivenessAndNextUse(table, inst.bid, inst.argument2);
            this->updateResult(inst.bid, table, inst.result);
            this->updateOperand(inst.bid, table, inst.argument1);
            this->updateOperand(inst.bid, table, inst.argument2);
        }
    }
}

void LivenessTable::defaultTable(
    std::map<std::string, Liveness> &table, const BBP bb
) const {
    for (const tac_line_t &inst : bb->getInstructions()) {
        if (inst.is_simple()) {
            this->tryInsertTableEntry(table, inst.result);
            this->tryInsertTableEntry(table, inst.argument1);
            this->tryInsertTableEntry(table, inst.argument2);
        }
    }
}

void LivenessTable::tryInsertTableEntry(
    std::map<std::string, Liveness> &table, 
    const std::string variable
) const {
    if (variable != "") {
        table.insert(std::make_pair(
            variable,
            this->getLivenessForVariable(variable)
        ));
    }
}

Liveness LivenessTable::getLivenessForVariable(const std::string &name) const {
    if (tac_line_t::is_user_defined_var(name)) {
        return this->getUserVarDefaultLiveness(name);
    }
    return this->getTempVarDefaultLiveness(name);
}

Liveness LivenessTable::getUserVarDefaultLiveness(
    const std::string &name
) const {
    return Liveness(true, NEVER_USED);
}

Liveness LivenessTable::getTempVarDefaultLiveness(
    const std::string &name
) const {
    return Liveness(false, NEVER_USED);
}

void LivenessTable::attachLivenessAndNextUse(
    std::map<std::string, Liveness> &table, 
    const STID &tid, 
    const std::string &variable
) {
    if (variable != "") {
        this->insert(tid, variable, table.at(variable));
    }
}

void LivenessTable::insert(
    const STID &tid, 
    const std::string &variable, 
    const Liveness &liveness
) {
    if (!this->table.count(tid)) {
        this->table.insert(std::make_pair(tid, LivenessMap()));
    }

    this->table.at(tid).putEntry(variable, liveness);
}

void LivenessTable::updateOperand(
    const STID &tid,
    std::map<std::string, Liveness> &table, 
    const std::string &variable
) const {
    if (variable != "") {
        table.at(variable).setLive(true);
        table.at(variable).setNextUse(tid);
    }
}

void LivenessTable::updateResult(
    const STID &tid,
    std::map<std::string, Liveness> &table, 
    const std::string &variable
) const {
    if (variable != "") {
        table.at(variable).setLive(false);
        table.at(variable).setNextUse(NEVER_USED);
    }
}