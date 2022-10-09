#ifndef PARSE_TREE_H__
#define PARSE_TREE_H__

#include <string>
#include <memory>
#include <functional>
#include <vector>

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

    static void dfsTraversal(
        PTPtr<T> root, 
        const std::function<void(T)> action
    );

    static void printTree(PTPtr<T> root, int treeSize);

    T getValue() const;

    const std::vector<PTPtr<T>> &getChildren() const;

    const void addChild(const PTPtr<T> node);
private:
    static void printNTree(
        PTPtr<T> node,
        std::vector<bool> flag, 
        int depth = 0,
        bool isLast = false
    );

    T value;
    std::vector<PTPtr<T>> children;
};

#include <parse_tree.cpp>
#endif