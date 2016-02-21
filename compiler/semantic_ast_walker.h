// Gunderscript-2 Semantic (type) Checker for Abstract Syntax Tree
// (C) 2015-2016 Christian Gunderman

#ifndef GUNDERSCRIPT_SEMANTIC_CHECKER__H__
#define GUNDERSCRIPT_SEMANTIC_CHECKER__H__

#include <string>
#include <vector>

#include "gunderscript/lexer_resources.h"
#include "gunderscript/node.h"
#include "gunderscript/symbol.h"
#include "gunderscript/type.h"

#include "ast_walker.h"
#include "symbol_table.h"

namespace gunderscript {
namespace compiler {

// Type checking abstract syntax tree walker.
// Walks along the AST and checks for type correctness.
class SemanticAstWalker : public AstWalker<Type> {
public:

    SemanticAstWalker(Node& node);
    virtual ~SemanticAstWalker();

    const SymbolTable<Symbol*>& symbol_table() const { return symbol_table_; }

protected:
    void WalkModule(Node* module_node);
    void WalkModuleName(Node* name_node);
    void WalkModuleDependsName(Node* name_node);
    void WalkSpecDeclaration(
        Node* spec_node,
        Node* access_modifier_node,
        Node* name_node);
    void WalkSpecFunctionDeclaration(
        Node* spec_node,
        Node* function_node,
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
    Type WalkFunctionCall(
        Node* spec_node,
        Node* name_node,
        Node* call_none,
        std::vector<Type>& arguments_result);
    Type WalkAssign(
        Node* spec_node,
        Node* name_node,
        Node* symbol_node,
        Node* assign_node,
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
        Node* add_node,
        Node* left_node,
        Node* right_node,
        Type left_result,
        Type right_result);
    Type WalkSub(
        Node* spec_node,
        Node* sub_node,
        Node* left_node,
        Node* right_node,
        Type left_result,
        Type right_result);
    Type WalkMul(
        Node* spec_node,
        Node* mul_node,
        Node* left_node,
        Node* right_node,
        Type left_result,
        Type right_result);
    Type WalkDiv(
        Node* spec_node,
        Node* div_node,
        Node* left_node,
        Node* right_node,
        Type left_result,
        Type right_result);
    Type WalkMod(
        Node* spec_node,
        Node* mod_node,
        Node* left_node,
        Node* right_node,
        Type left_result,
        Type right_result);
    Type WalkLogAnd(
        Node* spec_node,
        Node* log_and_node,
        Node* left_node,
        Node* right_node,
        Type left_result,
        Type right_result);
    Type WalkLogNot(
        Node* spec_node,
        Node* log_not_node,
        Node* child_node,
        Type child_result);
    Type WalkLogOr(
        Node* spec_node,
        Node* log_or_node,
        Node* left_node,
        Node* right_node,
        Type left_result,
        Type right_result);
    Type WalkGreater(
        Node* spec_node,
        Node* greater_node,
        Node* left_node,
        Node* right_node,
        Type left_result,
        Type right_result);
    Type WalkEquals(
        Node* spec_node,
        Node* equals_node,
        Node* left_node,
        Node* right_node,
        Type left_result,
        Type right_result);
    Type WalkNotEquals(
        Node* spec_node,
        Node* not_equals_node,
        Node* left_node,
        Node* right_node,
        Type left_result,
        Type right_result);
    Type WalkLess(
        Node* spec_node,
        Node* less_node,
        Node* left_node,
        Node* right_node,
        Type left_result,
        Type right_result);
    Type WalkGreaterEquals(
        Node* spec_node,
        Node* greater_equals_node,
        Node* left_node,
        Node* right_node,
        Type left_result,
        Type right_result);
    Type WalkLessEquals(
        Node* spec_node,
        Node* less_equals_node,
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
        Node* atomic_node,
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
    Type WalkExpressionChildren(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        PropertyFunction property_function,
        Node* expression_node);
     
private:
    SymbolTable<Symbol*> symbol_table_;

    void CheckValidModuleName(const std::string& module_name, int line, int column);
    void CheckAccessModifier(
        const std::string& caller_class,
        const std::string& callee_class,
        LexerSymbol callee_access_modifier,
        int line,
        int column);
    Type CalculateResultantType(
        Type left,
        Type right,
        int line,
        int column,
        ExceptionStatus type_mismatch_error);
    Type CalculateNumericResultantType(
        Type left, 
        Type right,
        int line,
        int column,
        ExceptionStatus type_mismatch_error);
    Type CalculateBoolResultantType(
        Type left,
        Type right,
        int line,
        int column,
        ExceptionStatus type_mismatch_error);
    Type ResolveTypeNode(Node* type_node);
    Type WalkFunctionLikeTypecast(
        Node* spec_node,
        Node* name_node,
        Node* call_node,
        Type argument_result);
};

} // namespace compiler
} // namespace gunderscript

#endif // GUNDERSCRIPT_SEMANTIC_CHECKER__H__
