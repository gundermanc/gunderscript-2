// Gunderscript-2 Lexer
// (C) 2014-2016 Christian Gunderman

#ifndef GUNDERSCRIPT_LEXER__H__
#define GUNDERSCRIPT_LEXER__H__

#include <sstream>
#include <unordered_map>

#include "gunderscript/compiler.h"
#include "gunderscript/exceptions.h"

namespace gunderscript {
namespace library {

// Lexer Class definition and Public/Private interfaces.
class Lexer {
public:
    Lexer(CompilerSourceInterface& source);
    ~Lexer();
    int current_column_number() const { return this->current_column_number_; }
    int current_line_number() const { return this->current_line_number_; }
    int next_column_number() const { return this->next_column_number_; }
    int next_line_number() const { return this->next_line_number_; }
    const LexerToken* AdvanceNext();
    const LexerToken* current_token() const;
    const LexerToken* next_token() const;
    bool has_next() const { return this->next_token() != NULL; }
    CompilerSourceInterface* source() const { return source_; }

private:
    CompilerSourceInterface* source_;
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
    LexerCommentException(const Lexer& lexer) :
        LexerException(lexer, "Unterminated comment near line " +
            std::to_string(lexer.current_line_number()) +
            ", column " + std::to_string(lexer.current_column_number()) +
            ".") { }
};

// Exception for unterminated String constants.
class LexerStringException : public LexerException {
public:
    LexerStringException(const Lexer& lexer) :
        LexerException(lexer,
            "Unterminated string or new line in string near line " +
            std::to_string(lexer.current_line_number()) +
            ", column " + std::to_string(lexer.current_column_number()) +
            ".") { }
};

// Exception for invalid escape sequence.
class LexerEscapeException : public LexerException {
public:
    LexerEscapeException(const Lexer& lexer) :
        LexerException(lexer,
            "Invalid string escape sequence near line " +
            std::to_string(lexer.current_line_number()) +
            ", column " + std::to_string(lexer.current_column_number()) +
            ".") { }
};

// Exception for numeric constant.
class LexerNumberException : public LexerException {
public:
    LexerNumberException(const Lexer& lexer) :
        LexerException(lexer,
            "Malformed number near line " +
            std::to_string(lexer.current_line_number()) +
            ", column " + std::to_string(lexer.current_column_number()) +
            ".") { }
};

// Exception for char constant.
class LexerCharacterException : public LexerException {
public:
    LexerCharacterException(const Lexer& lexer) :
        LexerException(lexer,
            "Malformed char constant near line " +
            std::to_string(lexer.current_line_number()) +
            ", column " + std::to_string(lexer.current_column_number()) +
            ".") { }
};

// Exception for no matches.
class LexerNoMatchException : public LexerException {
public:
    LexerNoMatchException(const Lexer& lexer) :
        LexerException(lexer,
            "Lexer did not match any near line " +
            std::to_string(lexer.current_line_number()) +
            ", column " + std::to_string(lexer.current_column_number()) +
            ".") { }
};

} // namespace library
} // namespace gunderscript

#endif // GUNDERSCRIPT_LEXER__H__
