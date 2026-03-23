#include <string>
#include <vector>
#include <memory>

#include "Node.hpp"

std::string Node::unfold() const
{
    if (this->isLeaf())
    {
        if (term == "(" || term == ")")
            return term;
        else if (term == "letter")
            return val;
        return "";
    }

    std::string result = ((this->term == "operator") ? this->val : "");

    for (const auto &child : children)
    {
        std::string part = child->unfold();
        if (part.empty())
            continue;

        if (!result.empty() && result.back() != '(' && part.front() != ')')
            result += " ";

        result += part;
    }

    return result;
}

std::string Node::unfoldWithPriority() const
{
    if (this->isLeaf())
    {
        if (term == "letter")
            return val;
        return "";
    }
    std::string rightChildTerm = children.back()->getTerm();

    std::string result = ((this->term == "operator") ? this->val : "");

    for (const auto &child : children)
    {
        std::string part = child->unfoldWithPriority();
        if (part.empty())
            continue;

        if (!result.empty() && result.back() != '(' && part.front() != ')')
            result += " ";

        result += part;
    }

    return ((rightChildTerm == "operator" || this->val == "not") ? "(" + result + ")" : result);
}

Node::Node(const std::string &term_, int id_) : id(id_), term(term_), val("") {}
Node::Node(const std::string &term_, int id_, const std::string &val_) : id(id_), term(term_), val(val_) {}
