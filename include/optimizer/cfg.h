#ifndef CFG_H__
#define CFG_H__

#include <optimizer/basic_block.h>
#include <optimizer/block_types.h>
#include <optimizer/dominator.h>
#include <optimizer/natural_loop.h>
#include <optimizer/reach.h>

#include <string>
#include <functional>

#define AUTOMATIC_VECTORIZATION_ENABLED

class CFG {
public:
    CFG();
    CFG(BlockSet &allBlocks, std::string &name, BBP firstBlock);

    void performPostorderTraversal(std::function<void(BBP block)> action) const;

    BBP getEntryBlock() const;
    const std::string getName() const;
    std::string to_graph() const;
private:
    void computePostorderTraversalInternal(
        BBP node, std::set<BBP> &visited, std::function<void(BBP block)> action
    ) const;

    std::set<std::pair<BBP, BBP>> computeBackwardsEdges();

    std::vector<NaturalLoop> computeNaturalLoops(
        std::set<std::pair<BBP, BBP>> backedges,
        BlockSet &allBlocks
    ) const;

    std::string name;
    BlockSet blocks;
    BBP entryBlock;
    Dominator dominator;
    Reach reach;
};

class PostOrderView {
public:
    PostOrderView(CFG *cfg);

    bool comparator(BBP lhs, BBP rhs) const; 

    std::vector<BBP> &getPO();
private:
    unsigned int idGenerator = 0;
    std::map<BBP, unsigned int> poIndexMap;
    std::vector<BBP> postorder;
};

#endif