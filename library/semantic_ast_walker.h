// Gunderscript-2 Parse/AST Node
// (C) 2015 Christian Gunderman

#ifndef GUNDERSCRIPT_SEMANTIC_CHECKER__H__
#define GUNDERSCRIPT_SEMANTIC_CHECKER__H__

#include <string>
#include <vector>

#include "gunderscript/node.h"
#include "gunderscript/type.h"

#include "ast_walker.h"
#include "lexer.h"
#include "symbol.h"
#include "symbol_table.h"

namespace gunderscript {
namespace library {

// Type checking abstract syntax tree walker.
// Walks along the AST and checks for type correctness.
class SemanticAstWalker : public AstWalker<Type> {
public:

    SemanticAstWalker(Node& node);

protected:
    void WalkModule(Node* module_node);
    void WalkModuleName(Node* name_node);
    void WalkModuleDependsName(Node* name_node);
    void WalkSpecDeclaration(Node* access_modifier_node, Node* name_node);
    void WalkSpecFunctionDeclaration(
        Node* spec_node,
        Node* access_modifier_node,
        Node* native_node,
        Node* type_node,
        Node* name_node,
        Node* block_node,
        std::vector<Type>& arguments_result,
        bool prescan);
    Type WalkSpecFunctionDeclarationParameter(
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
    Type WalkFunctionCall(
        Node* spec_node,
        Node* name_node,
        std::vector<Type>& arguments_result);
    Type WalkAssign(
        Node* spec_node,
        Node* name_node,
        Type operations_result);
    Type WalkReturn(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        PropertyFunction property_function,
        Type expression_result,
        std::vector<Type>* arguments_result);
    Type WalkAdd(
        Node* spec_node,
        Node* left_node,
        Node* right_node,
        Type left_result,
        Type right_result);
    Type WalkSub(
        Node* spec_node,
        Node* left_node,
        Node* right_node,
        Type left_result,
        Type right_result);
    Type WalkMul(
        Node* spec_node,
        Node* left_node,
        Node* right_node,
        Type left_result,
        Type right_result);
    Type WalkDiv(
        Node* spec_node,
        Node* left_node,
        Node* right_node,
        Type left_result,
        Type right_result);
    Type WalkMod(
        Node* spec_node,
        Node* left_node,
        Node* right_node,
        Type left_result,
        Type right_result);
    Type WalkLogAnd(
        Node* spec_node,
        Node* left_node,
        Node* right_node,
        Type left_result,
        Type right_result);
    Type WalkLogNot(
        Node* spec_node,
        Node* child_node,
        Type child_result);
    Type WalkLogOr(
        Node* spec_node,
        Node* left_node,
        Node* right_node,
        Type left_result,
        Type right_result);
    Type WalkGreater(
        Node* spec_node,
        Node* left_node,
        Node* right_node,
        Type left_result,
        Type right_result);
    Type WalkEquals(
        Node* spec_node,
        Node* left_node,
        Node* right_node,
        Type left_result,
        Type right_result);
    Type WalkNotEquals(
        Node* spec_node,
        Node* left_node,
        Node* right_node,
        Type left_result,
        Type right_result);
    Type WalkLess(
        Node* spec_node,
        Node* left_node,
        Node* right_node,
        Type left_result,
        Type right_result);
    Type WalkGreaterEquals(
        Node* spec_node,
        Node* left_node,
        Node* right_node,
        Type left_result,
        Type right_result);
    Type WalkLessEquals(
        Node* spec_node,
        Node* left_node,
        Node* right_node,
        Type left_result,
        Type right_result);
    Type WalkBool(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        PropertyFunction property_function,
        Node* bool_node);
    Type WalkInt(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        PropertyFunction property_function,
        Node* int_node);
    Type WalkFloat(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        PropertyFunction property_function,
        Node* float_node);
    Type WalkString(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        PropertyFunction property_function,
        Node* string_node);
    Type WalkChar(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        PropertyFunction property_function,
        Node* char_node);
    Type WalkVariable(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        PropertyFunction property_function,
        Node* name_node);
    Type WalkAnyType(
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
        std::vector<Type>* arguments_result);
     
private:
    SymbolTable<Symbol> symbol_table_;

    void CheckValidModuleName(const std::string& module_name);
    void CheckAccessModifier(
        const std::string& caller_class,
        const std::string& callee_class,
        LexerSymbol callee_access_modifier);
    Type CalculateResultantType(Type left, Type right);
    Type CalculateNumericResultantType(Type left, Type right);
    Type CalculateBoolResultantType(Type left, Type right);
    Type ResolveTypeNode(Node* type_node);
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
