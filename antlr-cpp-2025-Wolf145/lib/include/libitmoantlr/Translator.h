#ifndef LIBITMOANTLR_TRANSLATOR_H
#define LIBITMOANTLR_TRANSLATOR_H

#include <antlr4-runtime.h>

#include "GrammarLexer.h"
#include "GrammarParser.h"
#include "GrammarBaseVisitor.h"

#include <any>
#include <sstream>
#include <string>
#include <functional>
#include <unordered_map>
#include <vector>

class Translator : public GrammarBaseVisitor
{
public:
    Translator() { reset(); }
    std::string translate(const std::string &source, std::function<antlr4::tree::ParseTree *(GrammarParser &)> entry = nullptr);

private:
    std::ostringstream out_;
    int indent_;

    void reset();
    void emitLine(const std::string &s = "");
    void indent() { ++indent_; }
    void dedent() { (indent_ ? --indent_ : indent_); }
    std::string getType(const std::string &value);
    std::string constructAssignment(const std::string &name, const std::string &value, std::string isnConst = "");
    std::string pocessParams(std::vector<std::string> paramList);
    std::string proccessFormatTypes(const std::string &par);

    //--

    std::any visitProgram(GrammarParser::ProgramContext *ctx) override;

    std::any visitIdentifier(GrammarParser::IdentifierContext *ctx) override;

    std::any visitBlock(GrammarParser::BlockContext *ctx) override;

    std::any visitBlockInside(GrammarParser::BlockInsideContext *ctx) override;

    std::any visitConstantDefinitionPart(GrammarParser::ConstantDefinitionPartContext *ctx) override;

    std::any visitConstantDefinition(GrammarParser::ConstantDefinitionContext *ctx) override;

    std::any visitConstantChr(GrammarParser::ConstantChrContext *ctx) override;

    std::any visitConstant(GrammarParser::ConstantContext *ctx) override;

    std::any visitUnsignedNumber(GrammarParser::UnsignedNumberContext *ctx) override;

    std::any visitUnsignedInteger(GrammarParser::UnsignedIntegerContext *ctx) override;

    std::any visitUnsignedReal(GrammarParser::UnsignedRealContext *ctx) override;

    std::any visitSign(GrammarParser::SignContext *ctx) override;

    std::any visitBool_(GrammarParser::Bool_Context *ctx) override;

    std::any visitString(GrammarParser::StringContext *ctx) override;

    std::any visitTypeDefinitionPart(GrammarParser::TypeDefinitionPartContext *ctx) override;

    std::any visitTypeDefinition(GrammarParser::TypeDefinitionContext *ctx) override;

    std::any visitArrayType(GrammarParser::ArrayTypeContext *ctx) override;

    std::any visitRecordType(GrammarParser::RecordTypeContext *ctx) override;

    std::any visitTypeList(GrammarParser::TypeListContext *ctx) override;

    std::any visitType_(GrammarParser::Type_Context *ctx) override;

    std::any visitFixedPart(GrammarParser::FixedPartContext *ctx) override;

    std::any visitRecordSection(GrammarParser::RecordSectionContext *ctx) override;

    std::any visitIdentifierList(GrammarParser::IdentifierListContext *ctx) override;

    std::any visitSimpleType(GrammarParser::SimpleTypeContext *ctx) override;

    std::any visitPointerType(GrammarParser::PointerTypeContext *ctx) override;

    std::any visitTypeIdentifier(GrammarParser::TypeIdentifierContext *ctx) override;

    std::any visitStringType(GrammarParser::StringTypeContext *ctx) override;

    std::any visitScalarType(GrammarParser::ScalarTypeContext *ctx) override;

    std::any visitRangeList(GrammarParser::RangeListContext *ctx) override;

    std::any visitVariableDeclarationPart(GrammarParser::VariableDeclarationPartContext *ctx) override;

    std::any visitVariableDeclaration(GrammarParser::VariableDeclarationContext *ctx) override;

    std::any visitProcedureAndFunctionDeclarationPart(GrammarParser::ProcedureAndFunctionDeclarationPartContext *ctx) override;

    std::any visitProcedureOrFunctionDeclaration(GrammarParser::ProcedureOrFunctionDeclarationContext *ctx) override;

    std::any visitProcedureDeclaration(GrammarParser::ProcedureDeclarationContext *ctx) override;

    std::any visitFunctionDeclaration(GrammarParser::FunctionDeclarationContext *ctx) override;

    std::any visitFormalParameterList(GrammarParser::FormalParameterListContext *ctx) override;

    std::any visitParameterGroup(GrammarParser::ParameterGroupContext *ctx) override;

    std::any visitResultType(GrammarParser::ResultTypeContext *ctx) override;

    std::any visitCompoundStatement(GrammarParser::CompoundStatementContext *ctx) override;

    std::any visitStatements(GrammarParser::StatementsContext *ctx) override;

    std::any visitStatement(GrammarParser::StatementContext *ctx) override;

    std::any visitSimpleStatement(GrammarParser::SimpleStatementContext *ctx) override;

    std::any visitStructuredStatement(GrammarParser::StructuredStatementContext *ctx) override;

    std::any visitAssignmentStatement(GrammarParser::AssignmentStatementContext *ctx) override;

    std::any visitProcedureStatement(GrammarParser::ProcedureStatementContext *ctx) override;

    std::any visitVariable(GrammarParser::VariableContext *ctx) override;

    std::any visitAccess(GrammarParser::AccessContext *ctx) override;

    std::any visitArrayAccess(GrammarParser::ArrayAccessContext *ctx) override;

    std::any visitStructAccess(GrammarParser::StructAccessContext *ctx) override;

    std::any visitExpression(GrammarParser::ExpressionContext *ctx) override;

    std::any visitSimpleExpression(GrammarParser::SimpleExpressionContext *ctx) override;

    std::any visitRelationaloperator(GrammarParser::RelationaloperatorContext *ctx) override;

    std::any visitAdditiveoperator(GrammarParser::AdditiveoperatorContext *ctx) override;

    std::any visitTerm(GrammarParser::TermContext *ctx) override;

    std::any visitSignedFactor(GrammarParser::SignedFactorContext *ctx) override;

    std::any visitMultiplicativeoperator(GrammarParser::MultiplicativeoperatorContext *ctx) override;

    std::any visitFactor(GrammarParser::FactorContext *ctx) override;

    std::any visitFunctionDesignator(GrammarParser::FunctionDesignatorContext *ctx) override;

    std::any visitParameterList(GrammarParser::ParameterListContext *ctx) override;

    std::any visitUnsignedConstant(GrammarParser::UnsignedConstantContext *ctx) override;

    std::any visitConditionalStatement(GrammarParser::ConditionalStatementContext *ctx) override;

    std::any visitRepetetiveStatement(GrammarParser::RepetetiveStatementContext *ctx) override;

    std::any visitIfStatement(GrammarParser::IfStatementContext *ctx) override;

    std::any visitElseIfStatement(GrammarParser::ElseIfStatementContext *ctx) override;

    std::any visitElseStatement(GrammarParser::ElseStatementContext *ctx) override;

    std::any visitSwitchStatement(GrammarParser::SwitchStatementContext *ctx) override;

    std::any visitDefaultStatement(GrammarParser::DefaultStatementContext *ctx) override;

    std::any visitCaseListElement(GrammarParser::CaseListElementContext *ctx) override;

    std::any visitConstList(GrammarParser::ConstListContext *ctx) override;

    std::any visitWhileStatement(GrammarParser::WhileStatementContext *ctx) override;

    std::any visitRepeatStatement(GrammarParser::RepeatStatementContext *ctx) override;

    std::any visitForStatement(GrammarParser::ForStatementContext *ctx) override;

    std::any visitForList(GrammarParser::ForListContext *ctx) override;

    std::any visitInitialValue(GrammarParser::InitialValueContext *ctx) override;

    std::any visitFinalValue(GrammarParser::FinalValueContext *ctx) override;
};

#endif /* LIBITMOANTLR_TRANSLATOR_H */
