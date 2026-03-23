#include <stdexcept>
#include <string>

#include "Error.hpp"

ParseError::ParseError(const std::string &message) : std::logic_error(message) {}