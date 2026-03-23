#pragma once
#include <istream>

#include "Lexer.hpp"
#include "Node.hpp"

class Parser
{
    int cntNodes;
    std::unique_ptr<LexicalAnalyzer> lex;

public:
    Parser(std::istream &);
    Node::NodePtr parse();

private:
    Node::NodePtr S_();
    Node::NodePtr S();
    Node::NodePtr SPrime();
    Node::NodePtr T();
    Node::NodePtr TPrime();
    Node::NodePtr F();
    Node::NodePtr FPrime();
    Node::NodePtr N();
    Node::NodePtr C();
    Node::NodePtr CPrime();
    Node::NodePtr M();
};
