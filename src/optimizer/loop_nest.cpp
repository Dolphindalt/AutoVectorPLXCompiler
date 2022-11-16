#include <optimizer/loop_nest.h>

LoopNest::LoopNest() {

}

bool LoopNest::canVectorize() const {
    for (const NaturalLoop &loop : this->nest) {
        if (!loop.isSimpleLoop()) {
            return false;
        }
    }
}

std::vector<NaturalLoop> LoopNest::getNest() {
    return this->nest;
}