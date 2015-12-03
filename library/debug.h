// Gunderscript-2 Debug Functions
// (C) 2015 Christian Gunderman

#ifndef GUNDERSCRIPT_DEBUG__H__
#define GUNDERSCRIPT_DEBUG__H__

#include "lexer.h"
#include "node.h"

namespace gunderscript {
namespace library {

// Prints the debug representation of a lexer token to the console.
void DebugPrintLexerToken(const LexerToken& token);

// Prints the debug representation of a node and all of its children
// to the console.
void DebugPrintNode(Node* node);

// Prints the debug representation of a node and all of its children
// to the console. The target_node is marked with an arrow.
void DebugPrintNode(Node* node, Node* target_node);

} // namespace debug
} // namespace gunderscript

#endif // GUNDERSCRIPT_DEBUG__H__