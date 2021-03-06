// Gunderscript-2 Parser
// (C) 2014-2016 Christian Gunderman

#ifndef GUNDERSCRIPT_PARSER__H__
#define GUNDERSCRIPT_PARSER__H__

#include "gunderscript/exceptions.h"
#include "gunderscript/node.h"

#include "lexer.h"

// Debug assertion checks that we have the correct node rule.
// Not compiled in Release configuration.
#define GS_ASSERT_NODE_RULE(node, node_rule) \
    GS_ASSERT_TRUE(((node) != NULL) && ((node)->rule() == (node_rule)), "Null AstWalker node or invalid node rule");
#define GS_ASSERT_OPTIONAL_NODE_RULE(node, node_rule) \
    GS_ASSERT_TRUE(((node) == NULL) || ((node)->rule() == (node_rule)), "Invalid AstWalker node rule");

namespace gunderscript {
namespace compiler {

// Accepts a Lexer and converts a stream of lexemes into an abstract syntax tree
// that is ready to be analyzed.
class Parser {
public:
    Parser(Lexer& lexer) : lexer_(lexer) { }
    Node* Parse();

private:
    Lexer& lexer_;
    Node* module_node_;
    const LexerToken* AdvanceNext();
    const LexerToken* CurrentToken();
    const LexerToken* NextToken();
    bool AdvanceSymbol(LexerSymbol symbol);
    bool CurrentSymbol(LexerSymbol symbol);
    bool NextSymbol(LexerSymbol symbol);
    bool AdvanceType(LexerSymbol type);
    bool CurrentType(LexerSymbol type);
    bool AdvanceKeyword(LexerSymbol keyword);
    bool CurrentKeyword(LexerSymbol keyword);
    bool NextKeyword(LexerSymbol keyword);
    bool AdvanceAccessModifier(LexerSymbol am);
    bool CurrentAccessModifier(LexerSymbol am);
    void ThrowEOFIfNull(const LexerToken* token);
    bool has_next() { return lexer_.has_next(); }

    Node* ParseModule();
    void ParsePackageDeclaration(Node* node);
    void ParseDependsStatements(Node* node);
    void ParseDependsStatement(Node* node);
    void ParseSemicolon();
    void ParseModuleBody(Node* module_node);
    void ParseSpecDefinition(Node* specs_node);
    void ParseSpecBody(Node* spec_node);
    void ParseProperty(Node* node);
    void ParsePropertyBody(Node* property_body);
    void ParsePropertyBodyFunction(Node* getter_node, Node* setter_node);
    void ParseFunction(Node* node, bool in_spec);
    void ParseFunctionParameters(Node* function_node);
    void ParseFunctionParameter(Node* node);
    void ParseBlockStatement(Node* node);
    void ParseStatement(Node* node);
    void ParseKeywordStatement(Node* node);
    void ParseIfStatement(Node* node);
    void ParseElIfStatement(Node* node);
    void ParseWhileStatement(Node* node);
    void ParseForStatement(Node* node);
    void ParseReturnStatement(Node* node);
    void ParseNameStatement(Node* node);
    void ParseCallStatement(Node* node);
    void ParseAssignStatement(Node* node);
    void ParseExpression(Node* node);
    Node* ParseAssignExpressionA();
    Node* ParseAssignExpressionB(Node* left_operand_node);
    Node* ParseOrExpression(Node* left_operand_node);
    Node* ParseAndExpressionA();
    Node* ParseAndExpressionB(Node* left_operand_node);
    Node* ParseComparisonExpressionA();
    Node* ParseComparisonExpressionB(Node* left_operand_node);
    Node* ParsePrimaryExpressionA();
    Node* ParsePrimaryExpressionB(Node* left_operand_node);
    Node* ParseSecondaryExpressionA();
    Node* ParseSecondaryExpressionB(Node* left_operand_node);
    Node* ParseTertiaryExpressionA();
    Node* ParseTertiaryExpressionB(Node* left_operand_node);
    Node* ParseInvertExpression();
    Node* ParseAtomicExpression();
    Node* ParseValueExpression();
    Node* ParseNamedValueExpression();
    Node* ParseCallExpression();
    Node* ParseNewExpression();
    Node* ParseDefaultExpression();
    void ParseTypeExpression(Node* parent_node);
    void ParseCallParameters(Node* node);
    Node* ParseVariableExpression();
    Node* ParseBoolConstant();
    Node* ParseIntConstant();
    Node* ParseFloatConstant();
    Node* ParseCharConstant();
    Node* ParseStringConstant();
};

// Constructor mangled function name.
const std::string kConstructorName = "%construct%";

const std::string kThisKeyword = "this";

} // namespace library
} // namespace gunderscript

#endif // GUNDERSCRIPT_PARSER__H__
