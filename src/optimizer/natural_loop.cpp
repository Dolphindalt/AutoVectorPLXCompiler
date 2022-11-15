#include <optimizer/natural_loop.h>

NaturalLoop::NaturalLoop(BBP header, BBP footer) 
: header(header), footer(footer) {
    this->findIterators();
}

const BBP NaturalLoop::getHeader() const {
    return this->header;
}

const BBP NaturalLoop::getFooter() const {
    return this->footer;
}

/**
 * For now, we can only find simple iterations in the form of 
 * VALUE OP VALUE where one of the values is not present in the loop body
 * and one value is the loop iterator.
 */
void NaturalLoop::findIterators() {
    this->iterators.clear();

    for (
        auto i = header->getInstructions().begin();
        i != header->getInstructions().end();
        i++
    ) {
        const tac_line_t instruction = *i;
        
    }
}