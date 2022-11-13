#include <optimizer/optimizer.h>

#include <cstdio>

Optimizer::Optimizer(const std::vector<tac_line_t> &instructions) 
: blocker(Blocker(instructions)) {
    printf("%s", this->blocker.to_string().c_str());
}