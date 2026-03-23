#include <stdexcept>
#include <istream>
#include <string>

#include "Token.hpp"
#include "Lexer.hpp"
#include "Error.hpp"

LexicalAnalyzer::LexicalAnalyzer(std::istream &in_) : in(in_), curPos(-1) {}

void LexicalAnalyzer::nextToken()
{
    while (handleBlank(curChar))
        nextChar();
    switch (curChar)
    {
    case 'n':
        handleNot();
        break;
    case 'x':
        handleXor();
        break;
    case 'o':
        handleOr();
        break;
    case 'a':
        handleAnd();
        break;
    case '(':
        nextChar();
        curToken = Token(TokenType::LPAREN);
        break;
    case ')':
        nextChar();
        curToken = Token(TokenType::RPAREN);
        break;
    case '=':
        handleEq();
        break;
    case -1:
        curToken = Token(TokenType::END);
        break;
    default:
    {
        if (std::isalpha(curChar) && handleLetter())
            break;
        throw ParseError("Error: Illegal character: \'" + std::string(1, curChar) + "\'.\n");
    }
    }
}

void LexicalAnalyzer::nextChar()
{
    ++curPos;
    curChar = in.get();
    if (handleEOF(curChar))
    {
        curChar = -1;
        return;
    }
    if (!in)
        throw ParseError("Error: Can't read symbol.\n");
}

bool LexicalAnalyzer::handleBlank(int c)
{
    return c == ' ' || c == '\r' || c == '\n' || c == '\t';
}

bool LexicalAnalyzer::handleEOF(int c) {
    return c == std::char_traits<char>::eof();
}

bool LexicalAnalyzer::handleLetter()
{
    char letter = curChar;
    nextChar();
    if (curChar == ')' || curChar == '(' || handleEOF(curChar))
    {
        curToken = Token(letter);
        return true;
    }
    if (!(handleBlank(curChar)))
        return false;
    curToken = Token(letter);
    nextChar();
    return true;
}

void LexicalAnalyzer::handleNot()
{
    handlerTemplate('o', 't', Token(TokenType::NOT));
}

void LexicalAnalyzer::handleXor()
{
    handlerTemplate('o', 'r', Token(TokenType::XOR));
}

void LexicalAnalyzer::handleAnd()
{
    handlerTemplate('n', 'd', Token(TokenType::AND));
}

void LexicalAnalyzer::handleOr()
{
    if (handleLetter())
        return;
    if (curChar == 'r')
    {
        curToken = Token(TokenType::OR);
        nextChar();
        return;
    }
    throw ParseError("Error: Illegal Operation.\n");
}

void LexicalAnalyzer::handleEq() {
    nextChar();
    if (curChar == '=') {
        curToken = Token(TokenType::EQ);
        nextChar();
        return;
    }
    throw ParseError("Error: Illegal Operation.\n");
}

void LexicalAnalyzer::handlerTemplate(char c1, char c2, Token t)
{
    if (handleLetter())
        return;
    if (curChar != c1)
        goto err;
    nextChar();
    if (curChar != c2)
        goto err;
    nextChar();
    curToken = t;
    return;
err:
    throw ParseError("Error: Illegal Operation.\n");
}