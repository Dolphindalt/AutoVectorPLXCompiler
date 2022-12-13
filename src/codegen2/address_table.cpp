#include <codegen2/address_table.h>

#include <assertions.h>

static std::string locationTypeToString(const location_type_t loc) {
    switch (loc) {
        case LT_REGISTER: return "LT_REGISTER";
        case LT_MEMORY_GLOBAL: return "LT_MEMORY_GLOBAL";
        case LT_MEMORY_STACK: return "LT_MEMORY_STACK";
        case LT_IMMEDIATE: return "LT_IMMEDIATE";
        default:
            return "invalid location type";
    }
}

static RegPtr dummyRegister = std::make_shared<Register>("dummy");

std::string Location::largeImmediatesPrefix = "LIM";

std::string Location::getLargeImmediateName(const std::string &previousName) {
    return Location::largeImmediatesPrefix + previousName;
}

Location::Location() : type(LT_DUMMY), stackOffset(0), reg(dummyRegister) {}

Location::Location(const location_type_t &type) : type(type), stackOffset(0),
reg(dummyRegister) {}

const location_type_t &Location::getType() const {
    return this->type;
}

bool Location::inMemory() const {
    return this->type == LT_MEMORY_STACK || this->type == LT_MEMORY_GLOBAL;
}

bool Location::inRegister() const {
    return this->type == LT_REGISTER;
}

bool Location::isImmediate() const {
    return this->type == LT_IMMEDIATE;
}

Location &Location::setStack(const signed int offset) {
    this->stackOffset = offset;
    return *this;
}

Location &Location::setReg(const RegPtr reg) {
    this->reg = reg;
    return *this;
}

signed int Location::getStackOffset() const {
    return this->stackOffset;
}

RegPtr Location::getRegister() const {
    return this->reg;
}

const std::string &Location::getImmValueOrGlobal() const {
    return this->immValueOrGlobal;
}

Location Location::setImmValueOrGlobal(const std::string &value) {
    this->immValueOrGlobal = value;
    return *this;
}

bool Location::isRegAddress() const {
    return this->regIsAddress;
}

Location Location::setIsRegAddress(const bool value) {
    this->regIsAddress = value;
    return *this;
}

std::string Location::address(const bool forceRegValue) const {
    switch (this->type) {
        case LT_REGISTER:
            if (this->regIsAddress || forceRegValue) {
                return this->reg->getNameAsMemory();
            }
            return this->reg->getName();
        case LT_MEMORY_GLOBAL:
            return this->immValueOrGlobal + "(\%rip)";
        case LT_MEMORY_STACK:
            return stackOffset + "(\%rbp)";
        case LT_IMMEDIATE:
            return "$" + this->immValueOrGlobal;
        default:
            WARNING_LOGV("invalid address mode");
            exit(EXIT_FAILURE);
    }
}

std::string Location::to_string() const {
    return locationTypeToString(this->type);
}

AddressTable::AddressTable() {}

Location &AddressTable::getLocation(const std::string &variable) {
    ASSERT(this->contains(variable));
    return this->table.at(variable);
}

bool AddressTable::varIsInRegOrMem(const std::string &variable) const {
    return this->table.count(variable) > 0;
}

bool AddressTable::isInRegister(const std::string &variable) const {
    return this->table.count(variable) > 0 
        && this->table.at(variable).getType() == LT_REGISTER;
}

RegPtr AddressTable::getRegister(const std::string &variable) {
    ASSERT(this->isInRegister(variable));
    return this->table.at(variable).getRegister();
}

void AddressTable::insert(
    const std::string &variable, 
    const Location &location
) {
    this->table[variable] = location;
}

void AddressTable::insertIfLiteral(
    const std::string &variable, 
    const std::shared_ptr<SymbolTable> &table
) {
    unsigned int level;
    st_entry_t entry;
    const bool success = table->lookup(variable, &level, &entry);

    if (success && entry.entry_type == ST_LITERAL) {
        const Location &constLocation = Location(LT_IMMEDIATE)
            .setImmValueOrGlobal(std::to_string(entry.literal.value.int_value));
        this->insert(variable, constLocation);
    } 
}

bool AddressTable::contains(const std::string &variable) const {
    return this->table.count(variable) > 0;
}

std::vector<std::pair<std::string, Location>> 
AddressTable::getValueAndLocationInRegisters() {
    std::vector<std::pair<std::string, Location>> result;
    for (auto kv : table) {
        if (kv.second.inRegister()) {
            result.push_back(kv);
        }
    }
    return result;
}

void AddressTable::clearRegisters() {
    for (auto i = this->table.cbegin(); i != this->table.cend(); ) {
        if (i->second.inRegister()) {
            i = this->table.erase(i);
        } else {
            i++;
        }
    }
}

std::string AddressTable::to_string() const {
    std::string result = "";
    for (auto i = this->table.begin(); i != this->table.end(); i++) {
        result += "(" + i->first + ", " + i->second.to_string() + ") ";
    }
    return result;
}
