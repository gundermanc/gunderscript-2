// Gunderscript-2 Parser
// (C) 2014 Christian Gunderman

#include "parser.h"

namespace gunderscript {
namespace library {

Node* Parser::ParseModule() {
  Node* module_node = new Node(NodeRule::MODULE);
  Node* depends_node = new Node(NodeRule::DEPENDS);

  ParsePackageDeclaration(module_node);

  module_node->AddChild(depends_node);

  // Allow end of file after package.
  if (has_next()) {
    AdvanceNext();
    ParseDependsStatements(depends_node);
  }

  return module_node;
}

void Parser::ParsePackageDeclaration(Node* node) {

  // Check "package" keyword.
  if (!AdvanceKeyword(LexerSymbol::PACKAGE)) {
    throw ParserMalformedPackageException(*this, PARSER_ERR_EXPECTED_PACKAGE);
  }

  // Check package name string.
  if (AdvanceNext()->type != LexerTokenType::STRING) {
    throw ParserMalformedPackageException(*this, PARSER_ERR_BAD_PACKAGE_NAME);
  }

  // Save package name string in first module child.
  node->AddChild(new Node(NodeRule::NAME, CurrentToken()->string_const));

  ParseSemicolon(node);
}

void Parser::ParseDependsStatements(Node* node) {

  // While in a series of "depends" statements, parse as "depends."
  while (CurrentKeyword(LexerSymbol::DEPENDS)) {
    ParseDependsStatement(node);

    // Allow empty file after depends statements.
    if (has_next()) {
      AdvanceNext();
    }
  }
}

void Parser::ParseDependsStatement(Node* node) {

  // Check "depends" statement.
  if (!CurrentKeyword(LexerSymbol::DEPENDS)) {
    throw ParserMalformedDependsException(*this, PARSER_ERR_MALFORMED_DEPENDS);
  }

  // Check for package name to import.
  if (AdvanceNext()->type != LexerTokenType::STRING) {
    throw ParserMalformedDependsException(*this, PARSER_ERR_MALFORMED_DEPENDS);
  }

  // Add package name to the depends tree node.
  node->AddChild(new Node(NodeRule::NAME, CurrentToken()->string_const));

  ParseSemicolon(node);
}

void Parser::ParseSemicolon(Node* node) {

  // Check for terminating semicolon.
  if (!AdvanceSymbol(LexerSymbol::SEMICOLON)) {
    throw ParserUnexpectedTokenException(*this, PARSER_ERR_EXPECTED_SEMICOLON);
  }
}

const LexerToken* Parser::AdvanceNext() {
  const LexerToken* token = this->lexer_.AdvanceNext();
  ThrowEOFIfNull(token);
  return token;
}

const LexerToken* Parser::CurrentToken() {
  const LexerToken* token = this->lexer_.current_token();
  ThrowEOFIfNull(token);
  return token;
}

const LexerToken* Parser::NextToken() {
  const LexerToken* token = this->lexer_.next_token();
  ThrowEOFIfNull(token);
  return token;
}

bool Parser::AdvanceSymbol(LexerSymbol symbol) {
  return AdvanceNext()->type == LexerTokenType::SYMBOL &&
      CurrentToken()->symbol == symbol;
}

bool Parser::CurrentSymbol(LexerSymbol symbol) {
  return CurrentToken()->type == LexerTokenType::SYMBOL &&
      CurrentToken()->symbol == symbol;
}

bool Parser::AdvanceType(LexerSymbol type) {
  return AdvanceNext()->type == LexerTokenType::TYPE &&
      CurrentToken()->symbol == type;
}

bool Parser::CurrentType(LexerSymbol type) {
  return CurrentToken()->type == LexerTokenType::TYPE &&
      CurrentToken()->symbol == type;
}

bool Parser::AdvanceKeyword(LexerSymbol keyword) {
  return AdvanceNext()->type == LexerTokenType::KEYWORD &&
      CurrentToken()->symbol == keyword;
}

bool Parser::CurrentKeyword(LexerSymbol keyword) {
  return CurrentToken()->type == LexerTokenType::KEYWORD &&
      CurrentToken()->symbol == keyword;
}

bool Parser::AdvanceAccessModifier(LexerSymbol am) {
  return AdvanceNext()->type == LexerTokenType::ACCESS_MODIFIER &&
      CurrentToken()->symbol == am;
}

bool Parser::CurrentAccessModifier(LexerSymbol am) {
  return CurrentToken()->type == LexerTokenType::ACCESS_MODIFIER &&
      CurrentToken()->symbol == am;
}

void Parser::ThrowEOFIfNull(const LexerToken* token) {
  if (token == NULL) {
    throw ParserEndOfFileException(*this);
  }
}

} // namespace library
} // namespace gunderscript
