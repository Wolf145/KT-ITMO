#pragma once
#include <string>

#include "Node.hpp"

struct ParseResult
{
    Node::NodePtr tree;

    ParseResult(const std::string &);
};