#ifndef GROUP_H__
#define GROUP_H__

#include <string>
#include <vector>

#include <optimizer/basic_block.h>

#include <memory>

using BBP = std::shared_ptr<BasicBlock>;

/**
 * A group is a sequence of basic blocks that makes up the program entry point 
 * or the declaration of a procedure. All groups have a control flow graph 
 * that is distinct from each other due to properties of the language. Calls 
 * to fuctions may link groups and will hinder analysis if the functions 
 * cannot be inlined directly.
 */
class Group {
public:
    Group();
    virtual ~Group();

    void insertBasicBlock(const BBP block);

    void setName(const std::string &name);
    const std::string &getName() const;

    std::string to_string() const;
private:
    std::string name;
    std::vector<BBP> basicBlocks;
};

#endif