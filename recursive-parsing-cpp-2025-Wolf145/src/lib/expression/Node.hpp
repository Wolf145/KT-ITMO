#pragma once
#include <string>
#include <vector>
#include <memory>

class Node
{
public:
    using NodePtr = std::unique_ptr<Node>;

    inline int getId() const
    {
        return id;
    }
    inline std::string getTerm() const
    {
        return term;
    }
    inline std::string getVal() const
    {
        return val;
    }
    inline std::string getNodeName() const
    {
        return term + (val.empty() ? "" : ": \'" + val + "\'");
    }
    inline const std::vector<NodePtr> &getChildren() const
    {
        return children;
    }
    inline bool isLeaf() const
    {
        return children.empty();
    }
    inline void addChild(NodePtr child)
    {
        children.push_back(std::move(child));
    }

    std::string unfold() const;
    std::string unfoldWithPriority() const;

    Node(const std::string &, int);
    Node(const std::string &, int, const std::string &);

private:
    int id;
    std::string term;
    std::string val;
    std::vector<NodePtr> children;
};