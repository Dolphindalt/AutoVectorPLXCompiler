#ifndef LOOP_NEST_H__
#define LOOP_NEST_H__

#include <optimizer/natural_loop.h>
#include <vector>

class LoopNest {
public:
    LoopNest();

    bool canVectorize() const;
    std::vector<NaturalLoop> getNest();
private:
    std::vector<NaturalLoop> nest;
};

#endif