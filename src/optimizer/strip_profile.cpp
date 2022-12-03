#include <optimizer/strip_profile.h>

StripProfile::StripProfile(
    BBP bb, 
    const unsigned int factor, 
    std::vector<tac_line_t> &iteration,
    const bool vectorize
) : block(bb), factor(factor), iteration(iteration), vectorize(vectorize) {
    if (vectorize) {
        this->insertVectorInstructions();
    }
}

void StripProfile::insertVectorInstructions() {
    /**
     * If no statements in the loop are dependent upon the i iterator, then
     * the loop can be collapsed such that all operations are performed in 
     * parallel in a single iteration.
     */

    /**
     * Keep in mind that there is an assumption that the loop can be executed 
     * in parallel, so arrays will never contain any data dependencies. This 
     * could be relaxed to support a wider range of loops.
     */
    

}

void StripProfile::unroll() {
    for (unsigned int i = 0; i < this->factor; i++) {
        this->block->insertInstructions(this->iteration, false);
    }
}