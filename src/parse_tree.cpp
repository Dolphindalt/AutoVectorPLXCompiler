#ifndef PARSE_TREE_CPP__
#define PARSE_TREE_CPP__

#include <parse_tree.h>

#include <stack>
#include <iostream>
#include <set>

template<class T>
PTNode<T>::PTNode(const T value): value(value) {}

template<class T>
PTNode<T>::~PTNode() {
    this->children.erase(this->children.begin(), this->children.end());
}

template<class T>
void PTNode<T>::dfsTraversal(
    const PTPtr<T> node, 
    const std::function<void(T)> action
) {
    if (node == nullptr) {
        return;
    }

    std::set<PTPtr<T>> visited;
    visited.insert(node);

    std::stack<PTPtr<T>> stack;
    stack.push(node);

    while (!stack.empty()) {
        PTPtr<T> current = stack.top();
        stack.pop();

        action(current->getValue());

        int neighbors = current->getChildren().size();
        for (int i = 0; i < neighbors; i++) {

            PTPtr<T> child = current->getChildren().at(i);
            if (visited.find(child) == visited.end()) {
                stack.push(child);
                visited.insert(child);
            }

            child = nullptr;
        }

        current = nullptr;
    }
}

template<class T>
void PTNode<T>::printTree(PTPtr<T> root, int treeSize) {
    std::vector<bool> flag(treeSize, true);
    PTNode<T>::printNTree(root, flag, 0, false);
}

template<class T>
T PTNode<T>::getValue() const {
    return this->value;
}

template<class T>
const std::vector<PTPtr<T>> &PTNode<T>::getChildren() const {
    return this->children;
}

template<class T>
PTPtr<T> PTNode<T>::addChild(const PTPtr<T> node) {
    this->children.push_back(node);
    return node;
}

template<class T>
PTPtr<T> PTNode<T>::addChild(const T value) {
    auto toReturn = std::make_shared<PTNode<T>>(value);
    this->children.push_back(toReturn);
    return toReturn;
}

template<class T>
void PTNode<T>::printNTree(
    PTPtr<T> node, 
    std::vector<bool> flag, 
    int depth,
    bool isLast
) {
    if (node == nullptr) {
        return;
    }

    for (int i = 1; i < depth; i++) {
        if (flag[i] == true) {
            std::cout << "|    ";
        } else {
            std::cout << "    ";
        }
    }

    if (depth == 0) {
        std::cout << node->getValue() << std::endl;
    } else if (isLast) {
        std::cout << "+--- " << node->getValue() << std::endl;
        flag[depth] = false;
    } else {
        std::cout << "+--- " << node->getValue() << std::endl;
    }

    size_t it = 0;
    for (
        auto itr = node->getChildren().begin(); 
        itr != node->getChildren().end();
        itr++, it++
    ) {
        PTNode<T>::printNTree(
            *itr, 
            flag, 
            depth + 1, 
            it == (node->getChildren().size() - 1)
        );
    }

    flag[depth] = true;
}

#endif
