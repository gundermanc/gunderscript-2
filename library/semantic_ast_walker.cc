// Gunderscript-2 Semantic (type) Checker for Abstract Syntax Tree
// (C) 2015 Christian Gunderman

#include <regex>

#include "semantic_ast_walker.h"

namespace gunderscript {
namespace library {

// The pattern for checking module names.
std::regex module_name_pattern = std::regex("^([A-Z]|[a-z])+(\.([A-Z]|[a-z])+)?$");

// Walks the MODULE node in the abstract syntax tree.
// Since there is no type information in this node, we can
// safely do nothing.
LexerSymbol SemanticAstWalker::WalkModule(Node* module_node) {
    // Module itself has no properties to check.
    // We instead check its child elements individually.

    return LexerSymbol::TNULL;
}

// Walks the MODULE node's NAME node. This node defines the module name
// (analogous to the Java package name) of a script file.
// The only verification performed at the moment is simple naive name
// pattern matching.
LexerSymbol SemanticAstWalker::WalkModuleName(Node* name_node) {

    // Check that the module name is a valid name.
    // Valid names are a series of 1 or more dot separated words.
    // TODO: broader range of accepted names here.
    if (!std::regex_match(*name_node->string_value(), module_name_pattern)) {
        throw SemanticAstWalkerInvalidPackageNameException(*this);
    }

    // Return type does nothing for this checker.
    return LexerSymbol::TNULL;
}

} // namespace library
} // namespace gunderscript
