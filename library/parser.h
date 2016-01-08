// Gunderscript-2 Parser
// (C) 2014-2016 Christian Gunderman

#ifndef GUNDERSCRIPT_PARSER__H__
#define GUNDERSCRIPT_PARSER__H__

#include "gunderscript/exceptions.h"

#include "gunderscript/node.h"

#include "lexer.h"

namespace gunderscript {
namespace library {

// Accepts a Lexer and converts a stream of lexemes into an abstract syntax tree
// that is ready to be analyzed.
class Parser {
public:
    Parser(Lexer& lexer) : lexer_(lexer) { }
    Node* Parse();
    Lexer* lexer() const { return &lexer_; }

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
    bool AdvanceAccessModifier(LexerSymbol am);
    bool CurrentAccessModifier(LexerSymbol am);
    void ThrowEOFIfNull(const LexerToken* token);
    bool has_next() { return lexer_.has_next(); }

    Node* ParseModule();
    void ParsePackageDeclaration(Node* node);
    void ParseDependsStatements(Node* node);
    void ParseDependsStatement(Node* node);
    void ParseSemicolon(Node* node);
    void ParseSpecDefinitions(Node* node);
    void ParseSpecDefinition(Node* node);
    void ParseSpecBody(Node* node);
    void ParseProperty(Node* node);
    void ParsePropertyBody(Node* node);
    void ParsePropertyBodyFunction(Node* getter_node, Node* setter_node);
    void ParseFunction(Node* node);
    void ParseFunctionParameters(Node* node);
    void ParseFunctionParameter(Node* node);
    void ParseBlockStatement(Node* node);
    void ParseStatement(Node* node);
    void ParseKeywordStatement(Node* node);
    void ParseIfStatement(Node* node);
    void ParseWhileStatement(Node* node);
    void ParseDoWhileStatement(Node* node);
    void ParseForStatement(Node* node);
    void ParseReturnStatement(Node* node);
    void ParseNameStatement(Node* node);
    void ParseCallStatement(Node* node);
    void ParseAssignStatement(Node* node);
    void ParseExpression(Node* node);
    Node* ParseAssignExpressionA();
    Node* ParseAssignExpressionB(Node* left_operand_node);
    Node* ParseOrExpressionA();
    Node* ParseOrExpressionB(Node* left_operand_node);
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
    Node* ParseMemberNameExpression();
    void ParseCallParameters(Node* node);
    Node* ParseVariableExpression();
    Node* ParseBoolConstant();
    Node* ParseIntConstant();
    Node* ParseFloatConstant();
    Node* ParseCharConstant();
    Node* ParseStringConstant();
};

// Parser Exceptions Parent Class
// All Parser exceptions descend from this class.
class ParserException : public Exception {
public:
    ParserException(const Parser& parser) : Exception(), parser_(parser) { }
    ParserException(const Parser& parser,
        const std::string& message) : Exception(message), parser_(parser) { }
    const Parser& parser() { return parser_; }

private:
    const Parser& parser_;
};

// Parser unexpected end of file exception.
class ParserEndOfFileException : public ParserException {
public:
    ParserEndOfFileException(const Parser& parser) :
        ParserException(parser,
            "Encountered end of file while parsing.") { }
};

// Parser malformed package declaration.
class ParserMalformedPackageException : public ParserException {
public:
    ParserMalformedPackageException(const Parser& parser,
        const std::string& message) :
        ParserException(parser, message) { }
};

// Parser malformed depends exception.
class ParserMalformedDependsException : public ParserException {
public:
    ParserMalformedDependsException(const Parser& parser,
        const std::string& message) :
        ParserException(parser, message) { }
};

// Parser malformed spec exception.
class ParserMalformedSpecException : public ParserException {
public:
    ParserMalformedSpecException(const Parser& parser,
        const std::string& message) :
        ParserException(parser, message) { }
};

// Parser malformed property exception.
class ParserMalformedPropertyException : public ParserException {
public:
    ParserMalformedPropertyException(const Parser& parser,
        const std::string& message) :
        ParserException(parser, message) { }
};

// Parser malformed function exception.
class ParserMalformedFunctionException : public ParserException {
public:
    ParserMalformedFunctionException(const Parser& parser,
        const std::string& message) :
        ParserException(parser, message) { }
};

// Parser malformed body exception.
class ParserMalformedBodyException : public ParserException {
public:
    ParserMalformedBodyException(const Parser& parser,
        const std::string& message) :
        ParserException(parser, message) { }
};

// Parser malformed expression exception.
class ParserMalformedExpressionException : public ParserException {
public:
    ParserMalformedExpressionException(const Parser& parser,
        const std::string& message) :
        ParserException(parser, message) { }
};


// Parser unexpected token exception.
class ParserUnexpectedTokenException : public ParserException {
public:
    ParserUnexpectedTokenException(const Parser& parser,
        const std::string& message) :
        ParserException(parser, message) { }
};

// String error constants:
const std::string PARSER_ERR_EXPECTED_PACKAGE 
    = "Expected package declaration at beginning of file.";

const std::string PARSER_ERR_EXPECTED_SEMICOLON
    = "Expected terminating semicolon.";

const std::string PARSER_ERR_EXPECTED_STATEMENT
    = "Expected statement. No known statement begins with given token.";

const std::string PARSER_ERR_BAD_PACKAGE_NAME
    = "Expected name for package in declaration at head of file.";

const std::string PARSER_ERR_MALFORMED_DEPENDS
    = "Malformed depends statement.";

const std::string PARSER_ERR_MALFORMED_SPEC
    = "Malformed spec definition.";

const std::string PARSER_ERR_MALFORMED_PROPERTY
    = "Malformed property definition.";

const std::string PARSER_ERR_MALFORMED_FUNCTION
    = "Malformed function definition.";

const std::string PARSER_ERR_MALFORMED_BODY
    = "Malformed function or property body definition.";

const std::string PARSER_ERR_MALFORMED_EXPRESSION
    = "Malformed expression.";

} // namespace library
} // namespace gunderscript

#endif // GUNDERSCRIPT_PARSER__H__
