// Gunderscript 2 Lexer Unit Test
// (C) 2014 Christian Gunderman

#include "gtest/gtest.h"

#include "lexer.h"

using gunderscript::library::Lexer;
using gunderscript::library::LexerToken;
using gunderscript::library::LexerTokenType;
using gunderscript::library::LexerSymbol;
using gunderscript::library::LexerStringSource;

// TEST_SYMBOL Macro:
// Defines the test template for symbol recognition.
// n: The test name.
// s: The symbol surrounded by quotes.
// l: The LexerSymbol that is expected.
#define TEST_SYMBOL(n, s, l)                        \
  TEST(Lexer, SymbolRecognition_ ## n ) {           \
    std::string input = s ;                         \
    LexerStringSource source(input);                \
    Lexer lexer(source);                            \
    const LexerToken* token = lexer.AdvanceNext();  \
    EXPECT_FALSE(token == NULL);                    \
    ASSERT_EQ(LexerTokenType::SYMBOL, token->type); \
    ASSERT_EQ(l, token->symbol);                    \
  }

// TEST_KEYWORD Macro:
// Defines the test template for keyword recognition.
// k: the keyword, as a string const.
// t: The LexerTokenType.
// s: The LexerSymbol it should be matched with.
#define TEST_KEYWORD(n, k, t, s)                   \
  TEST(Lexer, KeywordRecognition_ ## n ) {         \
    std::string input = k ;                        \
    LexerStringSource source(input);               \
    Lexer lexer(source);                           \
    const LexerToken* token = lexer.AdvanceNext(); \
    EXPECT_FALSE(token == NULL);                   \
    ASSERT_EQ(t, token->type);                     \
    ASSERT_EQ(token->symbol, s);                   \
  }

// Checks the LexerStringSource class to make sure that
// it parses strings correctly.
TEST(Lexer, LexerStringSource) {
  std::string input = "abc";
  LexerStringSource source(input);

  ASSERT_TRUE(source.has_next());
  ASSERT_EQ('a', source.PeekNextChar());
  ASSERT_EQ('a', source.NextChar());

  ASSERT_TRUE(source.has_next());
  ASSERT_EQ('b', source.PeekNextChar());
  ASSERT_EQ('b', source.NextChar());

  ASSERT_TRUE(source.has_next());
  ASSERT_EQ('c', source.PeekNextChar());
  ASSERT_EQ('c', source.NextChar());

  ASSERT_FALSE(source.has_next());
}

// Checks to make sure whitespace is recognized and removed.
TEST(Lexer, LexerWhitespace) {
  std::string input = " \n \t  = \n\t  ! \r";
  LexerStringSource source(input);
  Lexer lexer(source);

  const LexerToken* token = lexer.AdvanceNext();

  EXPECT_FALSE(token == NULL);
  ASSERT_EQ(token->type, LexerTokenType::SYMBOL);
  ASSERT_EQ(token->symbol, LexerSymbol::EQUALS);

  ASSERT_EQ(lexer.next_token()->symbol, LexerSymbol::LOGNOT);
}

// Tests for symbol recognition:
TEST_SYMBOL(ASSIGN, "<-", LexerSymbol::ASSIGN);
TEST_SYMBOL(LESS, "<", LexerSymbol::LESS);
TEST_SYMBOL(LESSEQUALS, "<=", LexerSymbol::LESSEQUALS);
TEST_SYMBOL(LSHIFT, "<<", LexerSymbol::LSHIFT);
TEST_SYMBOL(SWAP, "<->", LexerSymbol::SWAP);
TEST_SYMBOL(GREATER, ">", LexerSymbol::GREATER);
TEST_SYMBOL(GREATEREQUALS, ">=", LexerSymbol::GREATEREQUALS);
TEST_SYMBOL(RSHIFT, ">>", LexerSymbol::RSHIFT);
TEST_SYMBOL(ADD, "+", LexerSymbol::ADD);
TEST_SYMBOL(ADDEQUALS, "+=", LexerSymbol::ADDEQUALS);
TEST_SYMBOL(SUB, "-", LexerSymbol::SUB);
TEST_SYMBOL(SUBEQUALS, "-=", LexerSymbol::SUBEQUALS);
TEST_SYMBOL(MUL, "*", LexerSymbol::MUL);
TEST_SYMBOL(MULEQUALS, "*=", LexerSymbol::MULEQUALS);
TEST_SYMBOL(DIV, "/", LexerSymbol::DIV);
TEST_SYMBOL(DIVEQUALS, "/=", LexerSymbol::DIVEQUALS);
TEST_SYMBOL(MOD, "%", LexerSymbol::MOD);
TEST_SYMBOL(MODEQUALS, "%=", LexerSymbol::MODEQUALS);
TEST_SYMBOL(LPAREN, "(", LexerSymbol::LPAREN);
TEST_SYMBOL(RPAREN, ")", LexerSymbol::RPAREN);
TEST_SYMBOL(LSQUARE, "[", LexerSymbol::LSQUARE);
TEST_SYMBOL(RSQUARE, "]", LexerSymbol::RSQUARE);
TEST_SYMBOL(LBRACE, "{", LexerSymbol::LBRACE);
TEST_SYMBOL(RBRACE, "}", LexerSymbol::RBRACE);
TEST_SYMBOL(DOT, ".", LexerSymbol::DOT);
TEST_SYMBOL(SEMICOLON, ";", LexerSymbol::SEMICOLON);
TEST_SYMBOL(COMMA, ",", LexerSymbol::COMMA);
TEST_SYMBOL(LOGOR, "||", LexerSymbol::LOGOR);
TEST_SYMBOL(BINOR, "|", LexerSymbol::BINOR);
TEST_SYMBOL(LOGAND, "&&", LexerSymbol::LOGAND);
TEST_SYMBOL(BINAND, "&", LexerSymbol::BINAND);
TEST_SYMBOL(EQUALS, "=", LexerSymbol::EQUALS);
TEST_SYMBOL(NOTEQUALS, "!=", LexerSymbol::NOTEQUALS);
TEST_SYMBOL(LOGNOT, "!", LexerSymbol::LOGNOT);
TEST_SYMBOL(BINNOT, "~", LexerSymbol::BINNOT);
TEST_SYMBOL(COLON, ":", LexerSymbol::COLON);
TEST_SYMBOL(TERNARY, "?", LexerSymbol::TERNARY);

// Check symbols are still recognized sequentially,
// also sort of checks whitespace handling.
TEST(Lexer, SequentialSymbols) {
  std::string input = "  ! < > \n <- \t <->   <= << > >= >> /";
  LexerStringSource source(input);

  Lexer lexer(source);

  EXPECT_FALSE(lexer.AdvanceNext() == NULL);
  ASSERT_EQ(LexerTokenType::SYMBOL, lexer.current_token()->type);
  ASSERT_EQ(LexerSymbol::LOGNOT, lexer.current_token()->symbol);

  EXPECT_FALSE(lexer.AdvanceNext() == NULL);
  ASSERT_EQ(LexerTokenType::SYMBOL, lexer.current_token()->type);
  ASSERT_EQ(LexerSymbol::LESS, lexer.current_token()->symbol);

  EXPECT_FALSE(lexer.AdvanceNext() == NULL);
  ASSERT_EQ(LexerTokenType::SYMBOL, lexer.current_token()->type);
  ASSERT_EQ(LexerSymbol::GREATER, lexer.current_token()->symbol);

  EXPECT_FALSE(lexer.AdvanceNext() == NULL);
  ASSERT_EQ(LexerTokenType::SYMBOL, lexer.current_token()->type);
  ASSERT_EQ(LexerSymbol::ASSIGN, lexer.current_token()->symbol);

  EXPECT_FALSE(lexer.AdvanceNext() == NULL);
  ASSERT_EQ(LexerTokenType::SYMBOL, lexer.current_token()->type);
  ASSERT_EQ(LexerSymbol::SWAP, lexer.current_token()->symbol);

  EXPECT_FALSE(lexer.AdvanceNext() == NULL);
  ASSERT_EQ(LexerTokenType::SYMBOL, lexer.current_token()->type);
  ASSERT_EQ(LexerSymbol::LESSEQUALS, lexer.current_token()->symbol);

  EXPECT_FALSE(lexer.AdvanceNext() == NULL);
  ASSERT_EQ(LexerTokenType::SYMBOL, lexer.current_token()->type);
  ASSERT_EQ(LexerSymbol::LSHIFT, lexer.current_token()->symbol);

  EXPECT_FALSE(lexer.AdvanceNext() == NULL);
  ASSERT_EQ(LexerTokenType::SYMBOL, lexer.current_token()->type);
  ASSERT_EQ(LexerSymbol::GREATER, lexer.current_token()->symbol);

  EXPECT_FALSE(lexer.AdvanceNext() == NULL);
  ASSERT_EQ(LexerTokenType::SYMBOL, lexer.current_token()->type);
  ASSERT_EQ(LexerSymbol::GREATEREQUALS, lexer.current_token()->symbol);

  EXPECT_FALSE(lexer.AdvanceNext() == NULL);
  ASSERT_EQ(LexerTokenType::SYMBOL, lexer.current_token()->type);
  ASSERT_EQ(LexerSymbol::RSHIFT, lexer.current_token()->symbol);

  EXPECT_FALSE(lexer.AdvanceNext() == NULL);
  ASSERT_EQ(LexerTokenType::SYMBOL, lexer.current_token()->type);
  ASSERT_EQ(LexerSymbol::DIV, lexer.current_token()->symbol);
}

TEST(Lexer, SingleLineCommentCharOnly) {
  std::string input = "//";
  LexerStringSource source(input);

  Lexer lexer(source);

  EXPECT_TRUE(lexer.AdvanceNext() == NULL);
}

TEST(Lexer, TrailingSingleLineComment) {
  std::string input = " \t  ! < > // < > !";
  LexerStringSource source(input);
  Lexer lexer(source);

  EXPECT_FALSE(lexer.AdvanceNext() == NULL);
  ASSERT_EQ(LexerTokenType::SYMBOL, lexer.current_token()->type);
  ASSERT_EQ(LexerSymbol::LOGNOT, lexer.current_token()->symbol);

  EXPECT_FALSE(lexer.AdvanceNext() == NULL);
  ASSERT_EQ(LexerTokenType::SYMBOL, lexer.current_token()->type);
  ASSERT_EQ(LexerSymbol::LESS, lexer.current_token()->symbol);

  EXPECT_FALSE(lexer.AdvanceNext() == NULL);
  ASSERT_EQ(LexerTokenType::SYMBOL, lexer.current_token()->type);
  ASSERT_EQ(LexerSymbol::GREATER, lexer.current_token()->symbol);

  ASSERT_TRUE(lexer.AdvanceNext() == NULL);
}

TEST(Lexer, SingleLineCommentWithNewline) {
  std::string input = " // ! \n // <";
  LexerStringSource source(input);

  Lexer lexer(source);

  ASSERT_TRUE(lexer.AdvanceNext() == NULL);
}

TEST(Lexer, EmptyMultilineComment) {
  std::string input = "/**/";
  LexerStringSource source(input);

  Lexer lexer(source);

  ASSERT_TRUE(lexer.AdvanceNext() == NULL);
}

TEST(Lexer, MultilineCommentExtraStars) {
  std::string input = "/*****/";
  LexerStringSource source(input);

  Lexer lexer(source);

  ASSERT_TRUE(lexer.AdvanceNext() == NULL);
}

TEST(Lexer, MultilineCommentSingleStar) {
  std::string input = "/*/";
  LexerStringSource source(input);

  ASSERT_THROW(Lexer lexer(source),
               gunderscript::library::LexerCommentException);
}

TEST(Lexer, MultilineCommentUnterminated) {
  std::string input = "/*  < > ! <->";
  LexerStringSource source(input);

  ASSERT_THROW(Lexer lexer(source),
               gunderscript::library::LexerCommentException);
}

TEST(Lexer, MultilineCommentPrePostTokens) {
  std::string input = " <-> /* < > */ >> >";
  LexerStringSource source(input);
  Lexer lexer(source);

  EXPECT_FALSE(lexer.AdvanceNext() == NULL);
  ASSERT_EQ(LexerTokenType::SYMBOL, lexer.current_token()->type);
  ASSERT_EQ(LexerSymbol::SWAP, lexer.current_token()->symbol);

  EXPECT_FALSE(lexer.AdvanceNext() == NULL);
  ASSERT_EQ(LexerTokenType::SYMBOL, lexer.current_token()->type);
  ASSERT_EQ(LexerSymbol::RSHIFT, lexer.current_token()->symbol);

  EXPECT_FALSE(lexer.AdvanceNext() == NULL);
  ASSERT_EQ(LexerTokenType::SYMBOL, lexer.current_token()->type);
  ASSERT_EQ(LexerSymbol::GREATER, lexer.current_token()->symbol);

  ASSERT_TRUE(lexer.AdvanceNext() == NULL);
}


TEST(Lexer, MultilineCommentWithSecondStart) {
  std::string input = " <-> /* /* < > */ >> >";
  LexerStringSource source(input);
  Lexer lexer(source);

  EXPECT_FALSE(lexer.AdvanceNext() == NULL);
  ASSERT_EQ(LexerTokenType::SYMBOL, lexer.current_token()->type);
  ASSERT_EQ(LexerSymbol::SWAP, lexer.current_token()->symbol);

  EXPECT_FALSE(lexer.AdvanceNext() == NULL);
  ASSERT_EQ(LexerTokenType::SYMBOL, lexer.current_token()->type);
  ASSERT_EQ(LexerSymbol::RSHIFT, lexer.current_token()->symbol);

  EXPECT_FALSE(lexer.AdvanceNext() == NULL);
  ASSERT_EQ(LexerTokenType::SYMBOL, lexer.current_token()->type);
  ASSERT_EQ(LexerSymbol::GREATER, lexer.current_token()->symbol);

  ASSERT_TRUE(lexer.AdvanceNext() == NULL);
}

TEST(Lexer, MultilineCommentWithNewline) {
  std::string input = "\n <- /* /n/n * / */ +=";
  LexerStringSource source(input);
  Lexer lexer(source);

  EXPECT_FALSE(lexer.AdvanceNext() == NULL);
  ASSERT_EQ(LexerTokenType::SYMBOL, lexer.current_token()->type);
  ASSERT_EQ(LexerSymbol::ASSIGN, lexer.current_token()->symbol);

  EXPECT_FALSE(lexer.AdvanceNext() == NULL);
  ASSERT_EQ(LexerTokenType::SYMBOL, lexer.current_token()->type);
  ASSERT_EQ(LexerSymbol::ADDEQUALS, lexer.current_token()->symbol);

  ASSERT_TRUE(lexer.AdvanceNext() == NULL);
}

TEST(Lexer, LineNumbersWhitespace) {
  std::string input = "!    = \n  %=\n\n&";
  LexerStringSource source(input);
  Lexer lexer(source);

  // Initial conditions: no current token, only a next token.
  EXPECT_EQ(0, lexer.current_column_number());
  EXPECT_EQ(1, lexer.current_line_number());
  EXPECT_EQ(1, lexer.next_column_number());
  EXPECT_EQ(1, lexer.next_line_number());

  lexer.AdvanceNext();
  EXPECT_EQ(1, lexer.current_column_number());
  EXPECT_EQ(1, lexer.current_line_number());
  EXPECT_EQ(6, lexer.next_column_number());
  EXPECT_EQ(1, lexer.next_line_number());

  lexer.AdvanceNext();
  EXPECT_EQ(6, lexer.current_column_number());
  EXPECT_EQ(1, lexer.current_line_number());
  EXPECT_EQ(4, lexer.next_column_number());
  EXPECT_EQ(2, lexer.next_line_number());

  lexer.AdvanceNext();
  EXPECT_EQ(4, lexer.current_column_number());
  EXPECT_EQ(2, lexer.current_line_number());
  EXPECT_EQ(1, lexer.next_column_number());
  EXPECT_EQ(4, lexer.next_line_number());
}

TEST(Lexer, LineNumberComment) {
  std::string input = " % /* \n // & \n */   (\n&";
  LexerStringSource source(input);
  Lexer lexer(source);

  // Initial conditions: no current token, only a next token.
  EXPECT_EQ(0, lexer.current_column_number());
  EXPECT_EQ(1, lexer.current_line_number());
  EXPECT_EQ(2, lexer.next_column_number());
  EXPECT_EQ(1, lexer.next_line_number());

  lexer.AdvanceNext();
  EXPECT_EQ(2, lexer.current_column_number());
  EXPECT_EQ(1, lexer.current_line_number());
  EXPECT_EQ(7, lexer.next_column_number());
  EXPECT_EQ(3, lexer.next_line_number());

  lexer.AdvanceNext();
  EXPECT_EQ(7, lexer.current_column_number());
  EXPECT_EQ(3, lexer.current_line_number());
  EXPECT_EQ(1, lexer.next_column_number());
  EXPECT_EQ(4, lexer.next_line_number());

  lexer.AdvanceNext();
  EXPECT_EQ(1, lexer.current_column_number());
  EXPECT_EQ(4, lexer.current_line_number());

  EXPECT_TRUE(lexer.AdvanceNext() == NULL);
}

TEST(Lexer, LexRegularString) {
  std::string input = " \"Hello\" +";
  LexerStringSource source(input);
  Lexer lexer(source);

  lexer.AdvanceNext();
  ASSERT_FALSE(lexer.current_token() == NULL);
  EXPECT_EQ(LexerTokenType::STRING, lexer.current_token()->type);
  EXPECT_STREQ("Hello", lexer.current_token()->string_const->c_str());

  lexer.AdvanceNext();
  ASSERT_FALSE(lexer.current_token() == NULL);
  EXPECT_EQ(LexerTokenType::SYMBOL, lexer.current_token()->type);
  EXPECT_EQ(LexerSymbol::ADD, lexer.current_token()->symbol);
}

TEST(Lexer, LexUnterminatedString) {
  std::string input = " \"String is unterminated ";
  LexerStringSource source(input);

  ASSERT_THROW(Lexer lexer(source),
               gunderscript::library::LexerStringException);
}

TEST(Lexer, LexNewlineInString) {
  std::string input = " \" New line here -> \n <- OOPS\" ";
  LexerStringSource source(input);

  ASSERT_THROW(Lexer lexer(source),
               gunderscript::library::LexerStringException);
}

TEST(Lexer, LexEscapedString) {
  std::string input = "\"  \\'  \\\"  \\?  \\\\  \\b  \\n  \\t  \\r  \\v  \\f  \"";
  LexerStringSource source(input);
  Lexer lexer(source);

  lexer.AdvanceNext();
  ASSERT_FALSE(lexer.current_token() == NULL);
  EXPECT_EQ(LexerTokenType::STRING, lexer.current_token()->type);
  EXPECT_STREQ("  '  \"  ?  \\  \b  \n  \t  \r  \v  \f  ",
               lexer.current_token()->string_const->c_str());

  ASSERT_TRUE(lexer.AdvanceNext() == NULL);
}

TEST(Lexer, LexInvalidEscapedString) {
  std::string input = " \"   \\q    \" ";
  LexerStringSource source(input);

  ASSERT_THROW(Lexer lexer(source),
               gunderscript::library::LexerEscapeException);
}

TEST(Lexer, ParseNameKeyword) {
  std::string input = "hello    for  package";
  LexerStringSource source(input);
  Lexer lexer(source);

  ASSERT_FALSE(lexer.AdvanceNext() == NULL);
  EXPECT_EQ(LexerTokenType::NAME, lexer.current_token()->type);
  EXPECT_STREQ("hello", lexer.current_token()->string_const->c_str());

  ASSERT_FALSE(lexer.AdvanceNext() == NULL);
  EXPECT_EQ(LexerTokenType::KEYWORD, lexer.current_token()->type);
  EXPECT_EQ(LexerSymbol::FOR, lexer.current_token()->symbol);

  ASSERT_FALSE(lexer.AdvanceNext() == NULL);
  EXPECT_EQ(LexerTokenType::KEYWORD, lexer.current_token()->type);
  EXPECT_EQ(LexerSymbol::PACKAGE, lexer.current_token()->symbol);

  ASSERT_TRUE(lexer.AdvanceNext() == NULL);
}

// Tests for keyword recognition:
TEST_KEYWORD(PUBLIC, "public", LexerTokenType::ACCESS_MODIFIER, LexerSymbol::PUBLIC);
TEST_KEYWORD(CONCEALED, "concealed", LexerTokenType::ACCESS_MODIFIER, LexerSymbol::CONCEALED);
TEST_KEYWORD(PACKAGE, "package", LexerTokenType::KEYWORD, LexerSymbol::PACKAGE);
TEST_KEYWORD(INTERNAL, "internal", LexerTokenType::ACCESS_MODIFIER, LexerSymbol::INTERNAL);

TEST_KEYWORD(SPEC, "spec", LexerTokenType::KEYWORD, LexerSymbol::SPEC);
TEST_KEYWORD(IF, "if", LexerTokenType::KEYWORD, LexerSymbol::IF);
TEST_KEYWORD(ELSE, "else", LexerTokenType::KEYWORD, LexerSymbol::ELSE);
TEST_KEYWORD(DO, "do", LexerTokenType::KEYWORD, LexerSymbol::DO);
TEST_KEYWORD(WHILE, "while", LexerTokenType::KEYWORD, LexerSymbol::WHILE);
TEST_KEYWORD(TRUE, "true", LexerTokenType::KEYWORD, LexerSymbol::TRUE);
TEST_KEYWORD(FALSE, "false", LexerTokenType::KEYWORD, LexerSymbol::FALSE);
TEST_KEYWORD(RETURN, "return", LexerTokenType::KEYWORD, LexerSymbol::RETURN);
TEST_KEYWORD(GET, "get", LexerTokenType::KEYWORD, LexerSymbol::GET);
TEST_KEYWORD(SET, "set", LexerTokenType::KEYWORD, LexerSymbol::SET);
TEST_KEYWORD(CONCEIVE, "conceive", LexerTokenType::KEYWORD,LexerSymbol::CONCEIVE);
TEST_KEYWORD(ERADICATE, "eradicate", LexerTokenType::KEYWORD, LexerSymbol::ERADICATE);
TEST_KEYWORD(START, "start", LexerTokenType::KEYWORD, LexerSymbol::START);
TEST_KEYWORD(READONLY, "readonly", LexerTokenType::KEYWORD, LexerSymbol::READONLY);
TEST_KEYWORD(FOR, "for", LexerTokenType::KEYWORD, LexerSymbol::FOR);
TEST_KEYWORD(BREAK, "break", LexerTokenType::KEYWORD, LexerSymbol::BREAK);
TEST_KEYWORD(CONTINUE, "continue", LexerTokenType::KEYWORD, LexerSymbol::CONTINUE);
TEST_KEYWORD(DEPENDS, "depends", LexerTokenType::KEYWORD, LexerSymbol::DEPENDS);
TEST_KEYWORD(NATIVE, "native", LexerTokenType::KEYWORD, LexerSymbol::NATIVE);

TEST_KEYWORD(INT, "int", LexerTokenType::TYPE, LexerSymbol::INT);
TEST_KEYWORD(FLOAT, "float", LexerTokenType::TYPE, LexerSymbol::FLOAT);
TEST_KEYWORD(BOOL, "bool", LexerTokenType::TYPE, LexerSymbol::BOOL);
TEST_KEYWORD(CHAR, "char", LexerTokenType::TYPE, LexerSymbol::CHAR);

TEST(Lexer, ParseIntegers) {
  std::string input = "3433+ 211";
  LexerStringSource source(input);
  Lexer lexer(source);

  ASSERT_FALSE(lexer.AdvanceNext() == NULL);
  EXPECT_EQ(LexerTokenType::INT, lexer.current_token()->type);
  EXPECT_EQ(3433, lexer.current_token()->int_const);

  ASSERT_FALSE(lexer.AdvanceNext() == NULL);
  EXPECT_EQ(LexerTokenType::SYMBOL, lexer.current_token()->type);
  EXPECT_EQ(LexerSymbol::ADD, lexer.current_token()->symbol);

  ASSERT_FALSE(lexer.AdvanceNext() == NULL);
  EXPECT_EQ(LexerTokenType::INT, lexer.current_token()->type);
  EXPECT_EQ(211, lexer.current_token()->int_const);

  ASSERT_TRUE(lexer.AdvanceNext() == NULL);
}

TEST(Lexer, ParseIntegerOutOfRange) {
  std::string input = "9999999999999999999999999999999999999999999999999999999";
  LexerStringSource source(input);

  ASSERT_THROW(Lexer lexer(source),
               gunderscript::library::LexerNumberException);
}

TEST(Lexer, ParseFloats) {
  std::string input = "123.456 / 43.2";
  LexerStringSource source(input);
  Lexer lexer(source);

  ASSERT_FALSE(lexer.AdvanceNext() == NULL);
  EXPECT_EQ(LexerTokenType::FLOAT, lexer.current_token()->type);
  EXPECT_DOUBLE_EQ(123.456, lexer.current_token()->float_const);

  ASSERT_FALSE(lexer.AdvanceNext() == NULL);
  EXPECT_EQ(LexerTokenType::SYMBOL, lexer.current_token()->type);
  EXPECT_EQ(LexerSymbol::DIV, lexer.current_token()->symbol);

  ASSERT_FALSE(lexer.AdvanceNext() == NULL);
  EXPECT_EQ(LexerTokenType::FLOAT, lexer.current_token()->type);
  EXPECT_DOUBLE_EQ(43.2, lexer.current_token()->float_const);

  ASSERT_TRUE(lexer.AdvanceNext() == NULL);
}

TEST(Lexer, ParseFloatsWithMultipleDecimals) {
  std::string input = "34.43.3";
  LexerStringSource source(input);

  ASSERT_THROW(Lexer lexer(source),
               gunderscript::library::LexerNumberException);
}

TEST(Lexer, ParseChar) {
  std::string input = "  'c' '\"' ";
  LexerStringSource source(input);

  Lexer lexer(source);

  ASSERT_FALSE(lexer.AdvanceNext() == NULL);
  EXPECT_EQ(lexer.current_token()->type, LexerTokenType::CHAR);
  EXPECT_EQ(lexer.current_token()->char_const, 'c');

  ASSERT_FALSE(lexer.AdvanceNext() == NULL);
  EXPECT_EQ(lexer.current_token()->type, LexerTokenType::CHAR);
  EXPECT_EQ(lexer.current_token()->char_const, '"');

  ASSERT_TRUE(lexer.AdvanceNext() == NULL);
}

TEST(Lexer, ParseUnterminatedChar) {
  std::string input = " 'c  ";
  LexerStringSource source(input);

  ASSERT_THROW(Lexer lexer(source),
               gunderscript::library::LexerCharacterException);
}

TEST(Lexer, ParseMultiCharChar) {
  std::string input = " 'cd'  ";
  LexerStringSource source(input);

  ASSERT_THROW(Lexer lexer(source),
               gunderscript::library::LexerCharacterException);
}

TEST(Lexer, FunctionExample) {
  std::string input =
    "public int PrintFormat(string foobar) {"
    "  int x <- 3 * 2.56 + 5;"
    "}";

  LexerStringSource source(input);
  Lexer lexer(source);

  ASSERT_FALSE(lexer.AdvanceNext() == NULL);
  EXPECT_EQ(LexerTokenType::ACCESS_MODIFIER, lexer.current_token()->type);
  EXPECT_EQ(LexerSymbol::PUBLIC, lexer.current_token()->symbol);

  ASSERT_FALSE(lexer.AdvanceNext() == NULL);
  EXPECT_EQ(LexerTokenType::TYPE, lexer.current_token()->type);
  EXPECT_EQ(LexerSymbol::INT, lexer.current_token()->symbol);

  ASSERT_FALSE(lexer.AdvanceNext() == NULL);
  EXPECT_EQ(LexerTokenType::NAME, lexer.current_token()->type);
  EXPECT_STREQ("PrintFormat", lexer.current_token()->string_const->c_str());

  ASSERT_FALSE(lexer.AdvanceNext() == NULL);
  EXPECT_EQ(LexerTokenType::SYMBOL, lexer.current_token()->type);
  EXPECT_EQ(LexerSymbol::LPAREN, lexer.current_token()->symbol);

  ASSERT_FALSE(lexer.AdvanceNext() == NULL);
  EXPECT_EQ(LexerTokenType::TYPE, lexer.current_token()->type);
  EXPECT_EQ(LexerSymbol::STRING, lexer.current_token()->symbol);

  ASSERT_FALSE(lexer.AdvanceNext() == NULL);
  EXPECT_EQ(LexerTokenType::NAME, lexer.current_token()->type);
  EXPECT_STREQ("foobar", lexer.current_token()->string_const->c_str());

  ASSERT_FALSE(lexer.AdvanceNext() == NULL);
  EXPECT_EQ(LexerTokenType::SYMBOL, lexer.current_token()->type);
  EXPECT_EQ(LexerSymbol::RPAREN, lexer.current_token()->symbol);

  ASSERT_FALSE(lexer.AdvanceNext() == NULL);
  EXPECT_EQ(LexerTokenType::SYMBOL, lexer.current_token()->type);
  EXPECT_EQ(LexerSymbol::LBRACE, lexer.current_token()->symbol);

  ASSERT_FALSE(lexer.AdvanceNext() == NULL);
  EXPECT_EQ(LexerTokenType::TYPE, lexer.current_token()->type);
  EXPECT_EQ(LexerSymbol::INT, lexer.current_token()->symbol);

  ASSERT_FALSE(lexer.AdvanceNext() == NULL);
  EXPECT_EQ(LexerTokenType::NAME, lexer.current_token()->type);
  EXPECT_STREQ("x", lexer.current_token()->string_const->c_str());

  ASSERT_FALSE(lexer.AdvanceNext() == NULL);
  EXPECT_EQ(LexerTokenType::SYMBOL, lexer.current_token()->type);
  EXPECT_EQ(LexerSymbol::ASSIGN, lexer.current_token()->symbol);

  ASSERT_FALSE(lexer.AdvanceNext() == NULL);
  EXPECT_EQ(LexerTokenType::INT, lexer.current_token()->type);
  EXPECT_EQ(3, lexer.current_token()->int_const);

  ASSERT_FALSE(lexer.AdvanceNext() == NULL);
  EXPECT_EQ(LexerTokenType::SYMBOL, lexer.current_token()->type);
  EXPECT_EQ(LexerSymbol::MUL, lexer.current_token()->symbol);

  ASSERT_FALSE(lexer.AdvanceNext() == NULL);
  EXPECT_EQ(LexerTokenType::FLOAT, lexer.current_token()->type);
  EXPECT_EQ(2.56, lexer.current_token()->float_const);

  ASSERT_FALSE(lexer.AdvanceNext() == NULL);
  EXPECT_EQ(LexerTokenType::SYMBOL, lexer.current_token()->type);
  EXPECT_EQ(LexerSymbol::ADD, lexer.current_token()->symbol);

  ASSERT_FALSE(lexer.AdvanceNext() == NULL);
  EXPECT_EQ(LexerTokenType::INT, lexer.current_token()->type);
  EXPECT_EQ(5, lexer.current_token()->int_const);

  ASSERT_FALSE(lexer.AdvanceNext() == NULL);
  EXPECT_EQ(LexerTokenType::SYMBOL, lexer.current_token()->type);
  EXPECT_EQ(LexerSymbol::SEMICOLON, lexer.current_token()->symbol);

  ASSERT_FALSE(lexer.AdvanceNext() == NULL);
  EXPECT_EQ(LexerTokenType::SYMBOL, lexer.current_token()->type);
  EXPECT_EQ(LexerSymbol::RBRACE, lexer.current_token()->symbol);

  ASSERT_TRUE(lexer.AdvanceNext() == NULL);
}