#ifndef PARSE_TREE_CPP__
#define PARSE_TREE_CPP__

#include <parse_tree.h>

#include <stack>

template<class T>
PTNode<T>::PTNode(const T value): value(value) {}

template<class T>
PTNode<T>::~PTNode() {
    // Ensure the reference counters are decremented.
    // Also, never leave dangling pointers!
    this->left = nullptr;
    this->right = nullptr;
}

template<class T>
void PTNode<T>::inorderTraversal(
    const PTPtr<T> root, 
    const std::function<void(T)> action
) {
    if (root == nullptr) {
        return;
    }

    std::stack<PTPtr<T>> nodes;
    PTPtr<T> current = root;

    while (!nodes.empty() || current != nullptr) {
        if (current != nullptr) {
            nodes.push(current);
            current = current->getLeft();
        }
        else {
            current = nodes.top();
            nodes.pop();
            action(current->getValue());
            current = current->getRight();
        }
    }

    current = nullptr;
}

template<class T>
T PTNode<T>::getValue() const {
    return this->value;
}

template<class T>
PTPtr<T> PTNode<T>::getLeft() const {
    return this->left;
}

template<class T>
void PTNode<T>::setLeft(const PTPtr<T> left) {
    this->left = left;
}

template<class T>
PTPtr<T> PTNode<T>::getRight() const {
    return this->right;
}

template<class T>
void PTNode<T>::setRight(const PTPtr<T> right) {
    this->right = right;
}

#endif
