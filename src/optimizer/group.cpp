#include <optimizer/group.h>

Group::Group() : name("entry") {}

Group::~Group() {}

void Group::insertBasicBlock(const BBP block) {
    this->basicBlocks.push_back(block);
}

void Group::setName(const std::string &name) {
    this->name = name;
}

const std::string &Group::getName() const {
    return this->name;
}

std::string Group::to_string() const {
    std::string result = "Group " + this->name + "\n";
    for (
        auto i = this->basicBlocks.begin(); i != this->basicBlocks.end(); i++
    ) {
        result += (*i)->to_string();
    }
    return result;
}