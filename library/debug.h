// Gunderscript-2 Debug Functions
// (C) 2015 Christian Gunderman

#ifndef GUNDERSCRIPT_DEBUG__H__
#define GUNDERSCRIPT_DEBUG__H__

#include "lexer.h"
#include "node.h"

namespace gunderscript {
namespace library {

void DebugPrintLexerToken(const LexerToken& token);
const char* DebugLexerTokenTypeString(LexerTokenType type);
const char* DebugLexerSymbolString(LexerSymbol symbol);
const char* DebugNodeRuleString(NodeRule rule);
void DebugPrintNode(Node* node);
void DebugPrintNode(Node* node, Node* target_node);

} // namespace debug
} // namespace gunderscript

#endif // GUNDERSCRIPT_DEBUG__H__