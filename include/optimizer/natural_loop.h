#ifndef NATURAL_LOOP_H__
#define NATURAL_LOOP_H__

#include <optimizer/basic_block.h>

#include <string>

using address = std::string;

class NaturalLoop {
public:
    NaturalLoop(BBP header, BBP footer);

    const BBP getHeader() const;
    const BBP getFooter() const;
private:
    void findIterators();

    BBP header;
    BBP footer;

    std::set<address> iterators;
};

#endif