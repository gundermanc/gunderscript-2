// Gunderscript-2 Parser
// (C) 2014 Christian Gunderman

#ifndef GUNDERSCRIPT_PARSER__H__
#define GUNDERSCRIPT_PARSER__H__

#include "exceptions.h"
#include "lexer.h"
#include "node.h"

namespace gunderscript {
namespace library {

  /*class Parser {
#ifdef DEBUG
  friend class ParserTestHarness;
#endif // DEBUG

 public:
 Parser(Lexer& lexer) : //lexer_(lexer),
      root_(new Node<VOID_NODE>(NodeRule::MODULE) { }

 private:
  Lexer& lexer_;
  Node<VOID_NODE>& root_;

  const LexerToken* AdvanceNext();
  const LexerToken* CurrentToken();
  const LexerToken* NextToken();
  void ThrowEOFIfNull(const LexerToken* token);

  void ParsePackageDeclaration(Node<VOID_NODE>& node);
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

// Configuration constants:
const int PARSER_MIN_PACKAGE_LEN = 5;

// String error constants:
const std::string PARSER_ERR_EXPECTED_PACKAGE 
    = "Expected package declaration at beginning of file.";

const std::string PARSER_ERR_BAD_PACKAGE_NAME
    = "Invalid name in package declaration at head of file.";

// Parser test harness friend class. Grants access to private members to
// the unit tests.
#ifdef DEBUG
class ParserTestHarness {
 public:

  
};
#endif // DEBUG
  */
} // namespace library
} // namespace gunderscript

#endif // GUNDERSCRIPT_PARSER__H__
