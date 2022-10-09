#ifndef PARSE_TREE_H__
#define PARSE_TREE_H__

#include <string>
#include <memory>
#include <functional>

// Forward declaration for alias.
template<class T>
class PTNode;

// Parse Tree Pointer.
template<class T>
using PTPtr = std::shared_ptr<PTNode<T>>;

template<class T>
using ParseTree = PTPtr<T>;

// Parse Tree Node.
template<class T>
class PTNode {
public:
    PTNode(const T value);
    virtual ~PTNode();

    static void inorderTraversal(
        PTPtr<T> root, 
        const std::function<void(T)> action
    );

    T getValue() const;

    PTPtr<T> getLeft() const;
    void setLeft(const PTPtr<T> left);

    PTPtr<T> getRight() const;
    void setRight(const PTPtr<T> right);
private:
    T value;
    PTPtr<T> left = nullptr;
    PTPtr<T> right = nullptr;
};

#include <parse_tree.cpp>
#endif