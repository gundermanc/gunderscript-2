// Gunderscript-2 Semantic (type) Checker for Abstract Syntax Tree
// (C) 2015 Christian Gunderman

#include <regex>

#include "semantic_ast_walker.h"

namespace gunderscript {
namespace library {

// The pattern for checking module names.
std::regex module_name_pattern = std::regex("^([A-Z]|[a-z])+(\\.([A-Z]|[a-z])+)?$");

// Walks the MODULE node in the abstract syntax tree.
// Since there is no type information in this node, we can
// safely do nothing.
void SemanticAstWalker::WalkModule(Node* module_node) {
    // Module itself has no properties to check.
    // We instead check its child elements individually.
}

// Walks the MODULE node's NAME node. This node defines the module name
// (analogous to the Java package name) of a script file.
// The only verification performed at the moment is simple naive name
// pattern matching.
void SemanticAstWalker::WalkModuleName(Node* name_node) {
    CheckValidModuleName(*name_node->string_value());
}

// Checks that the given dependency module name is valid.
// If not, throws an exception.
// TODO: calculate dependency graph and lex/parse/typecheck the
// dependency first.
void SemanticAstWalker::WalkModuleDependsName(Node* name_node) {
    CheckValidModuleName(*name_node->string_value());
}

// Attempts to declare a new spec in the given scope. Throws if spec
// name is taken in this context.
void SemanticAstWalker::WalkSpecDeclaration(Node* access_modifier_node, Node* name_node) {
    Symbol spec_symbol(access_modifier_node->symbol_value(), *name_node->string_value());

    this->symbol_table().Put(*name_node->string_value(), spec_symbol);
}

// Checks to see if the given module name is valid. If it is not, throws
// an exception.
void SemanticAstWalker::CheckValidModuleName(const std::string& module_name) {
    if (!std::regex_match(module_name, module_name_pattern)) {
        throw SemanticAstWalkerInvalidPackageNameException(*this);
    }
}

} // namespace library
} // namespace gunderscript
