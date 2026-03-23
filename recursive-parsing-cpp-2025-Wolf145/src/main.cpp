#include <iostream>
#include <stdexcept>
#include <fstream>
#include <string>

#include "Token.hpp"
#include "Lexer.hpp"
#include "Error.hpp"
#include "Parser.hpp"

void execLexer(std::istream &in, std::ofstream &fout)
{
    LexicalAnalyzer lex(in);
    try
    {
        lex.parse();
    }
    catch (const ParseError &e)
    {
        std::cerr << e.what() << std::endl;
        goto fail;
    }
    while (true)
    {
        try
        {
            lex.nextToken();
        }
        catch (const ParseError &e)
        {
            std::cerr << e.what() << std::endl;
            goto fail;
        }
        std::cout << lex.getCurToken().to_string() << "\n";
        if (lex.getCurToken().type == TokenType::END)
            break;
    }
    fout << "Parsed." << std::endl;
    return;
fail:
    fout << "Not Parsed." << std::endl;
    return;
}
void visualize(std::ofstream &fout, const Node::NodePtr &tree)
{
    for (const auto &child : tree->getChildren())
    {
        fout << "\t" << child->getId() << " [label = \"" << child->getNodeName() << "\"];\n";
        fout << "\t\"" << tree->getId() << "\" -> \"" << child->getId() << "\"" << std::endl;
        visualize(fout, child);
    }
}
Node::NodePtr execParser(std::istream &in, std::ofstream &fout)
{
    Parser p(in);
    Node::NodePtr tree;
    try
    {
        tree = p.parse();
    }
    catch (const ParseError &e)
    {
        fout << "Not parsed." << std::endl;
        std::cerr << e.what() << std::endl;
        return nullptr;
    }

    fout << "Parsed." << std::endl;
    return tree;
}

int main()
{
    std::ifstream fin("./input.txt");
    std::ofstream fout("./output.txt");

    Node::NodePtr tree = execParser(fin, fout);

    if (tree != nullptr) {
        std::ofstream dot("./tree.dot");
        dot << "digraph G {" << std::endl;
        dot << "\t" << tree->getId() << " [label = \"" << tree->getNodeName() << "\"];\n";
        visualize(dot, tree);
        dot << "}" << std::endl;
    }
}
