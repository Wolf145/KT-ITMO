#include <antlr4-runtime.h>
#include "libitmoantlr/Translator.h"

#include "GrammarLexer.h"
#include "GrammarParser.h"
#include "GrammarBaseVisitor.h"

#include <any>
#include <sstream>
#include <string>
#include <functional>
#include <unordered_map>
#include <vector>
#include <regex>

namespace libitmoantlr
{
    std::string fname;
    bool isRef;

    std::vector<std::unordered_map<std::string, std::string>> symbolTable_;

    std::string process_table(const std::string &value)
    {
        for (auto it = symbolTable_.rbegin(); it != symbolTable_.rend(); ++it)
            if (it->count(value))
                return it->at(value);
        return std::string("UNKNOWN_TYPE");
    }

    struct VarType
    {
        std::string type;
        std::string name;
        std::string value;

        VarType() : name(""), value("") {}
        VarType(std::string name, std::string value) : name(std::move(name)), value(std::move(value)) {}
    };

    struct ArrayType
    {
        std::vector<int> start_ids;
        std::vector<std::string> dimensions;
        std::string type;

        ArrayType() : dimensions(), type("") {}
        ArrayType(std::vector<std::string> dimensions, std::string type) : dimensions(std::move(dimensions)), type(std::move(type)) {}
    };
}

void Translator::reset()
{
    out_.str(std::string());
    out_.clear();
    indent_ = 0;
    libitmoantlr::symbolTable_ = std::vector<std::unordered_map<std::string, std::string>>();
}

void Translator::emitLine(const std::string &s)
{
    for (int i = 0; i < indent_; ++i)
        out_ << "\t";
    out_ << s << "\n";
}

std::string Translator::getType(const std::string &value)
{
    if (value.find('\'') != std::string::npos)
        return std::string("char");
    if (value.find('"') != std::string::npos)
        return std::string("char * const");
    if (value == "true" || value == "false")
        return std::string("bool");
    if (std::regex_match(value, std::regex(R"(^[-+]?\d*\.(\d+)$)")) ||
        std::regex_match(value, std::regex(R"(^[-+]?((\d*\.(\d+))|\d+)[eE][-+]?(\d+)$)")))
        return std::string("double");
    if (std::regex_match(value, std::regex(R"(^\d+$)")))
        return std::string("unsigned int");
    if (std::regex_match(value, std::regex(R"(^-(\w+)$)")))
        return std::string("int");

    return libitmoantlr::process_table(value);
}

std::string Translator::proccessFormatTypes(const std::string &par)
{
    if (par == "char")
        return std::string("%c");
    if (par == "bool")
        return std::string("%d");
    if (par == "int")
        return std::string("%d");
    if (par == "unsigned int")
        return std::string("%u");
    if (par == "double")
        return std::string("%f");
    return "";
}

std::string Translator::constructAssignment(const std::string &name, const std::string &value, std::string constType)
{
    if (!constType.empty())
        constType += " ";
    if (constType == "#define ")
    {
        libitmoantlr::symbolTable_.back()[name] = getType(value);
        return constType + name + " " + value;
    }
    libitmoantlr::symbolTable_.back()[name] = getType(value);
    return constType + libitmoantlr::symbolTable_.back()[name] + " " + name + " = " + value + ";";
}

std::string Translator::pocessParams(std::vector<std::string> paramList)
{
    std::string expr;
    std::smatch match;

    for (size_t i = 0; i < paramList.size(); ++i)
    {
        std::string par = libitmoantlr::process_table(paramList[i]);
        expr += proccessFormatTypes(par);

        if (std::regex_match(par, std::regex(R"(^char\s+(\[\d+\]|\*\s*(const)?)$)")) || std::regex_match(paramList[i], std::regex(R"(^\".*\"$)")))
            expr += std::string("%s");
        if (std::regex_match(par, match, std::regex(R"(^(\w+)\s+\*$)")))
            expr += proccessFormatTypes(match[1].str());

        expr += (i + 1 < paramList.size() ? " " : "");
    }

    return expr;
}

std::string Translator::translate(const std::string &source, std::function<antlr4::tree::ParseTree *(GrammarParser &)> entry)
{
    reset();

    antlr4::ANTLRInputStream input(source);
    GrammarLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    GrammarParser parser(&tokens);

    antlr4::tree::ParseTree *tree = entry ? entry(parser) : parser.program();

    visit(tree);

    return out_.str();
}

std::any Translator::visitProgram(GrammarParser::ProgramContext *ctx)
{
    visit(ctx->block());
    return {};
}

std::any Translator::visitBlock(GrammarParser::BlockContext *ctx)
{
    libitmoantlr::symbolTable_.emplace_back();

    for (auto *cdp : ctx->constantDefinitionPart())
    {
        std::vector<libitmoantlr::VarType> constDef = std::any_cast<std::vector<libitmoantlr::VarType>>(visit(cdp));

        for (const auto &constDefBlock : constDef)
            emitLine(constructAssignment(constDefBlock.name, constDefBlock.value, "#define"));
    }
    for (auto *tdp : ctx->typeDefinitionPart())
        visit(tdp);
    for (auto *vdp : ctx->variableDeclarationPart())
        visit(vdp);
    for (auto *pfp : ctx->procedureAndFunctionDeclarationPart())
        visit(pfp);

    emitLine("int main() {");
    visit(ctx->compoundStatement());
    emitLine("}");

    libitmoantlr::symbolTable_.clear();
    return {};
}

std::any Translator::visitBlockInside(GrammarParser::BlockInsideContext *ctx)
{
    libitmoantlr::symbolTable_.emplace_back();

    indent();
    for (auto *cdp : ctx->constantDefinitionPart())
    {
        std::vector<libitmoantlr::VarType> constDef = std::any_cast<std::vector<libitmoantlr::VarType>>(visit(cdp));

        for (const auto &constDefBlock : constDef)
            emitLine(constructAssignment(constDefBlock.name, constDefBlock.value, "const"));
    }
    for (auto *vdp : ctx->variableDeclarationPart())
        visit(vdp);
    dedent();
    visit(ctx->compoundStatement());

    libitmoantlr::symbolTable_.pop_back();
    return {};
}

std::any Translator::visitConstantDefinitionPart(GrammarParser::ConstantDefinitionPartContext *ctx)
{
    std::vector<libitmoantlr::VarType> result;

    for (auto *cd : ctx->constantDefinition())
        result.push_back(
            std::any_cast<libitmoantlr::VarType>(visit(cd)));

    return result;
}

std::any Translator::visitConstantDefinition(GrammarParser::ConstantDefinitionContext *ctx)
{
    std::string name = std::any_cast<std::string>(visit(ctx->identifier()));
    std::string value = std::any_cast<std::string>(visit(ctx->constant()));

    return libitmoantlr::VarType(name, value);
}

std::any Translator::visitIdentifier(GrammarParser::IdentifierContext *ctx)
{
    if (ctx->IDENT())
    {
        std::string res = ctx->IDENT()->getSymbol()->getText();
        std::transform(res.begin(), res.end(), res.begin(), ::tolower);
        return res;
    }
    return {};
}

std::any Translator::visitConstant(GrammarParser::ConstantContext *ctx)
{
    if (ctx->unsignedNumber())
    {
        std::string num = std::any_cast<std::string>(visit(ctx->unsignedNumber()));
        if (ctx->sign())
        {
            std::string sgn = std::any_cast<std::string>(visit(ctx->sign()));
            num = sgn + num;
        }
        return num;
    }

    if (ctx->identifier())
    {
        std::string id = std::any_cast<std::string>(visit(ctx->identifier()));
        if (ctx->sign())
        {
            std::string sgn = std::any_cast<std::string>(visit(ctx->sign()));
            return sgn + id;
        }
        return id;
    }

    if (ctx->string())
        return visit(ctx->string());

    if (ctx->constantChr())
        return visit(ctx->constantChr());

    if (ctx->bool_())
        return visit(ctx->bool_());

    return std::string("");
}

std::any Translator::visitConstantChr(GrammarParser::ConstantChrContext *ctx)
{
    return "'" +
           std::string(1, static_cast<char>(std::stoi(std::any_cast<std::string>(visit(ctx->unsignedInteger()))))) + "'";
}

std::any Translator::visitUnsignedInteger(GrammarParser::UnsignedIntegerContext *ctx)
{
    if (ctx->NUM_INT())
        return ctx->NUM_INT()->getSymbol()->getText();
    return std::string("");
}

std::any Translator::visitString(GrammarParser::StringContext *ctx)
{
    if (ctx->STRING_LITERAL())
    {
        std::string res = ctx->STRING_LITERAL()->getSymbol()->getText();
        res = res.substr(1, res.size() - 2);
        res = "\"" + res + "\"";
        return res;
    }
    return std::string("");
}

std::any Translator::visitSign(GrammarParser::SignContext *ctx)
{
    if (ctx->MINUS())
        return std::string("-");
    return {};
}

std::any Translator::visitUnsignedNumber(GrammarParser::UnsignedNumberContext *ctx)
{
    if (ctx->unsignedInteger())
        return visit(ctx->unsignedInteger());
    if (ctx->unsignedReal())
        return visit(ctx->unsignedReal());
    return {};
}

std::any Translator::visitUnsignedReal(GrammarParser::UnsignedRealContext *ctx)
{
    if (ctx->NUM_REAL())
        return ctx->NUM_REAL()->getSymbol()->getText();
    return {};
}

std::any Translator::visitBool_(GrammarParser::Bool_Context *ctx)
{
    if (ctx->TRUE())
        return std::string("true");
    if (ctx->FALSE())
        return std::string("false");
    return std::string("");
}

std::any Translator::visitTypeDefinitionPart(GrammarParser::TypeDefinitionPartContext *ctx)
{
    for (auto *td : ctx->typeDefinition())
        visit(td);
    return {};
}

std::any Translator::visitTypeDefinition(GrammarParser::TypeDefinitionContext *ctx)
{
    std::string name = std::any_cast<std::string>(visit(ctx->identifier()));

    if (ctx->scalarType())
    {
        emitLine("typedef enum {");
        visit(ctx->scalarType());
        emitLine("} " + name + ";");
        return {};
    }

    if (ctx->arrayType())
    {
        if (ctx->arrayType()->type_())
        {
            if (ctx->arrayType()->type_() && ctx->arrayType()->type_()->recordType())
                emitLine("typedef struct {");
            if (ctx->arrayType()->type_() &&
                ctx->arrayType()->type_()->simpleType() &&
                ctx->arrayType()->type_()->simpleType()->scalarType())
                emitLine("typedef enum {");

            libitmoantlr::ArrayType arrayType = std::any_cast<libitmoantlr::ArrayType>(visit(ctx->arrayType()));

            for (const std::string &dim : arrayType.dimensions)
                name += "[" + dim + "]";

            emitLine("} " + name + ";");
            return {};
        }

        libitmoantlr::ArrayType arrayType = std::any_cast<libitmoantlr::ArrayType>(visit(ctx->arrayType()));
        name = "typedef " + arrayType.type + " " + name;
        for (const std::string &dim : arrayType.dimensions)
            name += "[" + dim + "]";
        emitLine(name + ";");

        return {};
    }
    if (ctx->recordType())
    {
        emitLine("typedef struct " + name + " {");
        visit(ctx->recordType());
        emitLine("} " + name + ";");

        return {};
    }
    return {};
}

std::any Translator::visitArrayType(GrammarParser::ArrayTypeContext *ctx)
{
    std::vector<std::string> dimensions = std::any_cast<std::vector<std::string>>(visit(ctx->typeList()));
    std::string type;

    type = std::any_cast<std::string>(visit(ctx->type_()));

    return libitmoantlr::ArrayType(dimensions, type);
}

std::any Translator::visitTypeList(GrammarParser::TypeListContext *ctx)
{
    std::vector<std::string> dimensions;
    for (auto *rl : ctx->rangeList())
        dimensions.push_back(std::any_cast<std::string>(visit(rl)));
    return dimensions;
}

std::any Translator::visitRangeList(GrammarParser::RangeListContext *ctx)
{
    std::string to = std::any_cast<std::string>(visit(ctx->unsignedInteger().back()));
    return std::to_string(std::stoi(to));
}

std::any Translator::visitType_(GrammarParser::Type_Context *ctx)
{
    if (ctx->simpleType())
        return visit(ctx->simpleType());
    if (ctx->arrayType())
        return visit(ctx->arrayType());
    if (ctx->pointerType())
        return visit(ctx->pointerType());
    if (ctx->recordType())
        return visit(ctx->recordType());
    return std::string("");
}

std::any Translator::visitPointerType(GrammarParser::PointerTypeContext *ctx)
{
    std::string type = std::any_cast<std::string>(visit(ctx->typeIdentifier()));
    if (type == "char [256]")
        return std::string("char **");
    type += " ";
    for (size_t i = 0; i < ctx->POINTER().size(); ++i)
        type += "*";
    return type;
}

std::any Translator::visitSimpleType(GrammarParser::SimpleTypeContext *ctx)
{
    if (ctx->scalarType())
        return visit(ctx->scalarType());
    if (ctx->typeIdentifier())
        return visit(ctx->typeIdentifier());
    if (ctx->stringType())
        return visit(ctx->stringType());
    return {};
}

std::any Translator::visitTypeIdentifier(GrammarParser::TypeIdentifierContext *ctx)
{
    if (ctx->CHAR())
        return std::string("char");
    if (ctx->INTEGER())
        return std::string("int");
    if (ctx->REAL())
        return std::string("double");
    if (ctx->BOOLEAN())
        return std::string("bool");
    if (ctx->STRING())
        return std::string("char [256]");
    if (ctx->identifier())
        return std::any_cast<std::string>(visit(ctx->identifier()));
    return std::string("");
}

std::any Translator::visitScalarType(GrammarParser::ScalarTypeContext *ctx)
{
    std::vector<std::string> enumValues = std::any_cast<std::vector<std::string>>(visit(ctx->identifierList()));

    indent();
    for (size_t i = 0; i < enumValues.size(); ++i)
    {
        const std::string &value = enumValues[i];
        emitLine(value + (i + 1 < enumValues.size() ? ", " : ""));
    }
    dedent();

    return std::string("enum");
}

std::any Translator::visitStringType(GrammarParser::StringTypeContext *ctx)
{
    if (ctx->identifier())
        return "char [" + std::any_cast<std::string>(visit(ctx->identifier())) + "]";
    if (ctx->unsignedInteger())
        return "char [" + std::any_cast<std::string>(visit(ctx->unsignedInteger())) + "]";
    return {};
}

std::any Translator::visitRecordType(GrammarParser::RecordTypeContext *ctx)
{
    libitmoantlr::symbolTable_.emplace_back();

    indent();
    visit(ctx->fixedPart());
    dedent();

    libitmoantlr::symbolTable_.pop_back();
    return std::string("struct");
}

std::any Translator::visitFixedPart(GrammarParser::FixedPartContext *ctx)
{
    for (auto *vdp : ctx->recordSection())
        visit(vdp);
    return {};
}

std::any Translator::visitRecordSection(GrammarParser::RecordSectionContext *ctx)
{
    std::vector<std::string> identList = std::any_cast<std::vector<std::string>>(visit(ctx->identifierList()));
    std::string identListStr;

    auto *getType = ctx->type_();

    if (getType->arrayType())
    {
        if (getType->arrayType()->type_() && getType->arrayType()->type_()->recordType())
            emitLine("struct {");
        if (getType->arrayType()->type_() &&
            getType->arrayType()->type_()->simpleType() &&
            getType->arrayType()->type_()->simpleType()->scalarType())
            emitLine("enum {");

        libitmoantlr::ArrayType arrayType = std::any_cast<libitmoantlr::ArrayType>(visit(getType->arrayType()));

        std::string baseType = ((arrayType.type == "struct" || arrayType.type == "enum") ? "}" : arrayType.type);
        std::vector<std::string> dimensions = arrayType.dimensions;

        for (size_t i = 0; i < identList.size(); ++i)
        {
            const std::string &ident = identList[i];
            libitmoantlr::symbolTable_.back()[ident] = arrayType.type;
            identListStr += ident;
            for (std::string dim : dimensions)
            {
                identListStr += "[" + dim + "]";
                libitmoantlr::symbolTable_.back()[ident] += " [" + dim + "]";
            }
            identListStr += (i + 1 < identList.size() ? ", " : "");
        }

        emitLine(baseType + " " + identListStr + ";");
        return {};
    }

    if (getType->recordType())
    {
        for (const std::string &ident : identList)
        {
            emitLine("struct " + ident + " {");
            visit(getType);
            emitLine("};");
            libitmoantlr::symbolTable_.pop_back();
            libitmoantlr::symbolTable_.back()[ident] = "struct";
        }

        return {};
    }

    if (getType->simpleType() && getType->simpleType()->scalarType())
    {
        for (const std::string &ident : identList)
        {
            emitLine("enum " + ident + " {");
            visit(ctx->type_());
            emitLine("};");
            libitmoantlr::symbolTable_.back()[ident] = "enum";
        }

        return {};
    }

    std::string type;
    std::any a = visit(getType);
    if (a.type() == typeid(std::string))
        type = std::any_cast<std::string>(a);

    std::smatch match;

    for (size_t i = 0; i < identList.size(); ++i)
    {
        const std::string &ident = identList[i];
        libitmoantlr::symbolTable_.back()[ident] = type;
    }

    if (std::regex_match(type, match, std::regex(R"(^(\w+) \[(\w+)\]$)")))
    {
        std::string baseType = match[1].str();
        std::string size = match[2].str();

        for (size_t i = 0; i < identList.size(); ++i)
        {
            const std::string &ident = identList[i];
            identListStr += ident + "[" + size + "]" + (i + 1 < identList.size() ? ", " : "");
        }

        emitLine(baseType + " " + identListStr + ";");
        return {};
    }

    if (std::regex_match(type, match, std::regex(R"(^(\w+) (\*+)$)")))
    {
        std::string baseType = match[1].str();
        std::string pointerLevel = match[2].str();

        for (size_t i = 0; i < identList.size(); ++i)
        {
            const std::string &ident = identList[i];
            identListStr += pointerLevel + ident + (i + 1 < identList.size() ? ", " : "");
        }

        emitLine(baseType + " " + identListStr + ";");
        return {};
    }

    for (size_t i = 0; i < identList.size(); ++i)
    {
        const std::string &ident = identList[i];
        identListStr += ident + (i + 1 < identList.size() ? ", " : "");
    }

    emitLine(type + " " + identListStr + ";");
    return {};
}

std::any Translator::visitIdentifierList(GrammarParser::IdentifierListContext *ctx)
{
    if (ctx->VAR())
        libitmoantlr::isRef = true;
    std::vector<std::string> result;

    for (size_t i = 0; i < ctx->identifier().size(); ++i)
        result.push_back(std::any_cast<std::string>(visit(ctx->identifier(i))));

    return result;
}

std::any Translator::visitVariableDeclarationPart(GrammarParser::VariableDeclarationPartContext *ctx)
{
    for (auto *vdp : ctx->variableDeclaration())
        visit(vdp);
    return {};
}

std::any Translator::visitVariableDeclaration(GrammarParser::VariableDeclarationContext *ctx)
{
    std::vector<std::string> identList = std::any_cast<std::vector<std::string>>(visit(ctx->identifierList()));
    std::string identListStr;

    auto *getType = ctx->type_();

    if (getType->arrayType())
    {
        if (getType->arrayType()->type_() && getType->arrayType()->type_()->recordType())
            emitLine("struct {");
        if (getType->arrayType()->type_() &&
            getType->arrayType()->type_()->simpleType() &&
            getType->arrayType()->type_()->simpleType()->scalarType())
            emitLine("enum {");

        libitmoantlr::ArrayType arrayType = std::any_cast<libitmoantlr::ArrayType>(visit(ctx->type_()));

        std::string baseType = ((arrayType.type == "struct" || arrayType.type == "enum") ? "}" : arrayType.type);
        std::vector<std::string> dimensions = arrayType.dimensions;

        for (size_t i = 0; i < identList.size(); ++i)
        {
            const std::string &ident = identList[i];
            libitmoantlr::symbolTable_.back()[ident] = arrayType.type;
            identListStr += ident;
            for (std::string dim : dimensions)
            {
                identListStr += "[" + dim + "]";
                libitmoantlr::symbolTable_.back()[ident] += " [" + dim + "]";
            }
            identListStr += (i + 1 < identList.size() ? ", " : "");
        }

        emitLine(baseType + " " + identListStr + ";");
        return {};
    }
    if (getType->recordType())
    {
        for (const std::string &ident : identList)
        {
            emitLine("struct " + ident + " {");
            visit(getType);
            emitLine("};");

            libitmoantlr::symbolTable_.pop_back();
            libitmoantlr::symbolTable_.back()[ident] = "struct";
        }

        return {};
    }

    if (getType->simpleType() && getType->simpleType()->scalarType())
    {
        for (const std::string &ident : identList)
        {
            emitLine("enum " + ident + " {");
            visit(ctx->type_());
            emitLine("};");
            libitmoantlr::symbolTable_.back()[ident] = "enum";
        }

        return {};
    }

    std::string type;
    std::any a = visit(getType);
    if (a.type() == typeid(std::string))
        type = std::any_cast<std::string>(a);

    std::smatch match;

    for (size_t i = 0; i < identList.size(); ++i)
    {
        const std::string &ident = identList[i];
        libitmoantlr::symbolTable_.back()[ident] = type;
    }

    if (std::regex_match(type, match, std::regex(R"(^(\w+) \[(\w+)\]$)")))
    {
        std::string baseType = match[1].str();
        std::string size = match[2].str();

        for (size_t i = 0; i < identList.size(); ++i)
        {
            const std::string &ident = identList[i];
            identListStr += ident + "[" + size + "]" + (i + 1 < identList.size() ? ", " : "");
        }

        emitLine(baseType + " " + identListStr + ";");
        return {};
    }

    if (std::regex_match(type, match, std::regex(R"(^(\w+) (\*+)$)")))
    {
        std::string baseType = match[1].str();
        std::string pointerLevel = match[2].str();

        for (size_t i = 0; i < identList.size(); ++i)
        {
            const std::string &ident = identList[i];
            identListStr += pointerLevel + ident + (i + 1 < identList.size() ? ", " : "");
        }

        emitLine(baseType + " " + identListStr + ";");
        return {};
    }

    for (size_t i = 0; i < identList.size(); ++i)
    {
        const std::string &ident = identList[i];
        identListStr += ident + (i + 1 < identList.size() ? ", " : "");
    }

    emitLine(type + " " + identListStr + ";");
    return {};
}

std::any Translator::visitProcedureAndFunctionDeclarationPart(GrammarParser::ProcedureAndFunctionDeclarationPartContext *ctx)
{
    visit(ctx->procedureOrFunctionDeclaration());
    return {};
}

std::any Translator::visitProcedureOrFunctionDeclaration(GrammarParser::ProcedureOrFunctionDeclarationContext *ctx)
{
    libitmoantlr::symbolTable_.emplace_back();

    if (ctx->procedureDeclaration())
        visit(ctx->procedureDeclaration());
    else if (ctx->functionDeclaration())
        visit(ctx->functionDeclaration());

    libitmoantlr::symbolTable_.pop_back();
    return {};
}

std::any Translator::visitProcedureDeclaration(GrammarParser::ProcedureDeclarationContext *ctx)
{
    std::string procName = std::any_cast<std::string>(visit(ctx->identifier()));

    if (ctx->formalParameterList() == nullptr)
    {
        emitLine("void " + procName + "() {");
        visit(ctx->blockInside());
        emitLine("}");
        return {};
    }
    std::vector<std::string> paramList = std::any_cast<std::vector<std::string>>(visit(ctx->formalParameterList()));

    std::string paramStr;
    for (size_t i = 0; i < paramList.size(); ++i)
        paramStr += paramList[i] + (i + 1 < paramList.size() ? ", " : "");
    emitLine("void " + procName + "(" + paramStr + ") {");
    visit(ctx->blockInside());
    emitLine("}");
    return {};
}

std::any Translator::visitFormalParameterList(GrammarParser::FormalParameterListContext *ctx)
{
    std::vector<std::string> paramList;
    for (auto *fps : ctx->parameterGroup())
    {
        std::vector<std::string> paramSubList = std::any_cast<std::vector<std::string>>(visit(fps));
        paramList.insert(paramList.end(), paramSubList.begin(), paramSubList.end());
    }
    return paramList;
}

std::any Translator::visitParameterGroup(GrammarParser::ParameterGroupContext *ctx)
{
    std::vector<std::string> paramList;
    std::vector<std::string> identList = std::any_cast<std::vector<std::string>>(visit(ctx->identifierList()));
    std::string type = std::any_cast<std::string>(visit(ctx->typeIdentifier()));

    if (type == "char [256]")
        type = "char *";

    for (const auto &ident : identList)
    {
        paramList.push_back(type + " " + (libitmoantlr::isRef ? "&" : "") + ident);
        libitmoantlr::symbolTable_.back()[ident] = type;
    }
    
    libitmoantlr::isRef = false;
    return paramList;
}

std::any Translator::visitFunctionDeclaration(GrammarParser::FunctionDeclarationContext *ctx)
{
    std::string type = std::any_cast<std::string>(visit(ctx->resultType()));
    std::string procName = std::any_cast<std::string>(visit(ctx->identifier()));

    libitmoantlr::fname = procName;

    if (ctx->formalParameterList() == nullptr)
    {
        emitLine(type + " " + procName + "() {");
        indent();
        emitLine(type + " returnVal;");
        dedent();
        visit(ctx->blockInside());
        indent();
        emitLine("return returnVal;");
        dedent();
        emitLine("}");
        return {};
    }
    std::vector<std::string> paramList = std::any_cast<std::vector<std::string>>(visit(ctx->formalParameterList()));

    std::string paramStr;
    for (size_t i = 0; i < paramList.size(); ++i)
        paramStr += paramList[i] + (i + 1 < paramList.size() ? ", " : "");

    emitLine(type + " " + procName + "(" + paramStr + ") {");
    indent();
    emitLine(type + " returnVal;");
    dedent();
    visit(ctx->blockInside());
    indent();
    emitLine("return returnVal;");
    dedent();
    emitLine("}");
    return {};
}

std::any Translator::visitResultType(GrammarParser::ResultTypeContext *ctx)
{
    return visit(ctx->typeIdentifier());
}

std::any Translator::visitCompoundStatement(GrammarParser::CompoundStatementContext *ctx)
{
    libitmoantlr::symbolTable_.emplace_back();

    indent();
    visit(ctx->statements());
    dedent();

    libitmoantlr::symbolTable_.pop_back();
    return {};
}

std::any Translator::visitStatements(GrammarParser::StatementsContext *ctx)
{
    for (auto *stmt : ctx->statement())
        visit(stmt);
    return {};
}

std::any Translator::visitStatement(GrammarParser::StatementContext *ctx)
{
    if (ctx->simpleStatement())
        visit(ctx->simpleStatement());
    if (ctx->structuredStatement())
        visit(ctx->structuredStatement());
    return {};
}

std::any Translator::visitSimpleStatement(GrammarParser::SimpleStatementContext *ctx)
{
    if (ctx->assignmentStatement())
        visit(ctx->assignmentStatement());
    if (ctx->procedureStatement())
        visit(ctx->procedureStatement());
    if (ctx->BREAK())
        emitLine("break;");
    if (ctx->CONTINUE())
        emitLine("continue;");
    return {};
}

std::any Translator::visitAssignmentStatement(GrammarParser::AssignmentStatementContext *ctx)
{
    std::string var = std::any_cast<std::string>(visit(ctx->variable()));
    if (var == libitmoantlr::fname)
        var = "returnVal";
    std::string expr = std::any_cast<std::string>(visit(ctx->expression()));
    emitLine(var + " = " + expr + ";");
    return {};
}

std::any Translator::visitVariable(GrammarParser::VariableContext *ctx)
{
    std::string varName;

    if (ctx->AT())
        varName += "&";

    varName += std::any_cast<std::string>(visit(ctx->identifier()));

    for (const auto &acs : ctx->access())
    {
        if (acs->ptrAccess())
        {
            varName = "(*" + varName + ")";
            continue;
        }
        varName += std::any_cast<std::string>(visit(acs));
    }

    return varName;
}

std::any Translator::visitAccess(GrammarParser::AccessContext *ctx)
{
    if (ctx->arrayAccess())
        return visit(ctx->arrayAccess());
    if (ctx->structAccess())
        return visit(ctx->structAccess());
    return {};
}

std::any Translator::visitArrayAccess(GrammarParser::ArrayAccessContext *ctx)
{
    std::string accessStr;
    for (const auto &vexpr : ctx->expression())
    {
        std::string expr = std::any_cast<std::string>(visit(vexpr));
        accessStr += "[" + expr + "]";
    }
    return accessStr;
}

std::any Translator::visitStructAccess(GrammarParser::StructAccessContext *ctx)
{
    return "." + std::any_cast<std::string>(visit(ctx->identifier()));
}

std::any Translator::visitExpression(GrammarParser::ExpressionContext *ctx)
{
    std::string expr = std::any_cast<std::string>(visit(ctx->simpleExpression()));

    if (ctx->relationaloperator())
        expr += " " + std::any_cast<std::string>(visit(ctx->relationaloperator())) + " " + std::any_cast<std::string>(visit(ctx->expression()));

    return expr;
}

std::any Translator::visitRelationaloperator(GrammarParser::RelationaloperatorContext *ctx)
{
    if (ctx->EQUAL())
        return std::string("==");
    if (ctx->NOT_EQUAL())
        return std::string("!=");
    if (ctx->LT())
        return std::string("<");
    if (ctx->LE())
        return std::string("<=");
    if (ctx->GT())
        return std::string(">");
    if (ctx->GE())
        return std::string(">=");
    return std::string("");
}

std::any Translator::visitSimpleExpression(GrammarParser::SimpleExpressionContext *ctx)
{
    std::string expr = std::any_cast<std::string>(visit(ctx->term()));

    if (ctx->additiveoperator())
        expr += " " + std::any_cast<std::string>(visit(ctx->additiveoperator())) + " " + std::any_cast<std::string>(visit(ctx->simpleExpression()));

    return expr;
}

std::any Translator::visitAdditiveoperator(GrammarParser::AdditiveoperatorContext *ctx)
{
    if (ctx->PLUS())
        return std::string("+");
    if (ctx->MINUS())
        return std::string("-");
    if (ctx->OR())
        return std::string("||");
    return std::string("");
}

std::any Translator::visitTerm(GrammarParser::TermContext *ctx)
{
    std::string expr = std::any_cast<std::string>(visit(ctx->signedFactor()));

    if (ctx->multiplicativeoperator())
        expr += " " + std::any_cast<std::string>(visit(ctx->multiplicativeoperator())) + " " + std::any_cast<std::string>(visit(ctx->term()));

    return expr;
}

std::any Translator::visitMultiplicativeoperator(GrammarParser::MultiplicativeoperatorContext *ctx)
{
    if (ctx->STAR())
        return std::string("*");
    if (ctx->SLASH() || ctx->DIV())
        return std::string("/");
    if (ctx->MOD())
        return std::string("%");
    if (ctx->AND())
        return std::string("&&");
    return std::string("");
}

std::any Translator::visitSignedFactor(GrammarParser::SignedFactorContext *ctx)
{
    std::string expr;

    if (ctx->PLUS())
        expr += "+";
    if (ctx->MINUS())
        expr += "-";

    return expr + std::any_cast<std::string>(visit(ctx->factor()));
}

std::any Translator::visitFactor(GrammarParser::FactorContext *ctx)
{
    if (ctx->variable())
        return visit(ctx->variable());
    if (ctx->expression())
        return "(" + std::any_cast<std::string>(visit(ctx->expression())) + ")";
    if (ctx->functionDesignator())
        return visit(ctx->functionDesignator());
    if (ctx->unsignedConstant())
        return visit(ctx->unsignedConstant());
    if (ctx->NOT())
        return "!" + std::any_cast<std::string>(visit(ctx->factor()));
    if (ctx->bool_())
        return std::any_cast<std::string>(visit(ctx->bool_()));
    if (ctx->string())
        return std::any_cast<std::string>(visit(ctx->string()));
    return std::string("");
}

std::any Translator::visitFunctionDesignator(GrammarParser::FunctionDesignatorContext *ctx)
{
    std::string expr = std::any_cast<std::string>(visit(ctx->identifier())) + "(";
    std::vector<std::string> paramList;

    if (ctx->parameterList())
        paramList = std::any_cast<std::vector<std::string>>(visit(ctx->parameterList()));

    for (size_t i = 0; i < paramList.size(); i++)
        expr += paramList[i] + (i + 1 < paramList.size() ? ", " : "");

    return expr + ")";
}

std::any Translator::visitParameterList(GrammarParser::ParameterListContext *ctx)
{
    std::vector<std::string> params;
    for (auto *ec : ctx->expression())
        params.push_back(std::any_cast<std::string>(visit(ec)));

    return params;
}

std::any Translator::visitProcedureStatement(GrammarParser::ProcedureStatementContext *ctx)
{
    std::string expr = std::any_cast<std::string>(visit(ctx->identifier())) + "(";
    std::vector<std::string> paramList;
    bool needamp = (expr == "read(" || expr == "readln(");

    if (ctx->parameterList())
        paramList = std::any_cast<std::vector<std::string>>(visit(ctx->parameterList()));

    if (needamp)
    {
        expr = std::string("scanf(\"") + pocessParams(paramList) + (expr == "readln(" ? "\\n" : "") + "\"";
        if (ctx->parameterList())
            expr += ", ";
    }

    if (expr == "write(" || expr == "writeln(")
    {
        expr = std::string("printf(\"") + pocessParams(paramList) + (expr == "writeln(" ? "\\n" : "") + "\"";
        if (ctx->parameterList())
            expr += ", ";
    }

    for (size_t i = 0; i < paramList.size(); i++)
        expr += ((needamp && !std::regex_match(libitmoantlr::process_table(paramList[i]), std::regex(R"(^\w+ \*+$)")))
                     ? "&"
                     : "") +
                paramList[i] + (i + 1 < paramList.size() ? ", " : "");

    emitLine(expr + ");");
    return {};
}

std::any Translator::visitUnsignedConstant(GrammarParser::UnsignedConstantContext *ctx)
{
    if (ctx->NIL())
        return std::string("NULL");
    if (ctx->string())
        return visit(ctx->string());
    if (ctx->constantChr())
        return visit(ctx->constantChr());
    if (ctx->unsignedNumber())
        return visit(ctx->unsignedNumber());

    return {};
}

std::any Translator::visitStructuredStatement(GrammarParser::StructuredStatementContext *ctx)
{
    if (ctx->compoundStatement())
    {
        emitLine("{");
        visit(ctx->compoundStatement());
        emitLine("}");
    }
    if (ctx->conditionalStatement())
        visit(ctx->conditionalStatement());
    if (ctx->repetetiveStatement())
        visit(ctx->repetetiveStatement());
    return {};
}

std::any Translator::visitConditionalStatement(GrammarParser::ConditionalStatementContext *ctx)
{
    if (ctx->ifStatement())
        visit(ctx->ifStatement());
    if (ctx->switchStatement())
        visit(ctx->switchStatement());
    return {};
}

std::any Translator::visitIfStatement(GrammarParser::IfStatementContext *ctx)
{
    std::string expr = std::any_cast<std::string>(visit(ctx->expression()));
    emitLine("if (" + expr + ") {");
    if (ctx->statement()->structuredStatement() &&
        ctx->statement()->structuredStatement()->compoundStatement())
    {
        visit(ctx->statement()->structuredStatement()->compoundStatement());
    }
    else
    {
        indent();
        visit(ctx->statement());
        dedent();
    }
    for (auto *ifctx : ctx->elseIfStatement())
    {
        emitLine("} else if (" + std::any_cast<std::string>(visit(ifctx->expression())) + ") {");
        visit(ifctx);
    }
    if (ctx->elseStatement())
    {
        emitLine("} else {");
        visit(ctx->elseStatement());
    }
    emitLine("}");

    return {};
}

std::any Translator::visitElseIfStatement(GrammarParser::ElseIfStatementContext *ctx)
{
    if (ctx->statement()->structuredStatement() &&
        ctx->statement()->structuredStatement()->compoundStatement())
    {
        visit(ctx->statement()->structuredStatement()->compoundStatement());
        return {};
    }
    indent();
    visit(ctx->statement());
    dedent();
    return {};
}

std::any Translator::visitElseStatement(GrammarParser::ElseStatementContext *ctx)
{
    if (ctx->statement()->structuredStatement() &&
        ctx->statement()->structuredStatement()->compoundStatement())
    {
        visit(ctx->statement()->structuredStatement()->compoundStatement());
        return {};
    }
    indent();
    visit(ctx->statement());
    dedent();
    return {};
}

std::any Translator::visitSwitchStatement(GrammarParser::SwitchStatementContext *ctx)
{
    std::string expr = std::any_cast<std::string>(visit(ctx->expression()));
    emitLine("switch (" + expr + ") {");
    indent();

    for (auto *cle : ctx->caseListElement())
        visit(cle);

    if (ctx->defaultStatement())
    {
        emitLine("default: {");
        visit(ctx->defaultStatement());
        emitLine("}");
    }

    dedent();
    emitLine("}");

    return {};
}

std::any Translator::visitCaseListElement(GrammarParser::CaseListElementContext *ctx)
{
    std::vector<std::string> constList = std::any_cast<std::vector<std::string>>(visit(ctx->constList()));

    for (size_t i = 0; i + 1 < constList.size(); ++i)
        emitLine("сase " + constList[i] + ":");

    emitLine("сase " + constList.back() + ": {");
    if (ctx->statement()->structuredStatement() &&
        ctx->statement()->structuredStatement()->compoundStatement())
    {
        visit(ctx->statement()->structuredStatement()->compoundStatement());
    }
    else
    {
        indent();
        visit(ctx->statement());
        dedent();
    }
    indent();
    emitLine("break;");
    dedent();
    emitLine("}");

    return {};
}

std::any Translator::visitConstList(GrammarParser::ConstListContext *ctx)
{
    std::vector<std::string> csts;

    for (auto *cnst : ctx->constant())
        csts.push_back(std::any_cast<std::string>(visit(cnst)));

    return csts;
}

std::any Translator::visitDefaultStatement(GrammarParser::DefaultStatementContext *ctx)
{
    indent();
    visit(ctx->statement());
    dedent();
    return {};
}

std::any Translator::visitRepetetiveStatement(GrammarParser::RepetetiveStatementContext *ctx)
{
    if (ctx->whileStatement())
        visit(ctx->whileStatement());
    if (ctx->repeatStatement())
        visit(ctx->repeatStatement());
    if (ctx->forStatement())
        visit(ctx->forStatement());
    return {};
}

std::any Translator::visitWhileStatement(GrammarParser::WhileStatementContext *ctx)
{
    emitLine("while(" + std::any_cast<std::string>(visit(ctx->expression())) + ") {");
    if (ctx->statement()->structuredStatement() &&
        ctx->statement()->structuredStatement()->compoundStatement())
    {
        visit(ctx->statement()->structuredStatement()->compoundStatement());
        emitLine("}");
        return {};
    }
    indent();
    visit(ctx->statement());
    dedent();
    emitLine("}");
    return {};
}

std::any Translator::visitRepeatStatement(GrammarParser::RepeatStatementContext *ctx)
{
    emitLine("do {");
    visit(ctx->statements());
    emitLine("} while(" + std::any_cast<std::string>(visit(ctx->expression())) + ")");
    return {};
}

std::any Translator::visitForStatement(GrammarParser::ForStatementContext *ctx)
{
    emitLine("for (" + std::any_cast<std::string>(visit(ctx->forList())) + ") {");
    if (ctx->statement()->structuredStatement() &&
        ctx->statement()->structuredStatement()->compoundStatement())
    {
        visit(ctx->statement()->structuredStatement()->compoundStatement());
        emitLine("}");
        return {};
    }
    indent();
    visit(ctx->statement());
    dedent();
    emitLine("}");

    return {};
}

std::any Translator::visitForList(GrammarParser::ForListContext *ctx)
{
    std::string forVar = std::any_cast<std::string>(visit(ctx->identifier()));
    std::string from = std::any_cast<std::string>(visit(ctx->initialValue()));
    std::string to = std::any_cast<std::string>(visit(ctx->finalValue()));

    std::string res = forVar + " = " + from + "; " +
                      forVar + (ctx->TO() ? " <= " : " >= ") + to + "; " + forVar + "++";

    return res;
}

std::any Translator::visitInitialValue(GrammarParser::InitialValueContext *ctx)
{
    return std::any_cast<std::string>(visit(ctx->expression()));
}

std::any Translator::visitFinalValue(GrammarParser::FinalValueContext *ctx)
{
    return std::any_cast<std::string>(visit(ctx->expression()));
}
