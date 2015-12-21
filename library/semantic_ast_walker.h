// Gunderscript-2 Parse/AST Node
// (C) 2015 Christian Gunderman

#ifndef GUNDERSCRIPT_SEMANTIC_CHECKER__H__
#define GUNDERSCRIPT_SEMANTIC_CHECKER__H__

#include <string>
#include <vector>

#include "ast_walker.h"
#include "lexer.h"
#include "node.h"
#include "symbol_table.h"

namespace gunderscript {
namespace library {

// Type checking abstract syntax tree walker.
// Walks along the AST and checks for type correctness.
class SemanticAstWalker : public AstWalker<LexerSymbol> {
public:

    SemanticAstWalker(Node& node) : AstWalker(node), symbol_table_() { }

protected:
    void WalkModule(Node* module_node);
    void WalkModuleName(Node* module_name);
    void WalkModuleDependsName(Node* name_node);
    void WalkSpecDeclaration(Node* access_modifier_node, Node* name_node);
    void WalkSpecFunctionDeclaration(
        Node* spec_node,
        Node* access_modifier_node,
        Node* native_node,
        Node* type_node,
        Node* name_node,
        Node* block_node,
        std::vector<LexerSymbol>& arguments_result);
    LexerSymbol WalkSpecFunctionDeclarationParameter(
        Node* spec_node,
        Node* function_node,
        Node* type_node,
        Node* name_node);
    void WalkSpecPropertyDeclaration(
        Node* spec_node,
        Node* type_node,
        Node* name_node,
        Node* get_access_modifier_node,
        Node* set_access_modifier_node);
    LexerSymbol WalkFunctionCall(
        Node* spec_node,
        Node* name_node,
        std::vector<LexerSymbol>& arguments_result);
     
private:
    SymbolTable<Symbol> symbol_table_;

    void CheckValidModuleName(const std::string& module_name);
    void CheckAccessModifier(
        const std::string& caller_class,
        const std::string& callee_class,
        LexerSymbol callee_access_modifier);
};

// SemanticAstWalker Exceptions Parent Class
// All Parser exceptions descend from this class.
class SemanticAstWalkerException : public Exception {
public:
    SemanticAstWalkerException(const SemanticAstWalker& walker) : Exception(), walker_(walker) { }
    SemanticAstWalkerException(const SemanticAstWalker& walker,
        const std::string& message) : Exception(message), walker_(walker) { }
    const SemanticAstWalker& walker() { return walker_; }

private:
    const SemanticAstWalker& walker_;
};

// SemanticAstWalker invalid package name exception.
class SemanticAstWalkerInvalidPackageNameException : public SemanticAstWalkerException {
public:
    SemanticAstWalkerInvalidPackageNameException(const SemanticAstWalker& walker) :
        SemanticAstWalkerException(walker,
            "Invalid package name.") { }
};

// SemanticAstWalker member not accessible exception.s
class SemanticAstWalkerNotAccessibleException : public SemanticAstWalkerException {
public:
    SemanticAstWalkerNotAccessibleException(const SemanticAstWalker& walker) :
        SemanticAstWalkerException(walker,
            "Class or class member not accessible.") { }
};

} // namespace library
} // namespace gunderscript

#endif // GUNDERSCRIPT_SEMANTIC_CHECKER__H__
