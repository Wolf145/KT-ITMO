#include <iostream>
#include <sstream>
#include <string>

#include "Node.hpp"
#include "Parser.hpp"
#include "Error.hpp"
#include "ParseResult.hpp"

ParseResult::ParseResult(const std::string &expr) 
{
   std::istringstream in(expr);

   try {
       Parser p(in);
       tree = p.parse();
   } catch (const ParseError &e) {
       tree = nullptr;
   }
}
