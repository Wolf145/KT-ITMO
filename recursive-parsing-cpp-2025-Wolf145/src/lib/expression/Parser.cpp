#include <istream>
#include <string>
#include <memory>

#include "Lexer.hpp"
#include "Error.hpp"
#include "Node.hpp"
#include "Token.hpp"
#include "Parser.hpp"

Parser::Parser(std::istream &in)
{
    cntNodes = 0;
    lex = std::make_unique<LexicalAnalyzer>(in);
}
Node::NodePtr Parser::parse()
{
    try
    {
        lex->parse();
        lex->nextToken();
    }
    catch (const ParseError &e)
    {
        throw e;
    }
    Node::NodePtr tree = S_();
    if (lex->getCurToken().type != TokenType::END)
        throw ParseError("Error: Expected end of file, but found: " + lex->getCurToken().to_string() + ".");
    return tree;
}
Node::NodePtr Parser::S_()
{
    Node::NodePtr tree = std::make_unique<Node>("S_", ++cntNodes);

    Token curToken = lex->getCurToken();

    switch (curToken.type)
    {
    case TokenType::END:
        return tree;
    case TokenType::LPAREN:
    case TokenType::NOT:
    case TokenType::LETTER:
    {
        tree->addChild(S());
        return tree;
    }
    default:
        throw ParseError("Error: Invalid token: " + lex->getCurToken().to_string() + ".");
    }
}
Node::NodePtr Parser::S()
{
    Node::NodePtr tree = std::make_unique<Node>("S", ++cntNodes);
    Token curToken = lex->getCurToken();

    switch (curToken.type)
    {
    case TokenType::LETTER:
    case TokenType::LPAREN:
    case TokenType::NOT:
    {
        tree->addChild(T());
        tree->addChild(SPrime());
        return tree;
    }
    default:
        throw ParseError("Error: Invalid token: " + lex->getCurToken().to_string() + ".");
    }
}
Node::NodePtr Parser::SPrime()
{
    Node::NodePtr tree;
    Token curToken = lex->getCurToken();

    switch (curToken.type)
    {
    case TokenType::END:
    case TokenType::RPAREN:
        tree = std::make_unique<Node>("S\'", ++cntNodes);
        break;
    case TokenType::OR:
    {
        tree = std::make_unique<Node>("operator", ++cntNodes, "or");
        lex->nextToken();
        tree->addChild(T());
        tree->addChild(SPrime());
        break;
    }
    default:
        throw ParseError("Error: Invalid token: " + lex->getCurToken().to_string() + ".");
    }

    return tree;
}
Node::NodePtr Parser::T()
{
    Node::NodePtr tree = std::make_unique<Node>("T", ++cntNodes);
    Token curToken = lex->getCurToken();

    switch (curToken.type)
    {
    case TokenType::LPAREN:
    case TokenType::LETTER:
    case TokenType::NOT:
    {
        tree->addChild(F());
        tree->addChild(TPrime());
        return tree;
    }
    default:
        throw ParseError("Error: Invalid token: " + lex->getCurToken().to_string() + ".");
    }
}
Node::NodePtr Parser::TPrime()
{
    Node::NodePtr tree;
    Token curToken = lex->getCurToken();

    switch (curToken.type)
    {
    case TokenType::END:
    case TokenType::OR:
    case TokenType::RPAREN:
        tree = std::make_unique<Node>("T\'", ++cntNodes);
        break;
    case TokenType::XOR:
    {
        tree = std::make_unique<Node>("operator", ++cntNodes, "xor");
        lex->nextToken();
        tree->addChild(F());
        tree->addChild(TPrime());
        break;
    }
    default:
        throw ParseError("Error: Invalid token: " + lex->getCurToken().to_string() + ".");
    }

    return tree;
}
Node::NodePtr Parser::F()
{
    Node::NodePtr tree = std::make_unique<Node>("F", ++cntNodes);
    Token curToken = lex->getCurToken();

    switch (curToken.type)
    {
    case TokenType::LPAREN:
    case TokenType::LETTER:
    case TokenType::NOT:
    {
        tree->addChild(N());
        tree->addChild(FPrime());
        return tree;
    }
    default:
        throw ParseError("Error: Invalid token: " + lex->getCurToken().to_string() + ".");
    }
}
Node::NodePtr Parser::FPrime()
{
    Node::NodePtr tree;
    Token curToken = lex->getCurToken();

    switch (curToken.type)
    {
    case TokenType::END:
    case TokenType::OR:
    case TokenType::XOR:
    case TokenType::RPAREN:
        tree = std::make_unique<Node>("F\'", ++cntNodes);
        break;
    case TokenType::AND:
    {
        tree = std::make_unique<Node>("operator", ++cntNodes, "and");
        lex->nextToken();
        tree->addChild(N());
        tree->addChild(FPrime());
        break;
    }
    default:
        throw ParseError("Error: Invalid token: " + lex->getCurToken().to_string() + ".");
    }

    return tree;
}

Node::NodePtr Parser::N()
{
    Node::NodePtr tree;
    Token curToken = lex->getCurToken();

    switch (curToken.type)
    {
    case TokenType::NOT:
    {
        tree = std::make_unique<Node>("operator", ++cntNodes, "not");
        lex->nextToken();
        tree->addChild(N());
        break;
    }
    case TokenType::LPAREN:
    case TokenType::LETTER:
    {
        tree = std::make_unique<Node>("N", ++cntNodes);
        tree->addChild(C());
        break;
    }
    default:
        throw ParseError("Error: Invalid token: " + lex->getCurToken().to_string() + ".");
    }
    return tree;
}

Node::NodePtr Parser::C()
{
    Node::NodePtr tree;
    Token curToken = lex->getCurToken();

    switch (curToken.type)
    {
    case TokenType::LPAREN:
    case TokenType::LETTER:
    {
        tree = std::make_unique<Node>("С", ++cntNodes);
        tree->addChild(M());
        tree->addChild(CPrime());
        return tree;
    }
    default:
        throw ParseError("Error: Invalid token: " + lex->getCurToken().to_string() + ".");
    }
    return tree;
}
Node::NodePtr Parser::CPrime()
{
    Node::NodePtr tree;
    Token curToken = lex->getCurToken();
    switch (curToken.type)
    {
    case TokenType::END:
    case TokenType::AND:
    case TokenType::OR:
    case TokenType::XOR:
    case TokenType::RPAREN:
        tree = std::make_unique<Node>("C\'", ++cntNodes);
        break;
    case TokenType::EQ:
    {
        tree = std::make_unique<Node>("operator", ++cntNodes, "==");
        lex->nextToken();
        tree->addChild(M());
        tree->addChild(CPrime());
        break;
    }
    default:
        throw ParseError("Error: Invalid token: " + lex->getCurToken().to_string() + ".");
    }
    return tree;
}

Node::NodePtr Parser::M()
{
    Node::NodePtr tree;
    Token curToken = lex->getCurToken();

    switch (curToken.type)
    {
    case TokenType::LETTER:
    {
        tree = std::make_unique<Node>("letter", ++cntNodes, std::string(1, curToken.val));
        lex->nextToken();
        return tree;
    }
    case TokenType::LPAREN:
    {
        tree = std::make_unique<Node>("M", ++cntNodes);
        tree->addChild(std::make_unique<Node>("(", ++cntNodes));
        lex->nextToken();
        tree->addChild(S());

        if (lex->getCurToken().type != TokenType::RPAREN)
            throw ParseError("Error: Invalid token: expected \')\', but found: " + lex->getCurToken().to_string() + ".");

        tree->addChild(std::make_unique<Node>(")", ++cntNodes));
        lex->nextToken();

        return tree;
    }
    default:
        throw ParseError("Error: Invalid token: " + lex->getCurToken().to_string() + ".");
    }
}
