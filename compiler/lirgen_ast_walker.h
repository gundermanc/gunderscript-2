// Gunderscript-2 NanoJIT LIR Generator
// (C) 2016 Christian Gunderman

#ifndef GUNDERSCRIPT_LIRGEN_AST_WALKER__H__
#define GUNDERSCRIPT_LIRGEN_AST_WALKER__H__

#include <string>
#include <vector>

#include "gunderscript/module.h"
#include "gunderscript/node.h"
#include "gunderscript/virtual_machine.h"

#include "ast_walker.h"
#include "symbol.h"
#include "symbol_table.h"
#include "type.h"


namespace gunderscript {
namespace compiler {

// The result of a generation operation.
class LirGenResult {

};

// LirGenResult checking abstract syntax tree walker.
// Walks along the AST and checks for type correctness.
class LIRGenAstWalker : public AstWalker<LirGenResult> {
public:

    LIRGenAstWalker(Node& node)
        : AstWalker(node) { }

    Module Generate();

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
        std::vector<LirGenResult>& arguments_result,
        bool prescan);
    LirGenResult WalkSpecFunctionDeclarationParameter(
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
    LirGenResult WalkFunctionCall(
        Node* spec_node,
        Node* name_node,
        std::vector<LirGenResult>& arguments_result);
    LirGenResult WalkAssign(
        Node* spec_node,
        Node* name_node,
        LirGenResult operations_result);
    LirGenResult WalkReturn(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        PropertyFunction property_function,
        LirGenResult expression_result,
        std::vector<LirGenResult>* arguments_result);
    LirGenResult WalkAdd(
        Node* spec_node,
        Node* left_node,
        Node* right_node,
        LirGenResult left_result,
        LirGenResult right_result);
    LirGenResult WalkSub(
        Node* spec_node,
        Node* left_node,
        Node* right_node,
        LirGenResult left_result,
        LirGenResult right_result);
    LirGenResult WalkMul(
        Node* spec_node,
        Node* left_node,
        Node* right_node,
        LirGenResult left_result,
        LirGenResult right_result);
    LirGenResult WalkDiv(
        Node* spec_node,
        Node* left_node,
        Node* right_node,
        LirGenResult left_result,
        LirGenResult right_result);
    LirGenResult WalkMod(
        Node* spec_node,
        Node* left_node,
        Node* right_node,
        LirGenResult left_result,
        LirGenResult right_result);
    LirGenResult WalkLogAnd(
        Node* spec_node,
        Node* left_node,
        Node* right_node,
        LirGenResult left_result,
        LirGenResult right_result);
    LirGenResult WalkLogNot(
        Node* spec_node,
        Node* child_node,
        LirGenResult child_result);
    LirGenResult WalkLogOr(
        Node* spec_node,
        Node* left_node,
        Node* right_node,
        LirGenResult left_result,
        LirGenResult right_result);
    LirGenResult WalkGreater(
        Node* spec_node,
        Node* left_node,
        Node* right_node,
        LirGenResult left_result,
        LirGenResult right_result);
    LirGenResult WalkEquals(
        Node* spec_node,
        Node* left_node,
        Node* right_node,
        LirGenResult left_result,
        LirGenResult right_result);
    LirGenResult WalkNotEquals(
        Node* spec_node,
        Node* left_node,
        Node* right_node,
        LirGenResult left_result,
        LirGenResult right_result);
    LirGenResult WalkLess(
        Node* spec_node,
        Node* left_node,
        Node* right_node,
        LirGenResult left_result,
        LirGenResult right_result);
    LirGenResult WalkGreaterEquals(
        Node* spec_node,
        Node* left_node,
        Node* right_node,
        LirGenResult left_result,
        LirGenResult right_result);
    LirGenResult WalkLessEquals(
        Node* spec_node,
        Node* left_node,
        Node* right_node,
        LirGenResult left_result,
        LirGenResult right_result);
    LirGenResult WalkBool(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        PropertyFunction property_function,
        Node* bool_node);
    LirGenResult WalkInt(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        PropertyFunction property_function,
        Node* int_node);
    LirGenResult WalkFloat(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        PropertyFunction property_function,
        Node* float_node);
    LirGenResult WalkString(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        PropertyFunction property_function,
        Node* string_node);
    LirGenResult WalkChar(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        PropertyFunction property_function,
        Node* char_node);
    LirGenResult WalkVariable(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        PropertyFunction property_function,
        Node* name_node);
    LirGenResult WalkAnyType(
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
        std::vector<LirGenResult>* arguments_result);

private:
    const SymbolTable<Symbol> symbol_table_;
};

} // namespace compiler
} // namespace gunderscript

#endif // GUNDERSCRIPT_LIRGEN_AST_WALKER__H__
