// Gunderscript-2 Parser
// (C) 2014 Christian Gunderman

#include "parser.h"

namespace gunderscript {
namespace library {

  /*
void Parser::ParsePackageDeclaration() {

  // Check "package" keyword.
  if (AdvanceNext()->symbol != LexerSymbol::PACKAGE) {
    throw ParserMalformedPackageException(*this, PARSER_ERR_EXPECTED_PACKAGE);
  }

  // Check package name string.
  if (AdvanceNext()->type != LexerTokenType::STRING ||
      CurrentToken()->string_const->length() < PARSER_MIN_PACKAGE_LEN) {
    throw ParserMalformedPackageException(*this, PARSER_ERR_BAD_PACKAGE_NAME);
  }

  // Save package name string in module.
  
  
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

void Parser::ThrowEOFIfNull(const LexerToken* token) {
  if (token == NULL) {
    throw ParserEndOfFileException(*this);
  }
}
  */

} // namespace library
} // namespace gunderscript
