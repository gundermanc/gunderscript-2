// Gunderscript-2 Parse/AST Node
// (C) 2014 Christian Gunderman

#ifndef GUNDERSCRIPT_SEMANTIC_CHECKER__H__
#define GUNDERSCRIPT_SEMANTIC_CHECKER__H__

#include <string>
#include <vector>

#include "ast_walker.h"

namespace gunderscript {
namespace library {

// Type checking abstract syntax tree walker.
// Walks along the AST and checks for type correctness.
class SemanticAstWalker : public AstWalker {
public:
    ~SemanticAstWalker() { }
};

} // namespace library
} // namespace gunderscript

#endif // GUNDERSCRIPT_SEMANTIC_CHECKER__H__
