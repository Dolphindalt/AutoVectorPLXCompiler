/**
 *  CPSC 323 Compilers and Languages
 * 
 *  Dalton Caron, Teaching Associate
 *  dcaron@fullerton.edu, +1 949-616-2699
 *  Department of Computer Science
 */
#include <catch2/catch.hpp>

#include <parse_tree.h>
#include <iostream>

TEST_CASE("DFS Traversal", "[ParseTree]") {

    PTPtr<int> root = std::make_shared<PTNode<int>>(1);
    root->addChild(std::make_shared<PTNode<int>>(2));
    root->addChild(std::make_shared<PTNode<int>>(3));
    REQUIRE(root->getChildren().size() == 2);
    root->getChildren().at(0)->addChild(std::make_shared<PTNode<int>>(4));
    root->getChildren().at(0)->addChild(std::make_shared<PTNode<int>>(5));
    root->getChildren().at(0)->addChild(std::make_shared<PTNode<int>>(6));

    int inorder[6] = { 1, 3, 2, 6, 5, 4 };
    unsigned int index = 0;

    PTNode<int>::dfsTraversal(root, [inorder, &index](int node_value) {
        REQUIRE(inorder[index] == node_value);
        index++;
    });
}