#include <optimizer/reach.h>

#include <optimizer/cfg.h>

#include <set>
#include <map>
#include <algorithm>

Reach::Reach() {}

Reach::Reach(CFG *cfg) : cfg(cfg) {
    this->worklistReaching();
}

std::set<std::string> Reach::getVariablesIntoBlock(const BBP bb) const {
    if (this->in.count(bb) != 0) {
        return this->in.at(bb);
    }
    return std::set<std::string>();
}

std::set<std::string> Reach::getVariablesOutOfBlock(const BBP bb) const {
    if (this->out.count(bb) != 0) {
        return this->out.at(bb);
    }
    return std::set<std::string>();
}

std::string Reach::to_string() const {
    std::string result = "In\tOut\n";
    this->cfg->performPostorderTraversal([this, &result](BBP bb) {
        // Print in set.
        result += "BB: " + std::to_string(bb->getID()) + " ";
        result += "{ ";
        for (auto i : this->in.at(bb)) {
            result += i + " ";
        }

        result += "} | { ";
        // Print out set.
        for (auto i : this->out.at(bb)) {
            result += i + " ";
        }
        result += "}\n";
    });
    return result;
}

// Algorithm from https://en.wikipedia.org/wiki/Reaching_definition
void Reach::worklistReaching() {

    std::map<BBP, TIDSet> out;
    std::map<BBP, TIDSet> in;
    std::set<BBP> changed;

    // Put all nodes into changed set.
    this->cfg->performPostorderTraversal(
        [&changed, &out, &in](BBP bb) { 
            bb->computeGenAndKillSets();
            changed.insert(bb);
            // Initialize out[n].
            out.insert(std::make_pair(bb, bb->getGenSet()));
            // Init in[n] to be empty set.
            in.insert(std::make_pair(bb, TIDSet()));
        }
    );

    while (!changed.empty()) {
        // Chose a node.
        BBP n = *changed.rbegin();
        // Remove it from the change set.
        changed.erase(n);
        // Calculate in[n] fom predecessors' out[p].
        for (
            auto p = n->getPredecessors().begin(); 
            p != n->getPredecessors().end(); 
            p++
        ) {
            in[n].merge(out.at(*p));
        }
        // Save the old out[n].
        TIDSet oldOut = TIDSet(out[n]);
        // Update out[n] using transfer function f_n().
        TIDSet result;
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

    for (auto p : in) {
        std::set<std::string> varNames;
        for (auto v : p.second) {
            varNames.insert(v.result);
        }
        this->in.insert(std::make_pair(p.first, varNames));
    }
    for (auto p : out) {
        std::set<std::string> varNames;
        for (auto v : p.second) {
            varNames.insert(v.result);
        }
        this->out.insert(std::make_pair(p.first, varNames));
    }
}