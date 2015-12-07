// Gunderscript-2 Lexer
// (C) 2014 Christian Gunderman

#include "lexer.h"

namespace gunderscript {
namespace library {

// Get next character from input and advance to next.
#define ADVANCE_CHAR()                                    \
    this->next_column_number_++;                          \
    this->source_->NextChar();

// Get next char without advancing to next.
#define PEEK_CHAR() (this->source_->PeekNextChar())

// Check if input source has input left/
#define HAS_CHAR() (this->source_->has_next())

// Save x as next token and return from function.
// x: LexerSymbol.
#define ACCEPT_SYMBOL_NU(x)                               \
    this->next_token_.type = LexerTokenType::SYMBOL;      \
    this->next_token_.symbol = x;                         \
    this->valid_next_token_ = true;                       \
    return;

// Save x as next token and advance input to next character
// and return from function.
// x: LexerTokeSymbol.
#define ACCEPT_SYMBOL(x)                          \
    ADVANCE_CHAR();                               \
    ACCEPT_SYMBOL_NU(x);

// Accept with or without suffix:
// x: suffix character.
// y: LexerSymbol if suffix present.
// z: LexerSymbol if suffix NOT present.
#define ACCEPT_WWOSUFFIX_NU(x, y, z)                          \
    if (PEEK_CHAR() == x) {                                   \
        ACCEPT_SYMBOL(y);                                     \
    } else {                                                  \
        ACCEPT_SYMBOL_NU(z);                                  \
    }                                                         \

// Accept token as y if it has suffix x, or z if x not present.
// x: suffix character.
// y: LexerSymbol if suffix present.
// z: LexerSymbol if suffix NOT present.
#define ACCEPT_WWOSUFFIX(x, y, z)                 \
    ADVANCE_CHAR();                               \
    ACCEPT_WWOSUFFIX_NU(x, y, z)

// Defines a keyword in the keyword map so it will be found during
// lexing.
#define DEFINE_KEYWORD(k, t, s)                               \
    this->keywords_map_.insert(std::make_pair(k, std::make_pair(t, s)));

// POTENTIAL BUG BUG BUG: Whenever you update this array be sure to update
// LexerTokenType enum in header. These two MUST be identical or things will
// break.
static const std::string kLexerTokenTypeString[] = {
    "ACCESS_MODIFIER", "TYPE", "KEYWORD", "SYMBOL", "NAME", "INT", "FLOAT",
    "STRING", "CHAR"
};

// POTENTIAL BUG BUG BUG: Whenever you update this array be sure to update
// LexerSymbolString enum in header. These two must be identical or things will
// break.
static const std::string kLexerSymbolString[] = {
    // Symbols:
    "SWAP", "ASSIGN", "LSHIFT", "LESSEQUALS", "LESS", "GREATEREQUALS", "RSHIFT",
    "GREATER", "ADD", "ADDEQUALS", "SUB", "SUBEQUALS", "MUL", "MULEQUALS", "DIV",
    "DIVEQUALS", "MOD", "MODEQUALS",
    "LPAREN", "RPAREN", "LSQUARE", "RSQUARE", "LBRACE", "RBRACE", "DOT", "SEMICOLON", "COMMA",
    "LOGOR", "BINOR", "LOGAND", "BINAND", "LOGNOT", "BINNOT", "EQUALS", "NOTEQUALS", "COLON",
    "TERNARY",

    // Access Modifiers:
    "PUBLIC", "CONCEALED", "INTERNAL",

    // Keywords:
    "SPEC", "IF", "ELSE", "DO", "WHILE", "TRUE", "FALSE", "RETURN", "GET", "SET", "CONCEIVE",
    "ERADICATE", "START", "READONLY", "FOR", "BREAK", "CONTINUE", "DEPENDS", "PACKAGE",
    "NATIVE",

    // Types:
    "CHAR", "INT", "FLOAT", "BOOL", "STRING", "NULL"
};

// Gets a string representation of each token.
// NOTE: for this method to operate correctly both the string array
// of tokens and the enum must match 1:1 and in the same order.
const std::string LexerTokenTypeString(LexerTokenType type) {
    return kLexerTokenTypeString[(int)type];
}

// Gets a string representation of each lexer symbol.
// NOTE: for this method to operate correctly both the string array
// of symbols and the enum must match 1:1 and in the same order.
const std::string LexerSymbolString(LexerSymbol symbol) {
    return kLexerSymbolString[(int)symbol];
}

// Gets the current character from an instance of LexerStringSource
// and advances the iterator to the next char.
// Returns: The next char, or -1 if no characters remain.
int LexerStringSource::NextChar() {
    if (!this->has_next()) {
        return -1;
    }

    return this->input_[this->index++];
}

// Looks at the current character in the series without moving the
// iterator.
// Returns: The current character, or -1 if no characters remain.
int LexerStringSource::PeekNextChar() {
    if (!this->has_next()) {
        return -1;
    }

    return this->input_[this->index];
}

// Constructs a Lexer instance from a LexerSource.
// input: the text data to lex.
// Throws: A LexerException or its subclasses.
Lexer::Lexer(LexerSourceInterface& source) {
    this->source_ = &source;
    this->first_load_ = true;
    this->current_column_number_ = 0;
    this->current_line_number_ = 0;
    this->next_column_number_ = 0;
    this->next_line_number_ = 1;

    // Load the keywords for the keywords table.
    this->LoadKeywords();

    // Parse up to the first token
    this->AdvanceTokens();
}

// Lexer Class destructor. Frees dynamically allocated strings.
Lexer::~Lexer() {
    if (this->valid_current_token_ &&
        (this->current_token_.type == LexerTokenType::STRING ||
            this->current_token_.type == LexerTokenType::NAME)) {
        delete this->current_token_.string_const;
    }

    if (this->valid_next_token_ &&
        (this->next_token_.type == LexerTokenType::STRING ||
            this->next_token_.type == LexerTokenType::NAME)) {
        delete this->next_token_.string_const;
    }
}

// Advances the current and next token and gets the new
// current token.
// Returns: The new current token (previously next_token),
// or NULL if no more tokens remain.
const LexerToken* Lexer::AdvanceNext() {
    this->AdvanceTokens();
    return this->current_token();
}

// Gets the current token. This is a fast constant time operation
// that requires no computation or IO.
// Returns: The current token, or NULL if no more tokens remain.
const LexerToken* Lexer::current_token() const {
    if (!this->valid_current_token_) {
        return NULL;
    }

    return &this->current_token_;
}

// Gets the next token (the that comes after the current one).
// This is a fast constant time operation.
// Returns: The next token, or NULL if there is no token after
// this one.
const LexerToken* Lexer::next_token() const {
    if (!this->valid_next_token_) {
        return NULL;
    }

    return &this->next_token_;
}

// Loads keywords into the keywords_map_ in preparation for lexing.
// Called by constructor.
void Lexer::LoadKeywords() {
    // Access Modifiers.
    DEFINE_KEYWORD("public", LexerTokenType::ACCESS_MODIFIER, LexerSymbol::PUBLIC);
    DEFINE_KEYWORD("concealed", LexerTokenType::ACCESS_MODIFIER, LexerSymbol::CONCEALED);
    DEFINE_KEYWORD("package", LexerTokenType::KEYWORD, LexerSymbol::PACKAGE);
    DEFINE_KEYWORD("internal", LexerTokenType::ACCESS_MODIFIER, LexerSymbol::INTERNAL);

    // Keywords.
    DEFINE_KEYWORD("spec", LexerTokenType::KEYWORD, LexerSymbol::SPEC);
    DEFINE_KEYWORD("if", LexerTokenType::KEYWORD, LexerSymbol::IF);
    DEFINE_KEYWORD("else", LexerTokenType::KEYWORD, LexerSymbol::ELSE);
    DEFINE_KEYWORD("do", LexerTokenType::KEYWORD, LexerSymbol::DO);
    DEFINE_KEYWORD("while", LexerTokenType::KEYWORD, LexerSymbol::WHILE);
    DEFINE_KEYWORD("true", LexerTokenType::KEYWORD, LexerSymbol::TRUE);
    DEFINE_KEYWORD("false", LexerTokenType::KEYWORD, LexerSymbol::FALSE);
    DEFINE_KEYWORD("return", LexerTokenType::KEYWORD, LexerSymbol::RETURN);
    DEFINE_KEYWORD("get", LexerTokenType::KEYWORD, LexerSymbol::GET);
    DEFINE_KEYWORD("set", LexerTokenType::KEYWORD, LexerSymbol::SET);
    DEFINE_KEYWORD("conceive", LexerTokenType::KEYWORD, LexerSymbol::CONCEIVE);
    DEFINE_KEYWORD("eradicate", LexerTokenType::KEYWORD, LexerSymbol::ERADICATE);
    DEFINE_KEYWORD("start", LexerTokenType::KEYWORD, LexerSymbol::START);
    DEFINE_KEYWORD("readonly", LexerTokenType::KEYWORD, LexerSymbol::READONLY);
    DEFINE_KEYWORD("for", LexerTokenType::KEYWORD, LexerSymbol::FOR);
    DEFINE_KEYWORD("break", LexerTokenType::KEYWORD, LexerSymbol::BREAK);
    DEFINE_KEYWORD("continue", LexerTokenType::KEYWORD, LexerSymbol::CONTINUE);
    DEFINE_KEYWORD("depends", LexerTokenType::KEYWORD, LexerSymbol::DEPENDS);
    DEFINE_KEYWORD("native", LexerTokenType::KEYWORD, LexerSymbol::NATIVE);
    DEFINE_KEYWORD("null", LexerTokenType::KEYWORD, LexerSymbol::TNULL);

    // Primitive Types:
    DEFINE_KEYWORD("int", LexerTokenType::TYPE, LexerSymbol::INT);
    DEFINE_KEYWORD("float", LexerTokenType::TYPE, LexerSymbol::FLOAT);
    DEFINE_KEYWORD("bool", LexerTokenType::TYPE, LexerSymbol::BOOL);
    DEFINE_KEYWORD("char", LexerTokenType::TYPE, LexerSymbol::CHAR);
    DEFINE_KEYWORD("string", LexerTokenType::TYPE, LexerSymbol::STRING);
}

// Processes and removes comments from code.
// Returns: True if there were comments to skip, and false if not.
bool Lexer::SkipComments() {

    if (this->SkipSingleLineComments() ||
        this->SkipMultiLineComments()) {
        return true;
    }

    return false;
}

// Processes and removes C++ style single line comments.
// Returns: True if there were comments to skip, and false if not.
bool Lexer::SkipSingleLineComments() {
    if (PEEK_CHAR() != '/') {
        return false;
    }

    while (true) {
        ADVANCE_CHAR();

        if (PEEK_CHAR() == '\n') {
            ADVANCE_CHAR();
            break;
        }

        if (!HAS_CHAR()) {
            this->valid_next_token_ = false;
            break;
        }
    }
    return true;
}

// Processes and removes C style multiline comments.
// Returns: True if this is a multiline comment.
// Throws: LexerException if there is an unterminated comment.
bool Lexer::SkipMultiLineComments() {

    if (PEEK_CHAR() != '*') {
        return false;
    }

    ADVANCE_CHAR();

    while (true) {

        if (PEEK_CHAR() == '*') {
            ADVANCE_CHAR();

            if (PEEK_CHAR() == '/') {
                ADVANCE_CHAR();
                break;
            }
        }
        else {
            ADVANCE_CHAR();
        }

        if (PEEK_CHAR() == '\n') {
            this->next_column_number_ = -1;
            this->next_line_number_++;
        }

        if (!HAS_CHAR()) {
            throw LexerCommentException(*this);
        }
    }

    if (!HAS_CHAR()) {
        this->valid_next_token_ = false;
    }
    return true;
}

// Extracts whitespace from code.
void Lexer::SkipWhitespace() {

    // skip contiguous blocks of whitespace
    while (PEEK_CHAR() == ' ' ||
        PEEK_CHAR() == '\t' ||
        PEEK_CHAR() == '\r' ||
        PEEK_CHAR() == '\n') {
        // Return carriage encountered: increment line numbers and c
        if (PEEK_CHAR() == '\n') {
            this->next_column_number_ = -1;
            this->next_line_number_++;
        }
        ADVANCE_CHAR();
    }

    if (!HAS_CHAR()) {
        this->valid_next_token_ = false;
    }
}

// Performs string escaping on popular escape sequences.
// buffer: A stringstream that will receive the escaped char.
// Throws: A LexerException if there is an unknown escape sequence.
void Lexer::EscapeChar(std::stringstream& buffer) {
    ADVANCE_CHAR();

    switch (PEEK_CHAR()) {
    case '\'':
    case '"':
    case '?':
    case '\\':
        buffer << (char)PEEK_CHAR();
        break;

    case 'b':
        buffer << '\b';
        break;
    case 'n':
        buffer << '\n';
        break;
    case 't':
        buffer << '\t';
        break;
    case 'r':
        buffer << '\r';
        break;
    case 'v':
        buffer << '\v';
        break;
    case 'f':
        buffer << '\f';
        break;
    default:
        throw LexerEscapeException(*this);
    }
}

void Lexer::ParseString() {
    std::stringstream buffer;

    while (HAS_CHAR()) {
        ADVANCE_CHAR();

        switch (PEEK_CHAR()) {
        case '\\':
            this->EscapeChar(buffer);
            break;

        case '\n':
            throw LexerStringException(*this);

        case '"':
            this->valid_next_token_ = true;
            this->next_token_.type = LexerTokenType::STRING;
            this->next_token_.string_const = new std::string(buffer.str());
            ADVANCE_CHAR();
            return;

        default:
            buffer << (char)PEEK_CHAR();
        }
    }

    throw LexerStringException(*this);
}

// Parses a name or Keyword from the InputStream and sets it as the
// next token.
void Lexer::ParseName() {
    std::stringstream buffer;

    while (isalpha(PEEK_CHAR()) ||
        isdigit(PEEK_CHAR()) ||
        PEEK_CHAR() == '_') {
        buffer << (char)PEEK_CHAR();
        ADVANCE_CHAR();
    }

    std::string name = buffer.str();
    try {
        // Check to see if parsed name is a keyword.
        std::pair<LexerTokenType, LexerSymbol> value = this->keywords_map_.at(name);

        this->next_token_.type = value.first;
        this->next_token_.symbol = value.second;
    }
    catch (const std::out_of_range&) {
        // Not a keyword, treat as name instead.
        this->next_token_.type = LexerTokenType::NAME;
        this->next_token_.string_const = new std::string(name);
    }

    this->valid_next_token_ = true;
}

// Parses a number from the input source into an integer or double
// constant and stores it in the next_token_.
void Lexer::ParseNumber() {
    std::stringstream buffer;
    bool is_float = false;

    while (isdigit(PEEK_CHAR()) || PEEK_CHAR() == '.') {
        if (PEEK_CHAR() == '.') {
            is_float = true;
        }

        buffer << (char)PEEK_CHAR();
        ADVANCE_CHAR();
    }

    std::string num_str = buffer.str();

    try {
        size_t consumed_idx = 0;
        if (is_float) {
            this->next_token_.type = LexerTokenType::FLOAT;
            this->next_token_.float_const = stod(num_str, &consumed_idx);
        }
        else {
            this->next_token_.type = LexerTokenType::INT;
            this->next_token_.int_const = stoi(num_str, &consumed_idx);
        }

        if (consumed_idx < num_str.length()) {
            throw LexerNumberException(*this);
        }
    }
    catch (const std::invalid_argument&) {
        throw LexerNumberException(*this);
    }
    catch (const std::out_of_range&) {
        throw LexerNumberException(*this);
    }

    this->valid_next_token_ = true;
}

// Cleans up tokens and memory from the previous iteration of AdvanceTokens.
void Lexer::CleanupLast() {

    // Free any old strings before losing the old current_token_ for good.
    if (this->valid_current_token_ &&
        (this->current_token_.type == LexerTokenType::STRING ||
            this->current_token_.type == LexerTokenType::NAME)) {
        delete this->current_token_.string_const;
    }

    // make old "next" token the "current" token
    this->valid_current_token_ = this->valid_next_token_;
    this->current_token_ = this->next_token_;
    this->current_column_number_ = this->next_column_number_;
    this->current_line_number_ = this->next_line_number_;
}

// Parse Character Constants from the input.
void Lexer::ParseCharacter() {
    ADVANCE_CHAR();
    char c = PEEK_CHAR();

    ADVANCE_CHAR();
    if (PEEK_CHAR() != '\'') {
        throw LexerCharacterException(*this);
    }

    ADVANCE_CHAR();

    this->valid_next_token_ = true;
    this->next_token_.type = LexerTokenType::CHAR;
    this->next_token_.char_const = c;
}

// Moves the former next_token_ to the current_token_ and parses new
// input until it finds the next cohesive token to place in next_token_.
// Throws: LexerError if any of the parse submethods has an error, or if
// no subparsers match the pattern of the input.
void Lexer::AdvanceTokens() {
    this->CleanupLast();

    // No more input, terminate.
    if (!HAS_CHAR()) {
        this->valid_next_token_ = false;
        return;
    }

    while (HAS_CHAR()) {
        // Lex input to find next token.
        switch (PEEK_CHAR()) {

        case '\n':
        case ' ':
        case '\t':
        case '\r':
            this->SkipWhitespace();
            break;
        case '"':
            this->ParseString();
            return;
        case '\'':
            this->ParseCharacter();
            return;
        case '<': // <-, <, <=, <<, <->
            ADVANCE_CHAR();
            switch (PEEK_CHAR()) {
            case '-':
                ADVANCE_CHAR();
                if (PEEK_CHAR() == '>') {
                    ACCEPT_SYMBOL(LexerSymbol::SWAP);
                }
                else {
                    ACCEPT_SYMBOL_NU(LexerSymbol::ASSIGN);
                }
            case '<':
                ACCEPT_SYMBOL(LexerSymbol::LSHIFT);
            case '=':
                ACCEPT_SYMBOL(LexerSymbol::LESSEQUALS);
            default:
                ACCEPT_SYMBOL_NU(LexerSymbol::LESS);
            }

        case '>': // >, >=, >>
            ADVANCE_CHAR();
            switch (PEEK_CHAR()) {
            case '=':
                ACCEPT_SYMBOL(LexerSymbol::GREATEREQUALS);
            case '>':
                ACCEPT_SYMBOL(LexerSymbol::RSHIFT);
            default:
                ACCEPT_SYMBOL_NU(LexerSymbol::GREATER);
            }

        case '+': // +, +=
            ACCEPT_WWOSUFFIX('=', LexerSymbol::ADDEQUALS, LexerSymbol::ADD);
        case '-': // -, -=
            ACCEPT_WWOSUFFIX('=', LexerSymbol::SUBEQUALS, LexerSymbol::SUB);
        case '*': // *, *=
            ACCEPT_WWOSUFFIX('=', LexerSymbol::MULEQUALS, LexerSymbol::MUL);
        case '/': // /, /=, COMMENTS( //, /*, */)
            ADVANCE_CHAR();

            if (!this->SkipComments()) {
                ACCEPT_WWOSUFFIX_NU('=', LexerSymbol::DIVEQUALS, LexerSymbol::DIV);
            }
            break;

        case '%': // %, %=
            ACCEPT_WWOSUFFIX('=', LexerSymbol::MODEQUALS, LexerSymbol::MOD);
        case '(': // (
            ACCEPT_SYMBOL(LexerSymbol::LPAREN);
        case ')': // )
            ACCEPT_SYMBOL(LexerSymbol::RPAREN);
        case '[': // [
            ACCEPT_SYMBOL(LexerSymbol::LSQUARE);
        case ']': // ]]
            ACCEPT_SYMBOL(LexerSymbol::RSQUARE);
        case '{': // {
            ACCEPT_SYMBOL(LexerSymbol::LBRACE);
        case '}': // }
            ACCEPT_SYMBOL(LexerSymbol::RBRACE);
        case '.': // .
            ACCEPT_SYMBOL(LexerSymbol::DOT);
        case ';': // ;
            ACCEPT_SYMBOL(LexerSymbol::SEMICOLON);
        case ',': // ,
            ACCEPT_SYMBOL(LexerSymbol::COMMA);
        case '|': // |, ||
            ACCEPT_WWOSUFFIX('|', LexerSymbol::LOGOR, LexerSymbol::BINOR);
        case '&': // &, &&
            ACCEPT_WWOSUFFIX('&', LexerSymbol::LOGAND, LexerSymbol::BINAND);
        case '=': // =
            ACCEPT_SYMBOL(LexerSymbol::EQUALS);
        case '!': // !, !=
            ACCEPT_WWOSUFFIX('=', LexerSymbol::NOTEQUALS, LexerSymbol::LOGNOT);
        case '~':
            ACCEPT_SYMBOL(LexerSymbol::BINNOT);
        case ':': // :
            ACCEPT_SYMBOL(LexerSymbol::COLON);
        case '?': // ?
            ACCEPT_SYMBOL(LexerSymbol::TERNARY);
        default:
            if (isdigit(PEEK_CHAR())) {
                this->ParseNumber();
            }
            else if (isalpha(PEEK_CHAR()) || PEEK_CHAR() == '_') {
                this->ParseName();
            }
            else {
                // No matching lexing rules.
                throw LexerNoMatchException(*this);
            }
            return;
        }
    }
}

} // namespace library
} // namespace gunderscript
