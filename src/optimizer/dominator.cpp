#include <optimizer/dominator.h>

#include <optimizer/cfg.h>
#include <algorithm>
#include <assertions.h>
#include <logging.h>

Dominator::Dominator() {}

Dominator::Dominator(CFG *cfg) : cfg(cfg) {
    this->buildDominatorTree();
}

bool Dominator::dominates(const BBP a, const BBP b) const {
    if (a == b) {
        return true;
    }

    return this->properlyDominates(a, b);
}

std::string Dominator::to_graph() {
    std::string result = "Dominator tree for " + this->cfg->getName() + ":\n";
    this->cfg->performPostorderTraversal([this, &result](BBP block) {

        ASSERT(this->iDoms.count(block));

        result += std::to_string(block->getID()) + " dom " + 
            std::to_string(this->iDoms[block]->getID()) + "\n";

    });
    return result;
}

BBP Dominator::getNode(const BBP node) const {
    auto i = this->iDoms.find(node);
    return i != this->iDoms.end() ? i->second : nullptr;
}

bool Dominator::properlyDominates(const BBP a, const BBP b) const {
    if (a == nullptr || b == nullptr || a == b) {
        return false;
    }

    // The entry node dominates all other blocks.
    if (a == this->cfg->getEntryBlock()) {
        return true;
    }

    BBP idom = this->getNode(b);
    while (idom != a && idom != this->cfg->getEntryBlock()) {
        idom = this->getNode(idom);
    }

    return idom != this->cfg->getEntryBlock();
}

// This code is based on the algorithm described in the following paper:
// https://www.cs.rice.edu/~keith/Embed/dom.pdf
void Dominator::buildDominatorTree() {
    BBP entryBlock = this->cfg->getEntryBlock();

    // Collect the nodes in postorder.
    PostOrderView pov(this->cfg);
    std::vector<BBP> postorder = pov.getPO();

    this->iDoms[entryBlock] = entryBlock;

    bool changed = true;
    while (changed) {
        changed = false;
        for (auto i = postorder.rbegin(); i != postorder.rend(); i++) {
            // Skip the start node.
            if (entryBlock == *i)
                continue;
            
            // Continue with the algorithm.
            if (BBP b = *i) {
                if (b->getPredecessors().size() == 0) continue;
                // Compute the immediate dominance of b.
                std::vector<BBP> preds = b->getPredecessors();
                BBP new_idiom = *preds.begin();
                ASSERT(new_idiom != nullptr);
                for (auto p = ++preds.begin(); p != preds.end(); p++) {
                    // If the dominace of p is already calculated.
                    if (this->iDoms.count(*p) != 0 && this->iDoms[*p] != nullptr) {
                        new_idiom = this->intersect(*p, new_idiom, pov);
                    }
                }
                if (this->iDoms[b] != new_idiom) {
                    this->iDoms[b] = new_idiom;
                    changed = true;
                }
            }
        }

    }

    entryBlock = nullptr;
}

BBP Dominator::intersect(BBP b1, BBP b2, PostOrderView &po) {
    ASSERT(b1 != nullptr);
    ASSERT(b2 != nullptr);
    BBP finger1 = b1;
    BBP finger2 = b2;
    while (finger1 != finger2) {
        while (po.comparator(finger1, finger2)) {
            finger1 = this->iDoms[finger1];
        }
        while (po.comparator(finger2, finger1)) {
            finger2 = this->iDoms[finger2];
        }
    }
    return finger1;
}