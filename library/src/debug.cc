// Gunderscript-2 Debug Functions
// (C) 2015 Christian Gunderman

// This code is only compiled into Gunderscript when building in all-debug
// build configuration. Put non-shipping debug code here.

#include <cstdio>

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
    DebugPrintNode(node->GetChild(i));
  }
}

} // namespace library
} // namspace gunderscript
