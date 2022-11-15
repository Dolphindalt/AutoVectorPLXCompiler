#include <optimizer/reach.h>

#include <optimizer/cfg.h>

#include <set>
#include <map>
#include <algorithm>

Reach::Reach() {}

Reach::Reach(CFG *cfg) : cfg(cfg) {
    this->worklistReaching();
}

std::string Reach::to_string() const {
    std::string result = "In\tOut\n";
    this->cfg->performPostorderTraversal([this, &result](BBP bb) {
        // Print in set.
        result += "BB: " + std::to_string(bb->getID()) + " ";
        result += "{ ";
        for (const TID i : this->in.at(bb)) {
            result += std::to_string(i) + " ";
        }

        result += "} | { ";
        // Print out set.
        for (const TID i : this->out.at(bb)) {
            result += std::to_string(i) + " ";
        }
        result += "}\n";
    });
    return result;
}

// Algorithm from https://en.wikipedia.org/wiki/Reaching_definition
void Reach::worklistReaching() {

    this->in.clear();
    this->out.clear();
    std::set<BBP> changed;

    // Put all nodes into changed set.
    this->cfg->performPostorderTraversal(
        [&changed, this](BBP bb) { 
            bb->computeGenAndKillSets();
            changed.insert(bb);
            // Initialize out[n].
            if (bb != this->cfg->getEntryBlock()) {
                this->out.insert(std::make_pair(bb, std::set<TID>()));
            }
        }
    );

    while (!changed.empty()) {
        // Chose a node.
        BBP n = *changed.begin();
        // Remove it from the change set.
        changed.erase(n);
        // Init in[n] to be empty set.
        in.insert(std::make_pair(n, std::set<TID>()));
        // Calculate in[n] fom predecessors' out[p].
        for (
            auto p = n->getPredecessors().begin(); 
            p != n->getPredecessors().end(); 
            p++
        ) {
            if (out.count(*p) != 0)
                in[n].merge(out.at(*p));
        }
        // Save the old out[n].
        std::set<TID> oldOut = std::set<TID>(out[n]);
        // Update out[n] using transfer function f_n().
        std::set<TID> result;
        auto gen = n->getGenSet();
        auto kill = n->getKillSet();
        std::set_difference(
            in.at(n).begin(), in.at(n).end(),
            kill.begin(), kill.end(),
            std::inserter(result, result.end())
        );
        result.merge(gen);
        out[n] = result;
        // Any change to out[n]?
        if (out[n] != oldOut) {
            // If yes, put all successors of n into the changed set.
            for (
                auto s = n->getSuccessors().begin();
                s != n->getSuccessors().end();
                s++
            ) {
                changed.insert(*s);
            }
        }
    }
}