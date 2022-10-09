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

TEST_CASE("Inorder Traversal", "[ParseTree]") {

    PTPtr<int> root = std::make_shared<PTNode<int>>(1);
    root->setLeft(std::make_shared<PTNode<int>>(2));
    root->setRight(std::make_shared<PTNode<int>>(3));
    root->getLeft()->setLeft(std::make_shared<PTNode<int>>(4));
    root->getLeft()->setRight(std::make_shared<PTNode<int>>(5));

    int inorder[5] = { 4, 2, 5, 1, 3 };
    unsigned int index = 0;

    PTNode<int>::inorderTraversal(root, [inorder, &index](int node_value) {
        REQUIRE(inorder[index] == node_value);
        index++;
    });
}