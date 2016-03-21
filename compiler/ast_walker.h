// Gunderscript-2 Abstract Syntax Tree Walker Class
// (C) 2015 Christian Gunderman

#ifndef GUNDERSCRIPT_AST_WALKER__H__
#define GUNDERSCRIPT_AST_WALKER__H__

#include <string>
#include <vector>

#include "gunderscript/node.h"
#include "gunderscript/symbol.h"

namespace gunderscript {
namespace compiler {

// Class for walking through the Abstract Syntax Tree Structure.
template<typename ReturnType>
class AstWalker {
public:
    AstWalker(Node& root) : root_(root) { }
  virtual ~AstWalker() { }

    void Walk() { WalkModuleChildren(); }

protected:
    Node& root() const { return this->root_; }

    // Mandatory Pure-Virtual (Abstract) methods that MUST be implemented
    // by child classes:

    virtual void WalkModule(Node* module_node) = 0;
    virtual void WalkModuleName(Node* name_node) = 0;
    virtual void WalkModuleDependsName(Node* name_node) = 0;
    virtual void WalkSpecDeclaration(
        Node* spec_node,
        Node* access_modifier_node,
        Node* name_node) = 0;
    virtual void WalkFunctionDeclaration(
        Node* spec_node,
        Node* function_node,
        Node* access_modifier_node,
        Node* type_node,
        Node* name_node,
        Node* block_node,
        std::vector<ReturnType>& argument_result,
        bool prescan) = 0;
    virtual ReturnType WalkSpecFunctionDeclarationParameter(
        Node* spec_node,
        Node* function_node,
        Node* type_node,
        Node* function_param_node,
        Node* name_node,
        bool prescan) = 0;
    virtual void WalkSpecPropertyDeclaration(
        Node* spec_node,
        Node* type_node,
        Node* name_node,
        Node* get_property_function_node,
        Node* set_property_function_node,
        Node* get_access_modifier_node,
        Node* set_access_modifier_node,
        bool prescan) = 0;
    virtual ReturnType WalkFunctionCall(
        Node* spec_node,
        Node* name_node,
        Node* call_node,
        std::vector<ReturnType>& arguments_result) = 0;
    virtual void WalkIfStatement(
        Node* spec_node,
        Node* if_node,
        ReturnType condition_result) = 0;
    virtual void WalkForStatement(
        Node* spec_node,
        Node* for_node,
        ReturnType condition_result) = 0;
    virtual ReturnType WalkAssign(
        Node* spec_node,
        Node* name_node,
        Node* symbol_node,
        Node* assign_node,
        ReturnType operations_result) = 0;
    virtual ReturnType WalkReturn(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        PropertyFunction property_function,
        ReturnType expression_result,
        std::vector<ReturnType>* arguments_result) = 0;
    virtual ReturnType WalkAdd(
        Node* spec_node,
        Node* add_node,
        Node* left_node,
        Node* right_node,
        ReturnType left_result,
        ReturnType right_result) = 0;
    virtual ReturnType WalkSub(
        Node* spec_node,
        Node* sub_node,
        Node* left_node,
        Node* right_node,
        ReturnType left_result,
        ReturnType right_result) = 0;
    virtual ReturnType WalkMul(
        Node* spec_node,
        Node* mul_node,
        Node* left_node,
        Node* right_node,
        ReturnType left_result,
        ReturnType right_result) = 0;
    virtual ReturnType WalkDiv(
        Node* spec_node,
        Node* div_node,
        Node* left_node,
        Node* right_node,
        ReturnType left_result,
        ReturnType right_result) = 0;
    virtual ReturnType WalkMod(
        Node* spec_node,
        Node* mod_node,
        Node* left_node,
        Node* right_node,
        ReturnType left_result,
        ReturnType right_result) = 0;
    virtual ReturnType WalkLogAnd(
        Node* spec_node,
        Node* log_and_node,
        Node* left_node,
        Node* right_node,
        ReturnType left_result,
        ReturnType right_result) = 0;
    virtual ReturnType WalkLogOr(
        Node* spec_node,
        Node* log_or_node,
        Node* left_node,
        Node* right_node,
        ReturnType left_result,
        ReturnType right_result) = 0;
    virtual ReturnType WalkLogNot(
        Node* spec_node,
        Node* log_not_node,
        Node* child_node,
        ReturnType child_result) = 0;
    virtual ReturnType WalkGreater(
        Node* spec_node,
        Node* greater_node,
        Node* left_node,
        Node* right_node,
        ReturnType left_result,
        ReturnType right_result) = 0;
    virtual ReturnType WalkEquals(
        Node* spec_node,
        Node* equals_node,
        Node* left_node,
        Node* right_node,
        ReturnType left_result,
        ReturnType right_result) = 0;
    virtual ReturnType WalkNotEquals(
        Node* spec_node,
        Node* not_equals_node,
        Node* left_node,
        Node* right_node,
        ReturnType left_result,
        ReturnType right_result) = 0;
    virtual ReturnType WalkLess(
        Node* spec_node,
        Node* less_node,
        Node* left_node,
        Node* right_node,
        ReturnType left_result,
        ReturnType right_result) = 0;
    virtual ReturnType WalkGreaterEquals(
        Node* spec_node,
        Node* greater_equals_node,
        Node* left_node,
        Node* right_node,
        ReturnType left_result,
        ReturnType right_result) = 0;
    virtual ReturnType WalkLessEquals(
        Node* spec_node,
        Node* less_equals_node,
        Node* left_node,
        Node* right_node,
        ReturnType left_result,
        ReturnType right_result) = 0;
    virtual ReturnType WalkBool(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        PropertyFunction property_function,
        Node* bool_node) = 0;
    virtual ReturnType WalkInt(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        PropertyFunction property_function,
        Node* int_node) = 0;
    virtual ReturnType WalkFloat(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        PropertyFunction property_function,
        Node* float_node) = 0;
    virtual ReturnType WalkString(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        PropertyFunction property_function,
        Node* string_node) = 0;    
    virtual ReturnType WalkChar(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        PropertyFunction property_function,
        Node* char_node) = 0;
    virtual ReturnType WalkVariable(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        PropertyFunction property_function,
        Node* variable_node,
        Node* name_node) = 0;
    virtual ReturnType WalkAnyType(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        PropertyFunction property_function,
        Node* any_type_node) = 0;

    // Optional Implementation method(s) that are critical for proper operation
    // of ASTWalker that MAY be optionally overridden by subclasses for increased
    // customization.

    virtual void WalkFunctionChildren(
        Node* spec_node,
        Node* function_node,
        bool prescan);
    virtual void WalkBlockChildren(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        PropertyFunction property_function,
        Node* block,
        std::vector<ReturnType>* arguments_result);
    virtual ReturnType WalkExpressionChildren(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        PropertyFunction property_function,
        Node* expression_node);
    virtual void WalkIfStatementChildren(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        PropertyFunction property_function,
        Node* if_node,
        std::vector<ReturnType>* arguments_result);
    virtual void WalkForStatementChildren(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        PropertyFunction property_function,
        Node* for_node,
        std::vector<ReturnType>* arguments_result);

private:
    Node& root_;

    void WalkModuleChildren();
    void WalkModuleDependsChildren(Node* depends_node);
    void WalkModuleSpecsChildren(Node* specs_node);
    void WalkSpec(Node* spec_node);
    void WalkFunctionsChildren(Node* spec_node, Node* functions_node);
    void WalkFunctionDeclarationParametersChildren(
        Node* spec_node,
        Node* function_node,
        Node* params_node,
        std::vector<ReturnType>& argument_result,
        bool prescan);
    void WalkPropertiesFunctionsPrescanChildren(
        Node* spec_node,
        Node* functions_node,
        Node* properties_node);
    void WalkSpecPropertiesChildren(Node* spec_node, Node* properties_node);
    void WalkSpecPropertyChildren(
        Node* spec_node,
        Node* property_node,
        bool prescan);
    ReturnType WalkFunctionCallChildren(
        Node* spec_node,
        Node* function_node,
        Node* call_node);
    ReturnType WalkAssignChildren(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        PropertyFunction property_function,
        Node* assign_node);
    void WalkReturnChildren(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        PropertyFunction property_function,
        Node* return_node,
        std::vector<ReturnType>* arguments_result);
    ReturnType WalkSubExpressionChildren(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        PropertyFunction property_function,
        Node* expression_node);
    ReturnType WalkBinaryOperationChildren(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        PropertyFunction property_function,
        Node* binary_operation_node);
    ReturnType WalkAtomicExpressionChildren(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        PropertyFunction property_function,
        Node* atomic_node);
};

} // namespace library
} // namespace gunderscript

#endif // GUNDERSCRIPT_AST_WALKER__H__
