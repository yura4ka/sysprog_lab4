#ifndef LAB4_TREE_NODE_H
#define LAB4_TREE_NODE_H

#include <string>
#include <utility>
#include <vector>

struct TreeNode {
    explicit TreeNode(std::string v) : value(std::move(v)) {}

    std::string value;
    std::vector<TreeNode> children;
};

#endif
