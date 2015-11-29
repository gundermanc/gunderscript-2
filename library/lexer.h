// Gunderscript-2 Lexer
// (C) 2014 Christian Gunderman

#ifndef GUNDERSCRIPT_LEXER__H__
#define GUNDERSCRIPT_LEXER__H__

#include <sstream>
#include <unordered_map>

#include "exceptions.h"

namespace gunderscript {
namespace library {

// Provides source of input to lexer.
// has_next: returns false if no more input.
// NextChar: returns -1 if no more input.
// PeekNextChar: returns -1 if no more inpu.
class LexerSourceInterface {
 public:
  virtual ~LexerSourceInterface() { };
  virtual bool has_next() = 0;
  virtual int NextChar() = 0;
  virtual int PeekNextChar() = 0;
};

// Implements LexerSourceInterface and provides front
// end for strings.
class LexerStringSource : public LexerSourceInterface {
 public:
  LexerStringSource(std::string& input) { this->input_ = input; }
  bool has_next() { return this->index < this->input_.length(); }
  int NextChar();
  int PeekNextChar();
 private:
  std::string input_;
  size_t index = 0;
};

// LexerToken Types.
// ACCESS_MODIFIER: contains LexerSymbol.
// TYPE: contains LexerSymbol.
// KEYWORD: contains LexerSymbol.
// SYMBOL: contains LexerSymbol
// NAME: contains string_const.
// INT: contains int_const.
// FLOAT: contains float_const.
// STRING: contains string_const.
enum class LexerTokenType {
  ACCESS_MODIFIER, TYPE, KEYWORD, SYMBOL, NAME, INT, FLOAT, STRING, CHAR
};

// All of the various operators, types, access modifiers, etc.
enum class LexerSymbol {
  // Symbols:
  SWAP, ASSIGN, LSHIFT, LESSEQUALS, LESS, GREATEREQUALS, RSHIFT, GREATER, ADD,
    ADDEQUALS, SUB, SUBEQUALS, MUL, MULEQUALS, DIV, DIVEQUALS, MOD, MODEQUALS,
    LPAREN, RPAREN, LSQUARE, RSQUARE, LBRACE, RBRACE, DOT, SEMICOLON, COMMA, 
    LOGOR, BINOR, LOGAND, BINAND, LOGNOT, BINNOT, EQUALS, NOTEQUALS, COLON,
    TERNARY,

    // Access Modifiers:
    PUBLIC, CONCEALED, INTERNAL,

    // Keywords:
    SPEC, IF, ELSE, DO, WHILE, TRUE, FALSE, RETURN, GET, SET, CONCEIVE,
    ERADICATE, START, READONLY, FOR, BREAK, CONTINUE, DEPENDS, PACKAGE,
    NATIVE,

    // Types:
    CHAR, INT, FLOAT, BOOL, STRING
};

// LexerToken.
// type: Tells the content of the token and its semantics.
// symbol/string_const/int_const/float_const: the data.
typedef struct {
  LexerTokenType type;
  union {
    LexerSymbol symbol;
    const std::string* string_const;
    long int_const;
    double float_const;
    char char_const;
  };
} LexerToken;

// Lexer Class definition and Public/Private interfaces.
class Lexer {
 public:
  Lexer(LexerSourceInterface& source);
  ~Lexer();
  int current_column_number() const { return this->current_column_number_; }
  int current_line_number() const { return this->current_line_number_; }
  int next_column_number() const { return this->next_column_number_; }
  int next_line_number() const { return this->next_line_number_; }
  const LexerToken* AdvanceNext();
  const LexerToken* current_token() const;
  const LexerToken* next_token() const;
  bool has_next() const { return this->next_token() != NULL; }
  LexerSourceInterface* source() const { return source_; }

 private:
  LexerSourceInterface* source_;
  bool first_load_;
  int current_column_number_;
  int current_line_number_;
  int next_column_number_;
  int next_line_number_;
  bool valid_current_token_ = false;
  bool valid_next_token_ = false;
  LexerToken current_token_;
  LexerToken next_token_;
  std::unordered_map<std::string, std::pair<LexerTokenType, LexerSymbol> > keywords_map_;

  void LoadKeywords();
  bool SkipComments();
  bool SkipSingleLineComments();
  bool SkipMultiLineComments();
  void SkipWhitespace();
  void EscapeChar(std::stringstream& buffer);
  void ParseString();
  void ParseName();
  void ParseNumber();
  void ParseCharacter();
  void CleanupLast();
  void AdvanceTokens();
};

// Lexer Exceptions Parent Class
// All Lexer exceptions descend from this class.
class LexerException : public Exception {
 public:
  LexerException(const Lexer& lexer) : Exception(), lexer_(lexer) { }
  LexerException(const Lexer& lexer,
                 const std::string& message) : Exception(message), lexer_(lexer) { }
  const Lexer& lexer() { return lexer_; }

 private:
  const Lexer& lexer_;
};

// Exception for unterminated comments.
class LexerCommentException : public LexerException {
 public:
  LexerCommentException (const Lexer& lexer) : 
               LexerException(lexer, "Unterminated comment near line " +
                 std::to_string(lexer.current_line_number()) + 
                 ", column " + std::to_string(lexer.current_column_number()) + 
                 ".") { }
};

// Exception for unterminated String constants.
class LexerStringException : public LexerException {
 public:
  LexerStringException (const Lexer& lexer) : 
               LexerException(lexer,
                 "Unterminated string or new line in string near line " +
                 std::to_string(lexer.current_line_number()) + 
                 ", column " + std::to_string(lexer.current_column_number()) +
                 ".") { }
};

// Exception for invalid escape sequence.
class LexerEscapeException : public LexerException {
 public:
  LexerEscapeException (const Lexer& lexer) : 
               LexerException(lexer,
                 "Invalid string escape sequence near line " +
                 std::to_string(lexer.current_line_number()) + 
                 ", column " + std::to_string(lexer.current_column_number()) + 
                 ".") { }
};

// Exception for numeric constant.
class LexerNumberException : public LexerException {
 public:
  LexerNumberException (const Lexer& lexer) : 
               LexerException(lexer,
                 "Malformed number near line " +
                 std::to_string(lexer.current_line_number()) + 
                 ", column " + std::to_string(lexer.current_column_number()) + 
                 ".") { }
};

// Exception for char constant.
class LexerCharacterException : public LexerException {
 public:
  LexerCharacterException (const Lexer& lexer) : 
               LexerException(lexer,
                 "Malformed char constant near line " +
                 std::to_string(lexer.current_line_number()) + 
                 ", column " + std::to_string(lexer.current_column_number()) + 
                 ".") { }
};

// Exception for no matches.
class LexerNoMatchException : public LexerException {
 public:
  LexerNoMatchException (const Lexer& lexer) : 
               LexerException(lexer,
                 "Lexer did not match any near line " +
                 std::to_string(lexer.current_line_number()) + 
                 ", column " + std::to_string(lexer.current_column_number()) +
                 ".") { }
};

} // namespace library
} // namespace gunderscript

#endif // GUNDERSCRIPT_LEXER__H__
