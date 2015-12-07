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
template<typename ReturnType>
class AstWalker {
public:
    AstWalker(Node& root) : root_(root) { }

    void Walk() { WalkModuleChildren(); }

protected:
    Node& root() const { return this->root_; }

    virtual ReturnType WalkModule(Node* module_node) = 0;
    virtual ReturnType WalkModuleName(Node* name_node) = 0;

private:
    Node& root_;

    void WalkModuleChildren();
    void WalkModuleDependsChildren(Node* depends_node);
    void WalkModuleSpecsChildren(Node* specs_node);
};

// Instantiate template with LexerSymbol so we can unit test from external module
// TODO: ifdef DEBUG here compatible with CMAKE.
template class AstWalker<LexerSymbol>;

} // namespace library
} // namespace gunderscript

#endif // GUNDERSCRIPT_AST_WALKER__H__
