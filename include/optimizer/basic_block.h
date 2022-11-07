#ifndef BASIC_BLOCK_H__
#define BASIC_BLOCK_H__

#include <3ac.h>

class BasicBlock {
public:
    BasicBlock();
    virtual ~BasicBlock();

    void insertInstruction(const tac_line_t instruction);

    std::vector<tac_line_t> &getInstructions(); 

    unsigned int getID() const;
    std::string to_string() const;
private:
    static unsigned int basicBlockIdGenerator;

    unsigned int id;
    std::vector<tac_line_t> instructions;
};

#endif