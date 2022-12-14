/**
 * This file contains code that supports the vectorization of for loops.
 * 
 * @file loop_vectorizer.h
 * @author Dalton Caron
 */
#ifndef LOOP_VECTORIZER
#define LOOP_VECTORIZER

#include <optimizer/natural_loop.h>

#define DISTANCE_LESS (-1)
#define DISTANCE_MORE 1
#define DISTANCE_EQUAL 0

/**
 * Performs vectorization on the input natural loop if vectorization is 
 * determined to be possible without affecting program correctness and would 
 * likely yield a performance gain.
 */
class LoopVectorizer {
public:
    /** 
     * Constructs the loop vectorize and computes vectorizability. 
     * @param loop The loop to evaluate and vectorize.
     */
    LoopVectorizer(NaturalLoop &loop);
    
    /** Attempts to vectorize the loop. */
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

    /**
     * Checks if an instruction is dependent upon the index/iterator of the 
     * loop.
     * 
     * @param loop Loop that contains the dependent index/iterator.
     * @param inst The instruction to evaluate.
     * @param index The loop iterator variable.
     * @return True if the instruction depends on the index, else false.
     */
    static bool isInstructionDependentOnIndex(
        const NaturalLoop &loop,
        const tac_line_t &inst,
        const induction_variable_t &index
    );

    /**
     * Checks if an variable is dependent upon the index/iterator of the loop.
     * 
     * @param loop Loop that contains the dependent index/iterator.
     * @param variable The variable to evaluate.
     * @param index The loop iterator variable.
     * @return True if the variable depends on the index, else false.
     */
    static bool isVariableDependentOnIndex(
        const NaturalLoop &loop, 
        const std::string &variable,
        const induction_variable_t &index
    );
private:
    /** @return True if can be vectorized, else false. Called once. */
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