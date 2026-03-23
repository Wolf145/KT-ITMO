#pragma once

#include <stdexcept>
#include <string>

class ParseError : public std::logic_error
{
public:
    explicit ParseError(const std::string &);
};