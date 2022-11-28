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

    NaturalLoop &loop;
    bool canVectorize;
    std::string index;
    std::vector<int> directionVectors;
};

#endif