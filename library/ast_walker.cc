// Gunderscript-2 Parse/AST Node
// (C) 2014-2015 Christian Gunderman

#include "ast_walker.h"

namespace gunderscript {
namespace library {

AstWalker::AstWalker(Node* root) : root_(root) {

}

AstWalker::~AstWalker() {

}

void AstWalker::Walk() {
    // We can't have a non-module node as the root. If we do, indicate
    // that this is a bug.
    if (this->root()->rule() != NodeRule::MODULE) {
        throw IllegalStateException();
    }

    // Iterate all child nodes and walk them as expected.
    for (int i = 0; i < this->root()->child_count(); i++) {
        const Node* current_node = this->root()->child(i);

        // TODO: support for walking expected child nodes.
        switch (current_node->rule()) {
        default:
            throw NotImplementedException();
            // Uncomment this when this is done.
            //throw new IllegalStateException();
        }
    }
}

} // namespace library
} // namespace gunderscript
