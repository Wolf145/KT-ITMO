#pragma once

#include <stdexcept>
#include <istream>
#include <string>

#include "Token.hpp"

class LexicalAnalyzer
{
    std::istream &in;
    Token curToken;
    int curChar;
    int curPos;

public:
    LexicalAnalyzer(std::istream &);

    inline void parse()
    {
        nextChar();
    }
    inline Token getCurToken()
    {
        return curToken;
    }
    inline int getCurPos()
    {
        return curPos;
    }

    void nextToken();

private:
    void nextChar();
    bool handleBlank(int);
    bool handleEOF(int);
    bool handleLetter();
    void handleNot();
    void handleXor();
    void handleAnd();
    void handleOr();
    void handleEq();
    void handlerTemplate(char, char, Token);
};