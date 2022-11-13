#ifndef BLOCK_TYPES_H__
#define BLOCK_TYPES_H__

inline auto bb_cmp = [](BBP a, BBP b) { return a->getID() < b->getID(); };

using BlockSet = std::set<BBP, decltype(bb_cmp)>;

#endif