// Gunderscript-2 Semantic (type) Checker for Abstract Syntax Tree
// (C) 2015-2016 Christian Gunderman

#ifndef GUNDERSCRIPT_SEMANTIC_CHECKER__H__
#define GUNDERSCRIPT_SEMANTIC_CHECKER__H__

#include <string>
#include <vector>

#include "gunderscript/lexer_resources.h"
#include "gunderscript/node.h"
#include "gunderscript/symbol.h"

#include "ast_walker.h"
#include "symbol_table.h"

namespace gunderscript {
namespace compiler {

// Type checking abstract syntax tree walker.
// Walks along the AST and checks for type correctness.
class SemanticAstWalker : public AstWalker<const SymbolBase*> {
public:

    SemanticAstWalker(Node& node);

    const SymbolTable<const SymbolBase*>& symbol_table() const { return symbol_table_; }

protected:
    void WalkModule(Node* module_node);
    void WalkModuleName(Node* name_node);
    void WalkModuleDependsName(Node* name_node);
    void WalkSpecDeclaration(
        Node* spec_node,
        Node* access_modifier_node,
        Node* type_node,
        bool prescan);
    void WalkFunctionDeclaration(
        Node* spec_node,
        Node* function_node,
        Node* access_modifier_node,
        Node* type_node,
        Node* name_node,
        Node* block_node,
        std::vector<const SymbolBase*>& arguments_result,
        bool prescan);
    const SymbolBase* WalkSpecFunctionDeclarationParameter(
        Node* spec_node,
        Node* function_node,
        Node* type_node,
        Node* function_param_node,
        Node* name_node,
        bool prescan);
    void WalkSpecPropertyDeclaration(
        Node* spec_node,
        Node* type_node,
        Node* name_node,
        Node* get_property_function_node,
        Node* set_property_function_node,
        Node* get_access_modifier_node,
        Node* set_access_modifier_node,
        bool prescan);
    const SymbolBase* WalkFunctionCall(
        Node* spec_node,
        Node* name_node,
        Node* call_none,
        std::vector<const SymbolBase*>& arguments_result);
    const SymbolBase* SemanticAstWalker::WalkFunctionCall(
        Node* spec_node,
        const std::string& spec_name,
        const std::string& function_name,
        Node* call_node,
        std::vector<const SymbolBase*>& arguments_result);
    const SymbolBase* WalkMemberFunctionCall(
        Node* spec_node,
        Node* member_node,
        const SymbolBase* left_result,
        Node* right_node,
        std::vector<const SymbolBase*>& arguments_result);
    const SymbolBase* WalkMemberPropertyGet(
        Node* spec_node,
        Node* member_node,
        const SymbolBase* left_result,
        Node* right_node);
    void WalkIfStatement(
        Node* spec_node,
        Node* if_node,
        const SymbolBase* condition_result);
    void WalkForStatement(
        Node* spec_node,
        Node* for_node,
        const SymbolBase* condition_result);
    const SymbolBase* WalkAssign(
        Node* spec_node,
        Node* name_node,
        Node* symbol_node,
        Node* assign_node,
        const SymbolBase* operations_result);
    const SymbolBase* WalkReturn(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        PropertyFunction property_function,
        const SymbolBase** expression_result,
        std::vector<const SymbolBase*>* arguments_result);
    const SymbolBase* WalkAdd(
        Node* spec_node,
        Node* add_node,
        Node* left_node,
        Node* right_node,
        const SymbolBase* left_result,
        const SymbolBase* right_result);
    const SymbolBase* WalkSub(
        Node* spec_node,
        Node* sub_node,
        Node* left_node,
        Node* right_node,
        const SymbolBase* left_result,
        const SymbolBase* right_result);
    const SymbolBase* WalkMul(
        Node* spec_node,
        Node* mul_node,
        Node* left_node,
        Node* right_node,
        const SymbolBase* left_result,
        const SymbolBase* right_result);
    const SymbolBase* WalkDiv(
        Node* spec_node,
        Node* div_node,
        Node* left_node,
        Node* right_node,
        const SymbolBase* left_result,
        const SymbolBase* right_result);
    const SymbolBase* WalkMod(
        Node* spec_node,
        Node* mod_node,
        Node* left_node,
        Node* right_node,
        const SymbolBase* left_result,
        const SymbolBase* right_result);
    const SymbolBase* WalkLogAnd(
        Node* spec_node,
        Node* log_and_node,
        Node* left_node,
        Node* right_node,
        const SymbolBase* left_result,
        const SymbolBase* right_result);
    const SymbolBase* WalkLogNot(
        Node* spec_node,
        Node* log_not_node,
        Node* child_node,
        const SymbolBase* child_result);
    const SymbolBase* WalkLogOr(
        Node* spec_node,
        Node* log_or_node,
        Node* left_node,
        Node* right_node,
        const SymbolBase* left_result,
        const SymbolBase* right_result);
    const SymbolBase* WalkGreater(
        Node* spec_node,
        Node* greater_node,
        Node* left_node,
        Node* right_node,
        const SymbolBase* left_result,
        const SymbolBase* right_result);
    const SymbolBase* WalkEquals(
        Node* spec_node,
        Node* equals_node,
        Node* left_node,
        Node* right_node,
        const SymbolBase* left_result,
        const SymbolBase* right_result);
    const SymbolBase* WalkNotEquals(
        Node* spec_node,
        Node* not_equals_node,
        Node* left_node,
        Node* right_node,
        const SymbolBase* left_result,
        const SymbolBase* right_result);
    const SymbolBase* WalkLess(
        Node* spec_node,
        Node* less_node,
        Node* left_node,
        Node* right_node,
        const SymbolBase* left_result,
        const SymbolBase* right_result);
    const SymbolBase* WalkGreaterEquals(
        Node* spec_node,
        Node* greater_equals_node,
        Node* left_node,
        Node* right_node,
        const SymbolBase* left_result,
        const SymbolBase* right_result);
    const SymbolBase* WalkLessEquals(
        Node* spec_node,
        Node* less_equals_node,
        Node* left_node,
        Node* right_node,
        const SymbolBase* left_result,
        const SymbolBase* right_result);
    const SymbolBase* WalkBool(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        PropertyFunction property_function,
        Node* bool_node);
    const SymbolBase* WalkInt(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        PropertyFunction property_function,
        Node* int_node);
    const SymbolBase* WalkFloat(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        PropertyFunction property_function,
        Node* float_node);
    const SymbolBase* WalkString(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        PropertyFunction property_function,
        Node* string_node);
    const SymbolBase* WalkChar(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        PropertyFunction property_function,
        Node* char_node);
    const SymbolBase* WalkVariable(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        PropertyFunction property_function,
        Node* atomic_node,
        Node* name_node);
    const SymbolBase* WalkAnyType(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        PropertyFunction property_function,
        Node* any_type_node);

    void WalkSpec(Node* spec_node, PrescanMode prescan);

    void WalkFunctionChildren(
        Node* spec_node,
        Node* function_node,
        bool prescan);
    void WalkBlockChildren(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        PropertyFunction property_function,
        Node* block,
        std::vector<const SymbolBase*>* arguments_result);
    const SymbolBase* WalkExpressionChildren(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        PropertyFunction property_function,
        Node* expression_node);
    const SymbolBase* WalkNewExpression(
        Node* new_node,
        Node* type_node,
        std::vector<const SymbolBase*>& arguments_result);
    const SymbolBase* WalkDefaultExpression(
        Node* default_node,
        Node* type_node);
     
private:
    SymbolTable<const SymbolBase*> symbol_table_;

    void CheckValidModuleName(const std::string& module_name, int line, int column);
    void CheckAccessModifier(
        const std::string& caller_class,
        const std::string& callee_class,
        LexerSymbol callee_access_modifier,
        int line,
        int column);
    const SymbolBase* CalculateResultantType(
        const SymbolBase* left,
        const SymbolBase* right,
        int line,
        int column,
        ExceptionStatus type_mismatch_error);
    const SymbolBase* CalculateNumericResultantType(
        const SymbolBase* left, 
        const SymbolBase* right,
        int line,
        int column,
        ExceptionStatus type_mismatch_error);
    const SymbolBase* CalculateBoolResultantType(
        const SymbolBase* left,
        const SymbolBase* right,
        int line,
        int column,
        ExceptionStatus type_mismatch_error);
    const SymbolBase* ResolveTypeNode(Node* type_node);
    const SymbolBase* WalkFunctionLikeTypecast(
        Node* spec_node,
        Node* name_node,
        Node* call_node,
        const SymbolBase* argument_result);
    void WalkSpecDeclarationPrescan(
        Node* spec_node,
        Node* access_modifier_node,
        Node* type_node);
};

const std::string kThisKeyword = "this";

} // namespace compiler
} // namespace gunderscript

#endif // GUNDERSCRIPT_SEMANTIC_CHECKER__H__
