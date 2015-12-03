// Gunderscript-2 Abstract Syntax Tree Walker Class
// (C) 2015 Christian Gunderman

#ifndef GUNDERSCRIPT_AST_WALKER__H__
#define GUNDERSCRIPT_AST_WALKER__H__

#include <string>
#include <vector>

#include "node.h"

namespace gunderscript {
namespace library {

// Class for walking through the Abstract Syntax Tree Structure.
class AstWalker {
public:
    AstWalker(Node* root);
    virtual ~AstWalker();
    void Walk();

    const Node* root() const { return this->root_;  }
private:
    const Node* root_;
};

} // namespace library
} // namespace gunderscript

#endif // GUNDERSCRIPT_AST_WALKER__H__
