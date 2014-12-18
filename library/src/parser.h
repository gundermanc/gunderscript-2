// Gunderscript-2 Parser
// (C) 2014 Christian Gunderman

#ifndef GUNDERSCRIPT_PARSER__H__
#define GUNDERSCRIPT_PARSER__H__

#include "exceptions.h"
#include "lexer.h"
#include "node.h"

#include "gtest/gtest_prod.h"

namespace gunderscript {
namespace library {

class Parser {
 public:
  Parser(Lexer& lexer) : lexer_(lexer) { }
  Node* Parse() { return ParseModule(); }
  Lexer* lexer() const { return &lexer_; }

 private:
  Lexer& lexer_;
  const LexerToken* AdvanceNext();
  const LexerToken* CurrentToken();
  const LexerToken* NextToken();
  bool AdvanceSymbol(LexerSymbol symbol);
  bool CurrentSymbol(LexerSymbol symbol);
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
    = "Expected terminated semicolon.";

const std::string PARSER_ERR_BAD_PACKAGE_NAME
    = "Expected name for package in declaration at head of file.";

const std::string PARSER_ERR_MALFORMED_DEPENDS
    = "Malformed depends statement.";

} // namespace library
} // namespace gunderscript

#endif // GUNDERSCRIPT_PARSER__H__
