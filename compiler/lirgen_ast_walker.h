// Gunderscript-2 NanoJIT LIR Generator
// (C) 2016 Christian Gunderman

#ifndef GUNDERSCRIPT_LIRGEN_AST_WALKER__H__
#define GUNDERSCRIPT_LIRGEN_AST_WALKER__H__

#include <string>
#include <functional>
#include <vector>

#include "gunderscript/node.h"
#include "gunderscript/symbol.h"
#include "gunderscript/type.h"
#include "gunderscript/virtual_machine.h"

#include "ast_walker.h"
#include "moduleimpl.h"
#include "symbol_table.h"

#include "nanojit.h"

namespace gunderscript {
namespace compiler {

// Indicates whether we are not branching (true)
// branching to the false body (false)
// or end (end if)
// TODO: can we get rid of this and just null check ins()?
enum class BranchTarget {
    NONE_LABEL,
    FALSE_LABEL,
    END_LABEL
};

// The result of a generation operation.
class LirGenResult {
public:
    LirGenResult(const BranchTarget branch_target, const Type type, LIns* ins) 
        : branch_target_(branch_target), type_(type), ins_(ins) { }
    LirGenResult(const Type type, LIns* ins) : LirGenResult(BranchTarget::NONE_LABEL, type, ins) { }

    BranchTarget branch_target() { return branch_target_; }
    LIns* ins() { return ins_; }
    const Type& type() const { return type_; }

private:
    const BranchTarget branch_target_;
    const Type type_;
    LIns* ins_;
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
        std::vector<LirGenResult>& arguments_result,
        bool prescan);
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
    LirGenResult WalkFunctionLikeTypecast(
        Node* spec_node,
        Node* name_node,
        Node* call_node,
        LirGenResult argument_result);
    LirGenResult WalkAssign(
        Node* spec_node,
        Node* name_node,
        Node* assign_node,
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
    LirGenResult WalkExpressionChildren(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        PropertyFunction property_function,
        Node* expression_node);

private:
    LIns* GenerateLoad(const Type& type, nanojit::LIns* base);
    LIns* BackpatchJump(
        LirGenResult bool_jump_result,
        std::function<LIns* (LirBufWriter* writer)> true_func,
        std::function<LIns* (LirBufWriter* writer)> false_func);

    const std::string* module_name_;
    SymbolTable<nanojit::LIns*> register_table_;
    std::vector<ModuleImplSymbol>* symbols_vector_;
    std::vector<std::tuple<bool, LIns*> > backpatch_stack_;
    nanojit::Allocator& alloc_;
    nanojit::Fragment* current_fragment_;
    nanojit::Config& config_;
    nanojit::LirBufWriter* current_writer_;
};

} // namespace compiler
} // namespace gunderscript

#endif // GUNDERSCRIPT_LIRGEN_AST_WALKER__H__
