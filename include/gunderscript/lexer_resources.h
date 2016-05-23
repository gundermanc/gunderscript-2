// Gunderscript-2 Lexer API Resources
// (C) 2016 Christian Gunderman

#ifndef GUNDERSCRIPT_LEXER_RESOURCES__H__
#define GUNDERSCRIPT_LEXER_RESOURCES__H__

namespace gunderscript {

// POTENTIAL BUG BUG BUG: Whenever you update this enum be sure to update
// kLexerTokenTypeString array in lexer.cc file or things will break!
// LexerToken Types.
// ACCESS_MODIFIER: contains LexerSymbol.
// KEYWORD: contains LexerSymbol.
// SYMBOL: contains LexerSymbol
// NAME: contains string_const and represents a variable name or type name.
// INT: contains int_const.
// FLOAT: contains float_const.
// STRING: contains string_const.
enum class LexerTokenType {
    ACCESS_MODIFIER, KEYWORD, SYMBOL, NAME, INT, FLOAT, STRING, CHAR
};

// All of the various operators, types, access modifiers, etc.
enum class LexerSymbol {
    // Symbols:
    SWAP, ASSIGN, LESSEQUALS, LESS, GREATEREQUALS, GREATER, ADD,
    ADDEQUALS, SUB, SUBEQUALS, MUL, MULEQUALS, DIV, DIVEQUALS, MOD, MODEQUALS,
    LPAREN, RPAREN, LSQUARE, RSQUARE, LBRACE, RBRACE, DOT, SEMICOLON, COMMA,
    LOGOR, BINOR, LOGAND, BINAND, LOGNOT, BINNOT, EQUALS, NOTEQUALS, COLON,
    TERNARY,

    // Access Modifiers:
    PUBLIC, CONCEALED, INTERNAL,

    // Keywords:
    SPEC, IF, ELSE, DO, WHILE, KTRUE, KFALSE, RETURN, GET, SET, CONSTRUCT,
    START, READONLY, FOR, BREAK, CONTINUE, DEPENDS, PACKAGE,
    NEW, DEFAULT,

    // TODO: remove this when we make a legit type system.
    ANY_TYPE
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

// Maps a lexer token enum value to its string representation.
const std::string LexerTokenTypeString(LexerTokenType type);

// Maps a lexer symbol enum value to its string representation.
const std::string LexerSymbolString(LexerSymbol symbol);

} // namespace gunderscript

#endif // GUNDERSCRIPT_LEXER_RESOURCES__H__
