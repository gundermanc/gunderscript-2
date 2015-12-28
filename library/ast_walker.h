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

    // Mandatory Pure-Virtual (Abstract) methods that MUST be implemented
    // by child classes:

    virtual void WalkModule(Node* module_node) = 0;
    virtual void WalkModuleName(Node* name_node) = 0;
    virtual void WalkModuleDependsName(Node* name_node) = 0;
    virtual void WalkSpecDeclaration(Node* access_modifier_node, Node* name_node) = 0;
    virtual void WalkSpecFunctionDeclaration(
        Node* spec_node,
        Node* access_modifier_node,
        Node* native_node,
        Node* type_node,
        Node* name_node,
        Node* block_node,
        std::vector<ReturnType>& argument_result) = 0;
    virtual ReturnType WalkSpecFunctionDeclarationParameter(
        Node* spec_node,
        Node* function_node,
        Node* type_node,
        Node* name_node) = 0;
    virtual void WalkSpecPropertyDeclaration(
        Node* spec_node,
        Node* type_node,
        Node* name_node,
        Node* get_access_modifier_node,
        Node* set_access_modifier_node) = 0;
    virtual ReturnType WalkFunctionCall(
        Node* spec_node,
        Node* name_node,
        std::vector<ReturnType>& arguments_result) = 0;
    virtual ReturnType WalkAssign(
        Node* spec_node,
        Node* name_node,
        ReturnType operations_result) = 0;
    virtual ReturnType WalkReturn(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        ReturnType expression_result,
        std::vector<LexerSymbol>* arguments_result) = 0;
    virtual ReturnType WalkAdd(
        Node* spec_node,
        Node* left_node,
        Node* right_node,
        ReturnType left_result,
        ReturnType right_result) = 0;
    virtual ReturnType WalkSub(
        Node* spec_node,
        Node* left_node,
        Node* right_node,
        ReturnType left_result,
        ReturnType right_result) = 0;
    virtual ReturnType WalkMul(
        Node* spec_node,
        Node* left_node,
        Node* right_node,
        ReturnType left_result,
        ReturnType right_result) = 0;
    virtual ReturnType WalkDiv(
        Node* spec_node,
        Node* left_node,
        Node* right_node,
        ReturnType left_result,
        ReturnType right_result) = 0;
    virtual ReturnType WalkMod(
        Node* spec_node,
        Node* left_node,
        Node* right_node,
        ReturnType left_result,
        ReturnType right_result) = 0;
    virtual ReturnType WalkLogAnd(
        Node* spec_node,
        Node* left_node,
        Node* right_node,
        ReturnType left_result,
        ReturnType right_result) = 0;
    virtual ReturnType WalkLogOr(
        Node* spec_node,
        Node* left_node,
        Node* right_node,
        ReturnType left_result,
        ReturnType right_result) = 0;
    virtual ReturnType WalkGreater(
        Node* spec_node,
        Node* left_node,
        Node* right_node,
        ReturnType left_result,
        ReturnType right_result) = 0;
    virtual ReturnType WalkEquals(
        Node* spec_node,
        Node* left_node,
        Node* right_node,
        ReturnType left_result,
        ReturnType right_result) = 0;
    virtual ReturnType WalkNotEquals(
        Node* spec_node,
        Node* left_node,
        Node* right_node,
        ReturnType left_result,
        ReturnType right_result) = 0;
    virtual ReturnType WalkLess(
        Node* spec_node,
        Node* left_node,
        Node* right_node,
        ReturnType left_result,
        ReturnType right_result) = 0;
    virtual ReturnType WalkGreaterEquals(
        Node* spec_node,
        Node* left_node,
        Node* right_node,
        ReturnType left_result,
        ReturnType right_result) = 0;
    virtual ReturnType WalkLessEquals(
        Node* spec_node,
        Node* left_node,
        Node* right_node,
        ReturnType left_result,
        ReturnType right_result) = 0;
    virtual ReturnType WalkBool(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        Node* bool_node) = 0;
    virtual ReturnType WalkInt(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        Node* int_node) = 0;
    virtual ReturnType WalkFloat(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        Node* float_node) = 0;
    virtual ReturnType WalkString(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        Node* string_node) = 0;    
    virtual ReturnType WalkChar(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        Node* char_node) = 0;
    virtual ReturnType WalkVariable(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        Node* name_node) = 0;
    virtual ReturnType WalkAnyType(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        Node* any_type_node) = 0;

    // Optional Implementation method(s) that are critical for proper operation
    // of ASTWalker that MAY be optionally overridden by subclasses for increased
    // customization.

    virtual void WalkBlockChildren(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        Node* block,
        std::vector<ReturnType>* arguments_result);

private:
    Node& root_;

    void WalkModuleChildren();
    void WalkModuleDependsChildren(Node* depends_node);
    void WalkModuleSpecsChildren(Node* specs_node);
    void WalkSpec(Node* spec_node);
    void WalkSpecFunctionsChildren(Node* spec_node, Node* functions_node);
    void WalkSpecFunctionDeclarationParametersChildren(
        Node* spec_node,
        Node* function_node,
        Node* params_node,
        std::vector<ReturnType>& argument_result);
    void WalkSpecPropertiesChildren(Node* spec_node, Node* properties_node);
    void WalkFunctionCallChildren(
        Node* spec_node,
        Node* function_node,
        Node* call_node);
    ReturnType WalkAssignChildren(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        Node* assign_node);
    void WalkReturnChildren(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        Node* return_node,
        std::vector<ReturnType>* arguments_result);
    ReturnType WalkExpressionChildren(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        Node* expression_node);
    ReturnType WalkBinaryOperationChildren(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        Node* binary_operation_node);
    ReturnType WalkAtomicExpressionChildren(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        Node* atomic_node);
    void CheckNodeRule(Node* node, NodeRule rule);
};

// Instantiate template with LexerSymbol so we can unit test from external module
// TODO: ifdef DEBUG here compatible with CMAKE.
template class AstWalker<LexerSymbol>;

} // namespace library
} // namespace gunderscript

#endif // GUNDERSCRIPT_AST_WALKER__H__
