// Gunderscript-2 NanoJIT LIR Generator
// (C) 2016 Christian Gunderman

#ifndef GUNDERSCRIPT_LIRGEN_AST_WALKER__H__
#define GUNDERSCRIPT_LIRGEN_AST_WALKER__H__

#include <string>
#include <vector>

#include "gunderscript/node.h"
#include "gunderscript/symbol.h"
#include "gunderscript/virtual_machine.h"

#include "ast_walker.h"
#include "moduleimpl.h"
#include "symbol_table.h"

#include "nanojit.h"

using namespace nanojit;

namespace gunderscript {
namespace compiler {

union RegisterEntry {
    LIns* ins_;
    ModuleFunc* func_; // Pointer to a function pointer.

    RegisterEntry(LIns* ins) { ins_ = ins; }
    RegisterEntry(ModuleFunc* func) { func_ = func; }
};

// The result of a generation operation.
class LirGenResult {
public:
    LirGenResult(const SymbolBase* symbol, LIns* ins) : symbol_(symbol), ins_(ins) { }

    LIns* ins() { return ins_; }
    const SymbolBase* symbol() const { return symbol_; }

private:
    LIns* ins_;
    const SymbolBase* symbol_;
};

// LirGenResult checking abstract syntax tree walker.
// Walks along the AST and checks for type correctness.
class LIRGenAstWalker : public AstWalker<LirGenResult> {
public:

    LIRGenAstWalker(
        Allocator& alloc,
        Config& config,
        Node& node)
        : AstWalker(node),
        alloc_(alloc),
        config_(config),
        module_name_(NULL),
        current_fragment_(NULL) { }

    virtual ~LIRGenAstWalker() { }

    void Generate(Module& generated_module);

protected:
    void WalkModule(Node* module_node) { }
    void WalkModuleName(Node* name_node) { this->module_name_ = name_node->string_value(); }
    void WalkModuleDependsName(Node* name_node);
    void WalkSpecDeclaration(
        Node* spec_node,
        Node* access_modifier_node,
        Node* type_node,
        bool prescan) { }
    void WalkFunctionDeclaration(
        Node* spec_node,
        Node* function_node,
        Node* access_modifier_node,
        Node* type_node,
        Node* name_node,
        Node* block_node,
        std::vector<LirGenResult>& arguments_result,
        bool prescan) { }
    LirGenResult WalkSpecFunctionDeclarationParameter(
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
    LirGenResult WalkFunctionCall(
        Node* spec_node,
        Node* name_node,
        Node* call_node,
        std::vector<LirGenResult>& arguments_result);
    void WalkIfStatement(
        Node* spec_node,
        Node* if_node,
        LirGenResult condition_result) { }
    void WalkForStatement(
        Node* spec_node,
        Node* for_node,
        LirGenResult condition_result) { }
    LirGenResult WalkFunctionLikeTypecast(
        Node* spec_node,
        Node* name_node,
        Node* call_node,
        LirGenResult argument_result);
    LirGenResult WalkAssign(
        Node* spec_node,
        Node* name_node,
        Node* symbol_node,
        Node* assign_node,
        LirGenResult operations_result);
    LirGenResult WalkReturn(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        PropertyFunction property_function,
        LirGenResult* expression_result,
        std::vector<LirGenResult>* arguments_result);
    LirGenResult WalkAdd(
        Node* spec_node,
        Node* add_node,
        Node* left_node,
        Node* right_node,
        LirGenResult left_result,
        LirGenResult right_result);
    LirGenResult WalkSub(
        Node* spec_node,
        Node* sub_node,
        Node* left_node,
        Node* right_node,
        LirGenResult left_result,
        LirGenResult right_result);
    LirGenResult WalkMul(
        Node* spec_node,
        Node* mul_node,
        Node* left_node,
        Node* right_node,
        LirGenResult left_result,
        LirGenResult right_result);
    LirGenResult WalkDiv(
        Node* spec_node,
        Node* div_node,
        Node* left_node,
        Node* right_node,
        LirGenResult left_result,
        LirGenResult right_result);
    LirGenResult WalkMod(
        Node* spec_node,
        Node* mod_node,
        Node* left_node,
        Node* right_node,
        LirGenResult left_result,
        LirGenResult right_result);
    LirGenResult WalkLogAnd(
        Node* spec_node,
        Node* log_and_node,
        Node* left_node,
        Node* right_node,
        LirGenResult left_result,
        LirGenResult right_result);
    LirGenResult WalkLogNot(
        Node* spec_node,
        Node* log_not_node,
        Node* child_node,
        LirGenResult child_result);
    LirGenResult WalkLogOr(
        Node* spec_node,
        Node* log_or_node,
        Node* left_node,
        Node* right_node,
        LirGenResult left_result,
        LirGenResult right_result);
    LirGenResult WalkGreater(
        Node* spec_node,
        Node* greater_node,
        Node* left_node,
        Node* right_node,
        LirGenResult left_result,
        LirGenResult right_result);
    LirGenResult WalkEquals(
        Node* spec_node,
        Node* equals_node,
        Node* left_node,
        Node* right_node,
        LirGenResult left_result,
        LirGenResult right_result);
    LirGenResult WalkNotEquals(
        Node* spec_node,
        Node* not_equals_node,
        Node* left_node,
        Node* right_node,
        LirGenResult left_result,
        LirGenResult right_result);
    LirGenResult WalkLess(
        Node* spec_node,
        Node* less_node,
        Node* left_node,
        Node* right_node,
        LirGenResult left_result,
        LirGenResult right_result);
    LirGenResult WalkGreaterEquals(
        Node* spec_node,
        Node* greater_equals_node,
        Node* left_node,
        Node* right_node,
        LirGenResult left_result,
        LirGenResult right_result);
    LirGenResult WalkLessEquals(
        Node* spec_node,
        Node* less_equals_node,
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
        Node* atomic_node,
        Node* name_node);
    LirGenResult WalkAnyType(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        PropertyFunction property_function,
        Node* any_type_node);

    void WalkFunctionChildren(
        Node* spec_node,
        Node* function_node,
        bool prescan);
    void WalkIfStatementChildren(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        PropertyFunction property_function,
        Node* if_node,
        std::vector<LirGenResult>* arguments_result);
    void WalkForStatementChildren(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        PropertyFunction property_function,
        Node* for_node,
        std::vector<LirGenResult>* arguments_result);
    void WalkBlockChildren(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        PropertyFunction property_function,
        Node* block,
        std::vector<LirGenResult>* arguments_result);
    LirGenResult WalkNewExpression(
        Node* new_node,
        Node* type_node,
        std::vector<LirGenResult>& arguments_result);
    LirGenResult WalkDefaultExpression(
        Node* default_node,
        Node* type_node);

private:
    LIns* EmitLoad(const SymbolBase* symbol, nanojit::LIns* base, int offset);
    LIns* EmitStore(const SymbolBase* symbol, nanojit::LIns* base, int offset, LIns* value);
    int CountFunctions();

    ModuleFunc* func_table_;
    const std::string* module_name_;
    SymbolTable<std::tuple<const SymbolBase*, RegisterEntry>> register_table_;
    std::vector<ModuleImplSymbol>* symbols_vector_;
    nanojit::Allocator& alloc_;
    nanojit::Fragment* current_fragment_;
    nanojit::Config& config_;
    nanojit::LirBufWriter* current_writer_;
    int current_function_index_;
    int param_offset_;

    // Sanity check variables.
#ifdef _DEBUG
    int debug_functions_count_;
#endif // _DEBUG
};

} // namespace compiler
} // namespace gunderscript

#endif // GUNDERSCRIPT_LIRGEN_AST_WALKER__H__
