#pragma once

#include <string>

enum class TokenType
{
    LPAREN,
    RPAREN,
    LETTER, // [a-Z][A-Z]
    AND,
    OR,
    XOR,
    NOT,
    EQ,
    END
};

struct Token
{
    TokenType type;
    int val;

public:
    Token();
    Token(TokenType);
    Token(char);
    std::string to_string();
};