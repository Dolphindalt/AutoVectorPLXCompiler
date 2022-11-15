#ifndef DOMINATOR_H__
#define DOMINATOR_H__

#include <optimizer/basic_block.h>

#include <map>
#include <vector>
#include <string>

class CFG;

class PostOrderView;

class Dominator {
public:
    Dominator();
    Dominator(CFG *cfg);

    bool dominates(const BBP a, const BBP b) const;

    std::string to_graph();
private:
    BBP getNode(const BBP node) const;

    bool properlyDominates(const BBP a, const BBP b) const;

    void buildDominatorTree();

    BBP intersect(BBP b1, BBP b2, PostOrderView &po);

    CFG *cfg;
    std::map<BBP, BBP> iDoms;
};

#endif