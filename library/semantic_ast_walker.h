// Gunderscript-2 Parse/AST Node
// (C) 2015 Christian Gunderman

#ifndef GUNDERSCRIPT_SEMANTIC_CHECKER__H__
#define GUNDERSCRIPT_SEMANTIC_CHECKER__H__

#include <string>
#include <vector>

#include "ast_walker.h"
#include "lexer.h"
#include "node.h"
#include "symbol.h"
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
        std::vector<LexerSymbol>& arguments_result,
        bool prescan);
    LexerSymbol WalkSpecFunctionDeclarationParameter(
        Node* spec_node,
        Node* function_node,
        Node* type_node,
        Node* name_node,
        bool prescan);
    void WalkSpecPropertyDeclaration(
        Node* spec_node,
        Node* type_node,
        Node* name_node,
        Node* get_access_modifier_node,
        Node* set_access_modifier_node,
        bool prescan);
    LexerSymbol WalkFunctionCall(
        Node* spec_node,
        Node* name_node,
        std::vector<LexerSymbol>& arguments_result);
    LexerSymbol WalkAssign(
        Node* spec_node,
        Node* name_node,
        LexerSymbol operations_result);
    LexerSymbol WalkReturn(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        PropertyFunction property_function,
        LexerSymbol expression_result,
        std::vector<LexerSymbol>* arguments_result);
    LexerSymbol WalkAdd(
        Node* spec_node,
        Node* left_node,
        Node* right_node,
        LexerSymbol left_result,
        LexerSymbol right_result);
    LexerSymbol WalkSub(
        Node* spec_node,
        Node* left_node,
        Node* right_node,
        LexerSymbol left_result,
        LexerSymbol right_result);
    LexerSymbol WalkMul(
        Node* spec_node,
        Node* left_node,
        Node* right_node,
        LexerSymbol left_result,
        LexerSymbol right_result);
    LexerSymbol WalkDiv(
        Node* spec_node,
        Node* left_node,
        Node* right_node,
        LexerSymbol left_result,
        LexerSymbol right_result);
    LexerSymbol WalkMod(
        Node* spec_node,
        Node* left_node,
        Node* right_node,
        LexerSymbol left_result,
        LexerSymbol right_result);
    LexerSymbol WalkLogAnd(
        Node* spec_node,
        Node* left_node,
        Node* right_node,
        LexerSymbol left_result,
        LexerSymbol right_result);
    LexerSymbol WalkLogNot(
        Node* spec_node,
        Node* child_node,
        LexerSymbol child_result);
    LexerSymbol WalkLogOr(
        Node* spec_node,
        Node* left_node,
        Node* right_node,
        LexerSymbol left_result,
        LexerSymbol right_result);
    LexerSymbol WalkGreater(
        Node* spec_node,
        Node* left_node,
        Node* right_node,
        LexerSymbol left_result,
        LexerSymbol right_result);
    LexerSymbol WalkEquals(
        Node* spec_node,
        Node* left_node,
        Node* right_node,
        LexerSymbol left_result,
        LexerSymbol right_result);
    LexerSymbol WalkNotEquals(
        Node* spec_node,
        Node* left_node,
        Node* right_node,
        LexerSymbol left_result,
        LexerSymbol right_result);
    LexerSymbol WalkLess(
        Node* spec_node,
        Node* left_node,
        Node* right_node,
        LexerSymbol left_result,
        LexerSymbol right_result);
    LexerSymbol WalkGreaterEquals(
        Node* spec_node,
        Node* left_node,
        Node* right_node,
        LexerSymbol left_result,
        LexerSymbol right_result);
    LexerSymbol WalkLessEquals(
        Node* spec_node,
        Node* left_node,
        Node* right_node,
        LexerSymbol left_result,
        LexerSymbol right_result);
    LexerSymbol WalkBool(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        PropertyFunction property_function,
        Node* bool_node);
    LexerSymbol WalkInt(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        PropertyFunction property_function,
        Node* int_node);
    LexerSymbol WalkFloat(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        PropertyFunction property_function,
        Node* float_node);
    LexerSymbol WalkString(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        PropertyFunction property_function,
        Node* string_node);
    LexerSymbol WalkChar(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        PropertyFunction property_function,
        Node* char_node);
    LexerSymbol WalkVariable(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        PropertyFunction property_function,
        Node* name_node);
    LexerSymbol WalkAnyType(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        PropertyFunction property_function,
        Node* any_type_node);

    void WalkSpecFunctionChildren(
        Node* spec_node,
        Node* function_node,
        bool prescan);
    void WalkBlockChildren(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        PropertyFunction property_function,
        Node* block,
        std::vector<LexerSymbol>* arguments_result);
     
private:
    SymbolTable<Symbol> symbol_table_;

    void CheckValidModuleName(const std::string& module_name);
    void CheckAccessModifier(
        const std::string& caller_class,
        const std::string& callee_class,
        LexerSymbol callee_access_modifier);
    LexerSymbol CalculateResultantType(LexerSymbol left, LexerSymbol right);
    LexerSymbol CalculateNumericResultantType(LexerSymbol left, LexerSymbol right);
    LexerSymbol CalculateBoolResultantType(LexerSymbol left, LexerSymbol right);
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

// SemanticAstWalker member not accessible exception.
class SemanticAstWalkerNotAccessibleException : public SemanticAstWalkerException {
public:
    SemanticAstWalkerNotAccessibleException(const SemanticAstWalker& walker) :
        SemanticAstWalkerException(walker,
            "Class or class member not accessible.") { }
};

// SemanticAstWalker type mismatch exception.
class SemanticAstWalkerTypeMismatchException : public SemanticAstWalkerException {
public:
    SemanticAstWalkerTypeMismatchException(const SemanticAstWalker& walker) :
        SemanticAstWalkerException(walker,
            "Invalid type in operation.") { }
};

} // namespace library
} // namespace gunderscript

#endif // GUNDERSCRIPT_SEMANTIC_CHECKER__H__
