#ifndef LOOP_VECTORIZER
#define LOOP_VECTORIZER

#include <optimizer/natural_loop.h>

#define DISTANCE_LESS (-1)
#define DISTANCE_MORE 1
#define DISTANCE_EQUAL 0

class LoopVectorizer {
public:
    LoopVectorizer(NaturalLoop &loop);
    
    void vectorize();

    /**
     * Unrolls a loop up until the unroll factor.
     * 
     * Example:
     * i := 0;
     * while i < 10 do
     * begin
     *     i := i + 1;
     * end;
     * With a factor of 4:
     * while i < 10 do
     * begin
     *     i := i + 1;
     *     i := i + 1;
     *     i := i + 1;
     *     i := i + 1;
     * end
     * while i < 10 do
     * begin
     *     i := i + 1;
     * end
     * 
     * The purpose of this is to not unroll vectorized operations but unroll
     * all other scalar operations. The unroll factor would be how many 
     * elements are to be vectorized.
     */
    void stripMineLoop(const unsigned int unroll);

    static bool isInstructionDependentOnIndex(
        const NaturalLoop &loop,
        const tac_line_t &inst,
        const induction_variable_t &index
    );

    static bool isVariableDependentOnIndex(
        const NaturalLoop &loop, 
        const std::string &variable,
        const induction_variable_t &index
    );
private:
    bool checkCanLoopBeVectorized();
    /**
     * A direction vector shows how a memory use within a iteration relates to
     * other iterations.
     * 1 implies the next iteration is referenced.
     * 0 implies the current iteration is referenced.
     * -1 implies the previous iteration is referenced.
     */
    bool computeDirectionVectors(
        std::vector<int> &direction_vectors_out
    ) const;

    bool getDirectionVectorFromVariable(
        const std::string &variable,
        const std::string &index,
        bool &dependsOnIterator,
        int &vector_out
    ) const;

    bool getDistanceVectorFromVariable(
        const std::string &variable,
        const std::string &index,
        bool &containedIterator,
        int &vector_out
    ) const;

    // Assuming the loop is vectorized, should it be vectorized at all?
    bool shouldVectorizeLoop() const;

    NaturalLoop &loop;
    bool canVectorize;
    induction_variable_t index;
    std::vector<int> directionVectors;
};

#endif