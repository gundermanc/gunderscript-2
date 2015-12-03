// Gunderscript-2 Debug Functions
// (C) 2015 Christian Gunderman

#include <cstdio>
#include <cstring>

#include "debug.h"

namespace gunderscript {
namespace library {

static const char* kTokenTypeString[] = {
  "ACCESS_MODIFIER", "TYPE", "KEYWORD", "SYMBOL", "NAME", "INT", "FLOAT",
  "STRING", "CHAR"
};

static const char* kLexerSymbolString[] = {
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
  "CHAR", "INT", "FLOAT", "BOOL", "STRING"
};

static const char* kNodeRuleString[] = {
  "MODULE", "DEPENDS", "NAME", "TYPE", "ACCESS_MODIFIER", "SPECS", "SPEC",
  "PROPERTIES", "PROPERTY", "PROPERTY_FUNCTION", "FUNCTIONS", "FUNCTION",
  "NATIVE", "FUNCTION_PARAMETERS", "FUNCTION_PARAMETER", "BLOCK", "ASSIGN",
  "RETURN", "EXPRESSION", "MEMBER", "CALL", "CALL_PARAMETERS", "SYMBOL",
  "LOGOR", "LOGAND", "LOGNOT", "EQUALS", "NOT_EQUALS", "LESS", "LESS_EQUALS",
  "GREATER", "GREATER_EQUALS", "ADD", "SUB", "MUL", "DIV", "MOD", "BOOL", "INT",
  "FLOAT", "CHAR", "STRING"
};

void DebugPrintLexerToken(const LexerToken& token) {
    const int kMaxValueLen = 255;
    char value[kMaxValueLen] = "";

    switch (token.type) {
    case LexerTokenType::CHAR:
        snprintf(value, kMaxValueLen, "'%c'", token.char_const);
        break;
    case LexerTokenType::FLOAT:
        snprintf(value, kMaxValueLen, "%f", token.float_const);
        break;
    case LexerTokenType::INT:
        snprintf(value, kMaxValueLen, "%i", token.char_const);
        break;
    case LexerTokenType::ACCESS_MODIFIER:
    case LexerTokenType::KEYWORD:
    case LexerTokenType::SYMBOL:
    case LexerTokenType::TYPE:
        snprintf(value, kMaxValueLen, "%s", DebugLexerSymbolString(token.symbol));
        break;
    case LexerTokenType::NAME:
    case LexerTokenType::STRING:
        snprintf(value, kMaxValueLen, "\"%s\"", token.string_const->c_str());
        break;
    default:
        // Indicative of a bug.
        printf("**UNKNOWN_LEXER_TOKEN**");
        return;
    }

    printf("%s: %s\n", DebugLexerTokenTypeString(token.type), value);
}

const char* DebugLexerTokenTypeString(LexerTokenType type) {
  return kTokenTypeString[(int)type];
}

const char* DebugLexerSymbolString(LexerSymbol symbol) {
  return kLexerSymbolString[(int)symbol];
}

const char* DebugNodeRuleString(NodeRule rule) {
  return kNodeRuleString[(int)rule];
}

void DebugPrintNode(Node* node) {
  DebugPrintNode(node, NULL);
}

void DebugPrintNode(Node* node, Node* target_node) {

  if (node == target_node) {
    printf("--> ");
  }

  if (node == NULL) {
    printf("** NULL NODE **");
  }

  switch (node->rule()) {
    case NodeRule::NAME:
    case NodeRule::STRING:
      printf("%i:%s, \"%s\"\n",
             node->child_count(),
             DebugNodeRuleString(node->rule()),
             node->string_value()->c_str());
             break;

    case NodeRule::TYPE:
    case NodeRule::ACCESS_MODIFIER:
      printf("%i:%s, %s\n",
             node->child_count(),
             DebugNodeRuleString(node->rule()),
             DebugLexerSymbolString(node->symbol_value()));
      break;

    case NodeRule::BOOL:
    case NodeRule::NATIVE:
      printf("%i:%s, %s\n",
             node->child_count(),
             DebugNodeRuleString(node->rule()),
             node->bool_value() ? "true" : "false");
      break;

    case NodeRule::CHAR:
    case NodeRule::INT:
      printf("%i:%s, %lu\n",
             node->child_count(),
             DebugNodeRuleString(node->rule()),
             node->int_value());
      break;

    case NodeRule::FLOAT:
      printf("%i:%s, %lf\n",
             node->child_count(),
             DebugNodeRuleString(node->rule()),
             node->float_value());
      break;

    default:
      printf("%i:%s\n",
             node->child_count(),
             DebugNodeRuleString(node->rule()));
  }

  // Print child nodes.
  for (int i = 0; i < node->child_count(); i++) {
    DebugPrintNode(node->GetChild(i), target_node);
  }
}

} // namespace library
} // namspace gunderscript
