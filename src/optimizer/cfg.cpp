#include <optimizer/cfg.h>

#include <algorithm>
#include <cstdio>
#include <logging.h>
#include <assertions.h>

CFG::CFG() {};

CFG::CFG(std::string &name, BBP firstBlock) : name(name), 
    entryBlock(firstBlock), dominator(Dominator(this)), reach(Reach(this)) {
        printf("%s\n\n", this->to_graph().c_str());
        printf("%s", this->dominator.to_graph().c_str());
        auto backedges = this->computeBackwardsEdges();
        printf("Back edges\n");
        for (auto p : backedges) {
            printf("(%d, %d)\n", p.first->getID(), p.second->getID());
        }
        auto nloops = this->computeNaturalLoops(backedges);
        printf("Natural Loops\n");
        for (auto p : nloops) {
            printf("(%d, %d)\n", p.getHeader()->getID(), p.getFooter()->getID());
        }
        printf("Reach Analysis\n%s\n", this->reach.to_string().c_str());
    }

BBP CFG::getEntryBlock() const {
    return this->entryBlock;
}

const std::string CFG::getName() const {
    return this->name;
}

std::string CFG::to_graph() const {
    std::string result = "digraph G {\n";

    // Print out the node declarations first.
    this->performPostorderTraversal([&result](BBP block) {
        result += "\t" + block->to_string() + "\n";
    });

    // Then print the edges.
    this->performPostorderTraversal([&result](BBP block) {
        auto successors = block->getSuccessors();

        std::for_each(successors.begin(), successors.end(), 
            [&result, &block](BBP inner) {
                result += "\t" + std::to_string(block->getID()) + 
                    " -> " + std::to_string(inner->getID()) + "\n";
            }
        );
    });

    result += "}\n";
    return result;
}

void CFG::performPostorderTraversal(
    std::function<void(BBP block)> action
) const {
    std::set<BBP> visited;
    this->computePostorderTraversalInternal(this->entryBlock, visited, action);
}

void CFG::computePostorderTraversalInternal(
    BBP node, std::set<BBP> &visited, std::function<void(BBP block)> action
) const {
    // Visit all children first from left to right.
    const std::vector<BBP> successors = node->getSuccessors();
    std::for_each(successors.begin(), successors.end(),
        [this, &visited, action](BBP block) {
            if (visited.count(block) == 0) {
                visited.insert(block);
                this->computePostorderTraversalInternal(block, visited, action);
            }
        }
    );

    // Visit the node.
    action(node);
}

std::set<std::pair<BBP, BBP>> CFG::computeBackwardsEdges() {
    std::set<std::pair<BBP, BBP>> backedges;
    std::set<BBP> visited;
    
    PostOrderView pov(this);
    std::vector<BBP> po = pov.getPO();
    for (auto i = po.rbegin(); i != po.rend(); i++) {
        BBP current = *i;
        std::vector<BBP> succ = current->getSuccessors();
        for (auto j = succ.begin(); j != succ.end(); j++) {
            BBP after = *j;
            if (visited.count(after) == 1) {
                backedges.insert(std::make_pair(current, after));
            }
        }
        visited.insert(current);
    }

    return backedges;
}

std::vector<NaturalLoop> CFG::computeNaturalLoops(
    std::set<std::pair<BBP, BBP>> backedges
) const {
    std::vector<NaturalLoop> loops;

    for (auto e = backedges.begin(); e != backedges.end(); e++) {
        if (this->dominator.dominates(e->second, e->first)) {
            loops.push_back(NaturalLoop(e->second, e->first));
        }
    }

    return loops;
}

PostOrderView::PostOrderView(CFG *cfg) {
    cfg->performPostorderTraversal([this](BBP block) {
        this->poIndexMap[block] = this->idGenerator++;
        this->postorder.push_back(block);
    });
}

bool PostOrderView::comparator(BBP lhs, BBP rhs) const {
    ASSERT(lhs != nullptr);
    ASSERT(rhs != nullptr);
    return this->poIndexMap.at(lhs) < this->poIndexMap.at(rhs);
}

std::vector<BBP> &PostOrderView::getPO() {
    return this->postorder;
}
