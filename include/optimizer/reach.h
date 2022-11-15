#ifndef REACH_H__
#define REACH_H__

#include <optimizer/basic_block.h>
#include <3ac.h>
#include <map>
#include <string>

class CFG;

class Reach {
public:
    Reach();
    Reach(CFG *cfg);

    std::string to_string() const;
private:
    void worklistReaching();

    // All varibles that are available when coming out of a block.
    std::map<BBP, std::set<TID>> out;
    // All variables that are available when coming into a block.
    std::map<BBP, std::set<TID>> in;
    CFG *cfg;
};

#endif