#ifndef STRIP_PROFILE_H__
#define STRIP_PROFILE_H__

#include <optimizer/basic_block.h>
#include <vector>

class StripProfile {
public:
    StripProfile(
        BBP bb, 
        const unsigned int factor, 
        std::vector<tac_line_t> &iteration,
        const bool vectorize
    );
    void unroll();
private:
    void insertVectorInstructions();

    BBP block;
    unsigned int factor;
    std::vector<tac_line_t> &iteration;
    bool vectorize;
};

#endif