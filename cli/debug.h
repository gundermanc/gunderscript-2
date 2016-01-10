// Gunderscript-2 Debug Print Functions
// (C) 2015-2016 Christian Gunderman

#ifndef GUNDERSCRIPT_DEBUG__H__
#define GUNDERSCRIPT_DEBUG__H__

#include "gunderscript/compiler.h"

namespace gunderscript {

// Prints the debug representation of a lexer token to the console.
void DebugPrintLexerToken(const LexerToken& token);

// Prints the debug representation of a node and all of its children
// to the console.
void DebugPrintNode(const Node* node);

} // namespace gunderscript

#endif // GUNDERSCRIPT_DEBUG__H__