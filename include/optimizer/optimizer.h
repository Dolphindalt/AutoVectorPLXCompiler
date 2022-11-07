#ifndef OPTIMIZER_H__
#define OPTIMIZER_H__

#include <vector>
#include <map>
#include <memory>

#include <3ac.h>
#include <optimizer/basic_block.h>
#include <optimizer/group.h>

class Optimizer {
public:
    Optimizer(std::vector<tac_line_t> &instructions);
    virtual ~Optimizer();

    std::string to_string() const;
private:
    /**
     * The start and end iterator denote the insructions to consider. At first,
     * the entire program is considered, but procedures are skipped and 
     * processed independently via a recursive call.
     * 
     * The map keeps track of which labels are in what blocks so that labels
     * can be substituted for block numbers.
     * 
     * @param start The iterator pointing to the start of the instructions to
     * consider.
     * @param end The iterator pointing to the end of the instructions to 
     * consider. The end is not inclusive.
     * @param labelToBlockMap A map that is populated with label to block 
     * number pairs.
     */
    void populateGroups(
        std::vector<tac_line_t>::iterator start,
        const std::vector<tac_line_t>::iterator end,
        std::map<std::string, unsigned int> &labelToBlockMap
    );

    void replaceLabelsWithTargetBlocks(
        const std::map<std::string, unsigned int> &labelToBlockMap
    );

    std::vector<Group> groups;
    std::vector<BBP> allBasicBlocks;
};

#endif