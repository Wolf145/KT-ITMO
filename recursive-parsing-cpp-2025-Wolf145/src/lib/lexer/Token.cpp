#include <string>
#include "Token.hpp"

std::string Token::to_string()
{
    switch (this->type)
    {
    case TokenType::LPAREN:
        return "\'(\'";
    case TokenType::RPAREN:
        return "\')\'";
    case TokenType::LETTER:
        return "LETTER(\'" + std::string(1, val) + "\')";
    case TokenType::AND:
        return "\"and\"";
    case TokenType::OR:
        return "\"or'|";
    case TokenType::XOR:
        return "\"xor\"";
    case TokenType::NOT:
        return "\"not\"";
    case TokenType::EQ:
        return "\"==\"";
    case TokenType::END:
        return "\'$\'";
    }
    return "UNKNOWN";
}

Token::Token() : type(TokenType::END), val(-1) {}
Token::Token(TokenType t) : type(t), val(-1) {}
Token::Token(char symb) : type(TokenType::LETTER), val(symb) {}