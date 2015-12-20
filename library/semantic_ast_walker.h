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
     
private:
    SymbolTable<Symbol> symbol_table_;

    void CheckValidModuleName(const std::string& module_name);
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

} // namespace library
} // namespace gunderscript

#endif // GUNDERSCRIPT_SEMANTIC_CHECKER__H__
