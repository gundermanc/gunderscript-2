// Gunderscript-2 Parse/AST Node
// (C) 2014-2015 Christian Gunderman

#include "ast_walker.h"

namespace gunderscript {
namespace library {

// Walks through all expected children of the MODULE
// AST node (the root of the AST). Expected children of this
// node are walked recursively. Unexpected children cause an
// IllegalStateException indicating that there is a bug.
template <typename ReturnType>
void AstWalker<ReturnType>::WalkModuleChildren() {
    // We can't have a non-module node as the root. If we do, indicate
    // that this is a bug.
    if (this->root().rule() != NodeRule::MODULE) {
        throw IllegalStateException();
    }

    // Check that we have the proper number of MODULE node children.
    if (this->root().child_count() != 3) {
        throw IllegalStateException();
    }

    // Call public pure virtual function implemented by child class
    // to give them a chance to walk/modify the AST.
    WalkModule(&this->root());

    Node* name_node = this->root().child(0);
    Node* depends_node = this->root().child(1);
    Node* specs_node = this->root().child(2);

    // Check child node types.
    if (name_node->rule() != NodeRule::NAME ||
        depends_node->rule() != NodeRule::DEPENDS ||
        specs_node->rule() != NodeRule::SPECS) {
        throw IllegalStateException();
    }

    // Walk children.
    WalkModuleName(name_node);
    WalkModuleDependsChildren(depends_node);
    WalkModuleSpecsChildren(specs_node);
}

// Walks all child nodes of the DEPENDS node (nodes that indicate
// which files this script depends on). Children will all be NAME nodes
// with the name of another script file.
template <typename ReturnType>
void AstWalker<ReturnType>::WalkModuleDependsChildren(Node* depends_node) {

    // Iterate through all classes that this class depends on.
    for (int i = 0; i < depends_node->child_count(); i++) {

        // Check that each node is a NAME node with the name of a script.
        if (depends_node->child(i)->rule() != NodeRule::NAME) {
            throw IllegalStateException();
        }

        WalkModuleDependsName(depends_node->child(i));
    }
}

// Walks all child nodes of the SPECS node (nodes defining the specs/classes
// defined in this module.
// TODO: update comments.
template <typename ReturnType>
void AstWalker<ReturnType>::WalkModuleSpecsChildren(Node* specs_node) {
    throw NotImplementedException();
}

} // namespace library
} // namespace gunderscript
