#ifndef CFG_H__
#define CFG_H__

#include <optimizer/basic_block.h>
#include <optimizer/block_types.h>

#include <string>

class CFG;

class CFGBuilder {
public:
    CFGBuilder();
    virtual ~CFGBuilder();

    void setName(const std::string &name);
    void insert(BBP block);

    CFG build();
private:
    std::string name = "entry";
    BlockSet blocks;
    BBP entryBlock = nullptr;
};

class CFG {
public:
    CFG() = delete;
    CFG(std::string &name, BlockSet &blocks, BBP entry);
private:
    std::string name;
    BlockSet blocks;
    BBP entryBlock;
};

#endif