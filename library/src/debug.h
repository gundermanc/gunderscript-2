// Gunderscript-2 Debug Functions
// (C) 2015 Christian Gunderman

// This code is only compiled into Gunderscript when building in all-debug
// build configuration. Put non-shipping debug code here.
#ifdef DEBUG

#include "lexer.h"
#include "node.h"

namespace gunderscript {
namespace library {

const char* DebugLexerTokenTypeString(LexerTokenType type);
const char* DebugLexerSymbolString(LexerSymbol symbol);
const char* DebugNodeRuleString(NodeRule rule);
void DebugPrintNode(Node* node);
void DebugPrintNode(Node* node, Node* target_node);

} // namespace debug
} // namespace gunderscript

#endif // DEBUG
