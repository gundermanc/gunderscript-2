// Gunderscript-2 NanoJIT LIR Generator
// (C) 2016 Christian Gunderman

#include <cmath>

#include "gunderscript/exceptions.h"

#include "gs_assert.h"
#include "lirgen_ast_walker.h"
#include "parser.h"
#include "symbolimpl.h"

#include "garbage_collector.h"

using namespace nanojit;

namespace gunderscript {
namespace compiler {

// Static functions we can call into.
// TODO: when we separate compiler and runtime these may have to be moved.
// TODO: check that we don't have to manually specify the calling convention between compilers (MSVC vs. GCC).

static float FloatMod(float a1, float a2) {
    return fmod(a1, a2);
}

// Static function CallInfo structures.
// POTENTIAL BUG BUG BUG: be ABSOLUTELY certain that these match up or we're gonna have
// BAD things happen.

const CallInfo CI_FLOAT_MOD = {
    (uintptr_t)FloatMod,
    CallInfo::typeSig2(ARGTYPE_F, ARGTYPE_F, ARGTYPE_F),
    ABI_CDECL, ACCSET_STORE_ANY, false verbose_only(, "fmod")};

// Calling conventions for Gunderscript generated methods. One per machine level return type (int, float).
// These methods all take two arguments: A function pointer and a pointer to a vector of arguments in the stack
// and they return the function's return value. Admittedly this is weird but this is how arguments are passed in
// Tamarin as well.
// TODO: I suspect the 1 here is being mis-recognized as isPure=1 which could cause necessary function calls to be
// erronenously optimized out.

// Static methods:
const CallInfo CI_ICALLI = {
    CALL_INDIRECT,
    CallInfo::typeSig2(ARGTYPE_I, ARGTYPE_P, ARGTYPE_P),
    ABI_CDECL, 0, ACCSET_STORE_ANY verbose_only(, "CallIndirect_int32")};
const CallInfo CI_FCALLI = {
    CALL_INDIRECT,
    CallInfo::typeSig2(ARGTYPE_F, ARGTYPE_P, ARGTYPE_P),
    ABI_CDECL, 0, ACCSET_STORE_ANY verbose_only(, "CallIndirect_float32" )};
const CallInfo CI_PCALLI = {
    CALL_INDIRECT,
    CallInfo::typeSig2(ARGTYPE_P, ARGTYPE_P, ARGTYPE_P),
    ABI_CDECL, 0, ACCSET_STORE_ANY verbose_only(, "CallIndirect_pointer") };

// Member methods.
const CallInfo CI_M_ICALLI = {
    CALL_INDIRECT,
    CallInfo::typeSig3(ARGTYPE_I, ARGTYPE_P, ARGTYPE_P, ARGTYPE_P),
    ABI_CDECL, 0, ACCSET_STORE_ANY verbose_only(, "this.CallIndirect_int32") };
const CallInfo CI_M_FCALLI = {
    CALL_INDIRECT,
    CallInfo::typeSig3(ARGTYPE_F, ARGTYPE_P, ARGTYPE_P, ARGTYPE_P),
    ABI_CDECL, 0, ACCSET_STORE_ANY verbose_only(, "this.CallIndirect_float32") };
const CallInfo CI_M_PCALLI = {
    CALL_INDIRECT,
    CallInfo::typeSig3(ARGTYPE_P, ARGTYPE_P, ARGTYPE_P, ARGTYPE_P),
    ABI_CDECL, 0, ACCSET_STORE_ANY verbose_only(, "this.CallIndirect_pointer") };

// Generates IR code for the given module and stores it within the module.
// Throws: If this module has failed compilation once or is already compiled
// or a code generation step fails.
void LIRGenAstWalker::Generate(Module& module) {
    
    // Check if a module has already been compiled into this module file.
    if (module.compiled()) {
        THROW_EXCEPTION(1, 1, STATUS_INVALID_CALL);
    }

    // Mark as compiled. Even if compilation fails we invalidate this module for future compilations.
    module.pimpl()->set_compiled(true);

    // Set the output Module symbol vector as the current symbol vector.
    // This stores all exported symbols in the vector.
    this->symbols_vector_ = &module.pimpl()->symbols_vector();

    // Allocate function lookup table for the module.
    // Function look up table allows us to simplify compilation by assigning every yet-to-be compiled
    // function a place in memory in which its function pointer will be stored. At compilation rather
    // than jumping to the function's address, the function loads the pointer to the function from this
    // table. It knows where the function's POINTER is even if it doesn't know where the function is.
    int functions_count = this->CountFunctions();
    this->func_table_ = new ModuleFunc[functions_count];
    this->current_function_index_ = 0;

    // Debug sanity check prereq.
#ifdef _DEBUG
    this->debug_functions_count_ = functions_count;
#endif // _DEBUG

    // Walk through the AST and find everything we need for our new Module.
    this->Walk();

    GS_ASSERT_TRUE(this->register_table_.depth() == 1, "Inconsistent reg table levels");

    // Store the function lookup table in the module.
    module.pimpl()->set_func_table(this->func_table_);
}

// Handles depends statements.
void LIRGenAstWalker::WalkModuleDependsName(Node* name_node) {
    // TODO: implement support for depends statements.
    THROW_NOT_IMPLEMENTED();
}

// Walks a spec declaration and calculates it's compiled size.
void LIRGenAstWalker::WalkSpecDeclaration(
    Node* spec_node,
    Node* access_modifier_node,
    Node* type_node,
    bool prescan) {

    // Run only during the prescan stage.
    if (prescan) {
        int compiled_size = 0;

        Node* properties_node = spec_node->child(3);

        GS_ASSERT_NODE_RULE(properties_node, NodeRule::PROPERTIES);

        // Sum properties sizes.
        for (size_t i = 0; i < properties_node->child_count(); i++) {
            Node* property_type_node = properties_node->child(i)->child(0);
            
            GS_ASSERT_NODE_RULE(property_type_node, NodeRule::TYPE);

            compiled_size += property_type_node->symbol()->type_symbol()->size();
        }

        // NOTE: there is no need to catch exceptions from this table. If this spec
        // is a duplicate then the type checker messed up. It is supposed to ensure uniqueness.
        this->type_size_table_.insert(std::make_pair(spec_node->symbol()->symbol_name(), compiled_size));
    }
}

// Walks a single parameter in a spec function declaration and generates
// addressing code for it and stores it in the registers_table_.
LirGenResult LIRGenAstWalker::WalkSpecFunctionDeclarationParameter(
    Node* spec_node,
    Node* function_node,
    Node* type_node,
    Node* function_param_node,
    Node* name_node,
    bool prescan) {

    const TypeSymbol* param_type_symbol = function_param_node->symbol()->type_symbol();

    // Only run during actual code generation, not during the prescan for out of order functions.
    if (!prescan) {

        // Determine argument size.
        int arg_size = param_type_symbol->size();

        // Store the param load type, address, and offset in the register_table_
        this->register_table_.Put(
            function_param_node->symbol()->symbol_name(),
            std::make_tuple(
                param_type_symbol,
                this->current_writer_->insParam(/* args vector*/ 0, /*func param kind*/0),
                this->param_offset_));

        this->param_offset_ += arg_size;
    }

    return LirGenResult(param_type_symbol, NULL);
}

// Walks a single property in a spec property declaration and emits code.
void LIRGenAstWalker::WalkSpecPropertyDeclaration(
    Node* spec_node,
    Node* type_node,
    Node* name_node,
    Node* get_property_function_node,
    Node* set_property_function_node,
    Node* get_access_modifier_node,
    Node* set_access_modifier_node,
    bool prescan) {

    // Only run during the prescan for out of order properties.
    if (prescan) {

        const TypeSymbol* property_type_symbol = get_property_function_node->symbol()->type_symbol();

        // Determine argument size.
        int arg_size = property_type_symbol->size();

        // Store the property load address in the register_table_.
        this->register_table_.PutBottom(
            get_property_function_node->symbol()->symbol_name(),
            std::make_tuple(
                property_type_symbol,
                (LIns*)NULL,
                this->property_offset_));

        this->register_table_.PutBottom(
            set_property_function_node->symbol()->symbol_name(),
            std::make_tuple(
                property_type_symbol,
                (LIns*)NULL,
                this->property_offset_));

        this->property_offset_ += arg_size;
    }
}

LirGenResult LIRGenAstWalker::WalkFunctionCall(
    Node* spec_node,
    Node* name_node,
    Node* call_node,
    std::vector<LirGenResult>& arguments_result) {

    const SymbolBase* call_symbol = call_node->symbol();

    // Check if this is a function-like typecast.
    // If so generate typecast code.
    if (call_symbol->symbol_type() == SymbolType::TYPE) {
        return WalkFunctionLikeTypecast(
            spec_node,
            name_node,
            call_node, arguments_result.at(0));
    }

    return WalkFunctionCall(
        spec_node,
        name_node,
        call_node,
        arguments_result,
        NULL);
}

// Walks a function call and function-like typecasts and emits code.
LirGenResult LIRGenAstWalker::WalkFunctionCall(
    Node* spec_node,
    Node* name_node,
    Node* call_node,
    std::vector<LirGenResult>& arguments_result,
    LirGenResult* obj_ref_result) {

    return WalkFunctionCall(
        call_node->symbol(),
        arguments_result,
        obj_ref_result);
}

// Walks a function call and function-like typecasts and emits code.
LirGenResult LIRGenAstWalker::WalkFunctionCall(
    const SymbolBase* call_symbol,
    std::vector<LirGenResult>& arguments_result,
    LirGenResult* obj_ref_result) {

    // Lookup the function's register entry.
    // Register entry contains register information for variables and 
    // a pointer to a location that will contain a function pointer after compilation
    // for functions. This is done to avoid the need to backpatch function addresses later.
    std::tuple<const SymbolBase*, LIns*, int> function_reg_tuple
        = this->register_table_.Get(call_symbol->symbol_name());

    // NanoJIT unfortunately depends on having a function pointer to jump to for function calls,
    // however, we don't have one readily available for most functions since compilation hasn't
    // completed. We get around this limitation by keeping a table of function pointers
    // in this->func_table_ (and later in moduleimpl). After compilation this table is populated
    // with pointers to each of our functions. Since we know WHERE the pointers are we can simply
    // write instructions to load them and call them. This also has the added benefit of making
    // it easy to perform partial recompiles by simply recompiling the modified function and
    // updating its pointer.
    LIns* target = this->current_writer_->insLoad(
        LIR_ldp,
        this->current_writer_->insImmP(std::get<1>(function_reg_tuple)),
        0,
        ACCSET_ALL,
        LOAD_NORMAL);

    // Choose indirect call method. Arguments are passed as a single argument in a vector so
    // only one call method is needed per return type.
    const CallInfo* method = NULL;
    const TypeSymbol* return_type_symbol = SYMBOL_TO_FUNCTION(call_symbol)->type_symbol();

    switch (return_type_symbol->type_format())
    {
    case TypeFormat::POINTER:
        method = obj_ref_result == NULL ? &CI_PCALLI : &CI_M_PCALLI;
        break;
    case TypeFormat::FVOID:
    case TypeFormat::BOOL:
    case TypeFormat::INT:
        method = obj_ref_result == NULL ? &CI_ICALLI : &CI_M_ICALLI;
        break;
    case TypeFormat::FLOAT:
        method = obj_ref_result == NULL ? &CI_FCALLI : &CI_M_FCALLI;
        break;
    default:
        GS_ASSERT_FAIL("Unimplemented return type in function call");
    }
    
    // Create a stack alloc instruction for a vector of arguments. This is backpatched later.
    LIns* arguments_vector = arguments_vector = this->current_writer_->insAlloc(sizeof(uint8_t));

    if (arguments_result.size() > 0) {
        int32_t arg_offset = 0;

        // Store arguments in the stack.
        for (size_t i = 0; i < arguments_result.size(); i++) {
            LirGenResult& argument = arguments_result.at(i);

            EmitStore(argument.symbol(), arguments_vector, arg_offset, argument.ins());
            arg_offset += argument.symbol()->type_symbol()->size();
        }

        // Backpatch the size of the arguments vector to the sum of the sizes of the
        // respective args.
        arguments_vector->setSize(arg_offset);
    }

    // We're using an indirect function call here meaning that we are calling a function pointer that
    // we loaded from somewhere (in this case the func_table, see comment above). Due to quirks of
    // NanoJIT we only need two arguments to the call instruction:
    // - A pointer to the first argument. Subsequent function arguments are found at various offsets
    //   along this pointer.
    // - A pointer to the target function.
    // This is how calls are made in Tamarin as well, presumably to allow virtual functions but they
    // do the same thing for static methods as well making me think that NanoJIT simply requires
    // generated functions to pass arguments in this way.
    // NanoJIT appears to require the function pointer to be the last argument but this is merely
    // speculation as it is undocumented and the code is difficult to follow. If tests fail, try
    // making it first.
    LIns* call;
    if (obj_ref_result == NULL) {
        LIns* args[] = { arguments_vector, target };
        call = this->current_writer_->insCall(method, args);
    }
    else {
        LIns* args[] = { obj_ref_result->ins(), arguments_vector, target };
        call = this->current_writer_->insCall(method, args);
    }

    // Ensure the arguments remain live until after the call.
    this->current_writer_->ins1(LIR_livep, arguments_vector);

    return LirGenResult(return_type_symbol, call);
}

// Walks a member expression.
// e.g.: this.x()
LirGenResult LIRGenAstWalker::WalkMemberFunctionCall (
    Node* spec_node,
    Node* member_node,
    LirGenResult left_result,
    Node* right_node,
    std::vector<LirGenResult>& arguments_result) {

    GS_ASSERT_TRUE(right_node->rule() == NodeRule::CALL,
        "Unimplemented or improper member right node rule");

    Node* name_node = right_node->child(0);

    GS_ASSERT_NODE_RULE(name_node, NodeRule::NAME);

    // Walking a member function works pretty much the same as a non-member function
    // at this stage. Thanks to the typechecker the right_node (call_node) has been
    // annotated with the proper function call symbol and can simply be fed
    // to the normal code generator.
    return WalkFunctionCall(
        spec_node,
        name_node,
        right_node,
        arguments_result,
        &left_result);
}

LirGenResult LIRGenAstWalker::WalkMemberPropertyGet(
    Node* spec_node,
    Node* member_node,
    LirGenResult left_result,
    Node* right_node) {

    const SymbolBase* member_symbol = member_node->symbol();

    // Lookup property entry.
    const std::tuple<const SymbolBase*, LIns*, int>& reg_tuple
        = this->register_table_.Get(member_symbol->symbol_name());

    // Emit load instruction to load the value from a spec instance property.
    // The base pointer is the pointer from the left side of the member expression
    // (left.right) and the offset is the stored offset associated with this property
    // in the register table.
    return LirGenResult(
        member_symbol,
        EmitLoad(member_symbol, left_result.ins(), std::get<2>(reg_tuple)));
}

LirGenResult LIRGenAstWalker::WalkMemberPropertySet(
    Node* spec_node,
    Node* member_node,
    LirGenResult left_result,
    Node* right_node,
    LirGenResult value_result) {

    const SymbolBase* member_symbol = member_node->symbol();

    // Lookup property entry.
    const std::tuple<const SymbolBase*, LIns*, int>& reg_tuple
        = this->register_table_.Get(member_symbol->symbol_name());

    // Emit load instruction to load the value from a spec instance property.
    // The base pointer is the pointer from the left side of the member expression
    // (left.right) and the offset is the stored offset associated with this property
    // in the register table.
    return LirGenResult(
        member_symbol,
        EmitStore(member_symbol, left_result.ins(), std::get<2>(reg_tuple), value_result.ins()));
}

// Walks a function-like typecast and generates code for the type conversions.
LirGenResult LIRGenAstWalker::WalkFunctionLikeTypecast(
    Node* spec_node,
    Node* name_node,
    Node* call_node,
    LirGenResult argument_result) {

    const TypeSymbol* cast_symbol = call_node->symbol()->type_symbol();
    const TypeSymbol* expression_symbol = argument_result.symbol()->type_symbol();

    // If types are the same, no conversion neccessary.
    if (*expression_symbol == *cast_symbol) {
        return argument_result;
    }

    switch (cast_symbol->type_format()) {
    case TypeFormat::INT:
        if (expression_symbol->type_format() == TypeFormat::FLOAT) {
            return LirGenResult(cast_symbol,
                this->current_writer_->ins1(LIR_f2i, argument_result.ins()));
        }
        else if (expression_symbol->type_format() == TypeFormat::INT ||
            expression_symbol->type_format() == TypeFormat::BOOL) {

            // POTENTIAL BUG BUG BUG:
            // Although INT8/CHAR type is only 1 byte in size whereas INT32/INT is 32 bits
            // the standard register is 32 bits so we treat INT8 as an integer in many cases
            // including primitive arithmetic operations, return from functions, etc.
            // The exceptions are Load (var reference) and Store (var assign) for the benefit of
            // reduced overall memory consumption in the stack and dynamically alloc-ed memory.
            // Assuming that NanoJIT generates code in some semblance of a logical way any
            // typecasts should occur after a value has been loaded into a register and is once again
            // 32 bits. If this fails though you'll know it's because the value being typecasted is
            // not in a register (if this is even possible) or is greater than 4 bytes in size.
            return LirGenResult(cast_symbol, argument_result.ins());
        }
        break;

    case TypeFormat::FLOAT:
        if (expression_symbol->type_format() == TypeFormat::INT ||
            expression_symbol->type_format() == TypeFormat::BOOL) {
            return LirGenResult(cast_symbol,
                this->current_writer_->ins1(LIR_i2f, argument_result.ins()));
        }
        break;

    case TypeFormat::BOOL:
        if (expression_symbol->type_format() == TypeFormat::INT) {
            // The process for typecasting from INT to BOOL is more complex than BOOL to int
            // because going from BOOL to int we already know that BOOL is in {0, 1}
            // but going from INT to BOOL the INT can be anything so we compile this typecast
            // as CMOV (the equivalent of the ternary: x == 0 ? 1 : 0) so that INTs != 1 will
            // properly become true regardless.
            // 0 is false, all else are true.
            LIns* eq_ins = this->current_writer_->ins2(
                LIR_eqi,
                argument_result.ins(),
                this->current_writer_->insImmI(0));

            // Compiles as a CMOV instruction: x == 0 ? 1 : 0
            // where 1 = true and 0 = false.
            LIns* choose_ins = this->current_writer_->insChoose(
                eq_ins,
                this->current_writer_->insImmI(0),
                this->current_writer_->insImmI(1),
                true);

            return LirGenResult(cast_symbol, choose_ins);
        }
        break;
    }

    // Not implemented.
    THROW_NOT_IMPLEMENTED();
}

// Walks an assignment statement and generates code for it.
LirGenResult LIRGenAstWalker::WalkAssign(
    Node* spec_node,
    Node* name_node,
    Node* symbol_node,
    Node* assign_node,
    LirGenResult operations_result) {

    LIns* variable_ptr = NULL;

    // Check if the variable name was bound in the current scope. If it was, it is a local
    // and we can use the NanoJIT register for it.
    try {
        variable_ptr = std::get<1>(this->register_table_.GetTopOnly(*name_node->string_value()));;
        goto emit_assign_ins;
    }
    catch (const Exception&) {

        // Variable was not bound in the current scope, check if it was bound in any scope.
        // If so, if it is the same type as our expression then we are assigning to the already
        // declared variable.
        // If not, this variable is a narrower scope variable of the same name and a different
        // type that is masking the old one.
        try {
            std::tuple<const SymbolBase*, LIns*, int> variable_reg = this->register_table_.Get(*name_node->string_value());

            if (*std::get<0>(variable_reg) == *operations_result.symbol()) {
                variable_ptr = std::get<1>(variable_reg);
                goto emit_assign_ins;
            }
            /* else: Fall into new alloc */
        }
        catch (const Exception&) { /* Fall into new alloc */ }
    }

    // New Alloc: Allocate a new register (NanoJIT register) for this variable name,
    // it has either never been seen before or is a new var of a different type masking
    // the original definition in an enclosing scope.
    variable_ptr = this->current_writer_->insAlloc(operations_result.symbol()->type_symbol()->size());
    this->register_table_.Put(*name_node->string_value(), std::make_tuple(operations_result.symbol(), variable_ptr, 0));

    // Emit the assignment instruction. Dijkstra doesn't have to agree with me, gotos can be useful.
emit_assign_ins:
    EmitStore(operations_result.symbol()->type_symbol(), variable_ptr, 0, operations_result.ins());

    // The value and type of an assignment is equal to that of the right hand
    // side of the operation (the expression). Simply pass it along.
    return operations_result;
}

// Walks a return statement and generates code for it.
// TODO: make this work for properties.
LirGenResult LIRGenAstWalker::WalkReturn(
    Node* spec_node,
    Node* function_node,
    Node* property_node,
    PropertyFunction property_function,
    LirGenResult* expression_result,
    std::vector<LirGenResult>* arguments_result) {

    // No return expression given. Void function or constructor.
    if (expression_result == NULL) {
        return LirGenResult(&TYPE_VOID,
            this->current_writer_->ins1(LIR_reti, this->current_writer_->insImmI(0)));
    }

    const TypeSymbol* expression_result_symbol = expression_result->symbol()->type_symbol();

    switch (expression_result_symbol->type_format())
    {
    case TypeFormat::BOOL:
    case TypeFormat::INT:
        switch (expression_result_symbol->size())
        {
        case 4:
        case 1:
            // Return of 1 byte INT8/CHAR values is still LIR_reti because the actual
            // operation occurs in a standard >= 32 bit register on many systems.
            return LirGenResult((*expression_result).symbol(), 
                this->current_writer_->ins1(LIR_reti, expression_result->ins()));
        }
        break;
    case TypeFormat::FLOAT:
        switch (expression_result_symbol->size())
        {
        case 4:
            // Return of 1 byte INT8/CHAR values is still LIR_reti because the actual
            // operation occurs in a standard >= 32 bit register on many systems.
            return LirGenResult((*expression_result).symbol(),
                this->current_writer_->ins1(LIR_retf, expression_result->ins()));
        }
        break;
    case TypeFormat::POINTER:
        return LirGenResult((*expression_result).symbol(),
            this->current_writer_->ins1(LIR_retp, expression_result->ins()));
    }

    // Unknown return type.
    THROW_NOT_IMPLEMENTED();
}

// Walks the ADD node and generates code for it and its children.
LirGenResult LIRGenAstWalker::WalkAdd(
    Node* spec_node,
    Node* add_node,
    Node* left_node,
    Node* right_node,
    LirGenResult left_result,
    LirGenResult right_result) {

    const TypeSymbol* left_symbol = left_node->symbol()->type_symbol();

    // Ignore right_result types, we already went through the semantic_ast_walker.
    // Types are known.
    switch (left_symbol->type_format())
    {
    case TypeFormat::INT:
        switch (left_symbol->size())
        {
        case 4:
        case 1:
            // Addition of 1 INT8/CHAR values is still LIR_addi because the actual
            // operation occurs in a standard >= 32 bit register.
            return LirGenResult(
                left_result.symbol(),
                this->current_writer_->ins2(LIR_addi, left_result.ins(), right_result.ins()));
        }
        break;
    case TypeFormat::FLOAT:
        switch (left_symbol->size())
        {
        case 4:
            return LirGenResult(
                left_result.symbol(),
                this->current_writer_->ins2(LIR_addf, left_result.ins(), right_result.ins()));
        }
        break;
    }
    
    // Unhandled type format.
    THROW_NOT_IMPLEMENTED();
}

// Walks the SUB node and generates code for it.
LirGenResult LIRGenAstWalker::WalkSub(
    Node* spec_node,
    Node* sub_node,
    Node* left_node,
    Node* right_node,
    LirGenResult left_result,
    LirGenResult right_result) {

    const TypeSymbol* right_symbol = right_node->symbol()->type_symbol();
    LIns* left_ins = left_result.ins();

    // HACK: For the WalkSub case in particular we MUST use ONLY the RIGHT node's type because
    // the SemanticAstWalker returns ANY_TYPE for the left operand to get around the explict
    // typecast requirement for all operands with negative numbers.
    // TODO: see Github Issue #101 for fix for this.
    switch (right_symbol->type_format())
    {
    case TypeFormat::INT:
        switch (right_symbol->size())
        {
        case 4:
        case 1:
            // HACK: negative numbers special case.
            if (left_result.ins() == NULL) {
                left_ins = this->current_writer_->insImmI(0);
            }

            // Subtraction of 1 byte INT8/CHAR values is still LIR_subi because the actual
            // operation occurs in a standard >= 32 bit register.
            return LirGenResult(
                right_result.symbol(),
                this->current_writer_->ins2(LIR_subi, left_ins, right_result.ins()));
        }
        break;
    case TypeFormat::FLOAT:
        switch (right_symbol->size())
        {
        case 4:
            // HACK: negative numbers special case.
            if (left_result.ins() == NULL) {
                left_ins = this->current_writer_->insImmF(0.0);
            }

            return LirGenResult(
                right_result.symbol(),
                this->current_writer_->ins2(LIR_subf, left_ins, right_result.ins()));
        }
        break;
    }

    // Unhandled type format.
    THROW_NOT_IMPLEMENTED();
}

// Walks the MUL node and generates code for it and its children.
LirGenResult LIRGenAstWalker::WalkMul(
    Node* spec_node,
    Node* mul_node,
    Node* left_node,
    Node* right_node,
    LirGenResult left_result,
    LirGenResult right_result) {

    const TypeSymbol* left_symbol = left_node->symbol()->type_symbol();

    // Ignore right_result types, we already went through the semantic_ast_walker.
    // Types are known.
    switch (left_symbol->type_format())
    {
    case TypeFormat::INT:
        switch (left_symbol->size())
        {
        case 4:
        case 1:
            // Multiplication of 1 INT8/CHAR value is still LIR_muli because the actual
            // operation occurs in a standard >= 32 bit register.
            return LirGenResult(
                left_result.symbol(),
                this->current_writer_->ins2(LIR_muli, left_result.ins(), right_result.ins()));
        }
        break;
    case TypeFormat::FLOAT:
        switch (left_symbol->size())
        {
        case 4:
            return LirGenResult(
                left_result.symbol(),
                this->current_writer_->ins2(LIR_mulf, left_result.ins(), right_result.ins()));
        }
        break;
    }

    // Unhandled type format.
    THROW_NOT_IMPLEMENTED();
}

// Walks the DIV node and generates code for it and its children.
LirGenResult LIRGenAstWalker::WalkDiv(
    Node* spec_node,
    Node* div_node,
    Node* left_node,
    Node* right_node,
    LirGenResult left_result,
    LirGenResult right_result) {

    const TypeSymbol* left_symbol = left_node->symbol()->type_symbol();

    // Ignore right_result types, we already went through the semantic_ast_walker.
    // Types are known.
    switch (left_symbol->type_format())
    {
    case TypeFormat::INT:
        switch (left_symbol->size())
        {
        case 4:
        case 1:
            // Division of 1 INT8/CHAR values is still LIR_divi because the actual
            // operation occurs in a standard >= 32 bit register.
            return LirGenResult(
                left_result.symbol(),
                this->current_writer_->ins2(LIR_divi, left_result.ins(), right_result.ins()));
        }
        break;
    case TypeFormat::FLOAT:
        switch (left_symbol->size())
        {
        case 4:
            return LirGenResult(
                left_result.symbol(),
                this->current_writer_->ins2(LIR_divf, left_result.ins(), right_result.ins()));
        }
        break;

    }
    
    // Unhandled type format.
    THROW_NOT_IMPLEMENTED();
}

// Walks the MOD node and generates code for it and its children.
LirGenResult LIRGenAstWalker::WalkMod(
    Node* spec_node,
    Node* mul_node,
    Node* left_node,
    Node* right_node,
    LirGenResult left_result,
    LirGenResult right_result) {

    const TypeSymbol* left_symbol = left_node->symbol()->type_symbol();

    // Ignore right_result types, we already went through the semantic_ast_walker.
    // Types are known.
    switch (left_symbol->type_format())
    {
    case TypeFormat::INT:
        switch (left_symbol->size())
        {
        case 4:
        case 1:
            // Modulo of 1 INT8/CHAR values is still LIR_modi because the actual
            // operation occurs in a standard >= 32 bit register.
#if defined NANOJIT_IA32 || defined NANOJIT_X64
            return LirGenResult(
                left_result.symbol(),
                this->current_writer_->ins1(LIR_modi,
                    this->current_writer_->ins2(LIR_divi, left_result.ins(), right_result.ins())));
#else
            // LIR_modi instruction is only supported on x86 and x86_64 platforms.
            // TODO: If porting to ARM, SPARC, etc, be sure to fix this. We can probably use the
            // same strategy as below with the FloatMod function but I have no way to test it
            // so I'm gonna hold off until there is an official ARM port.
            THROW_NOT_IMPLEMENTED();
#endif
        }
        break;
    case TypeFormat::FLOAT:
        switch (left_symbol->size())
        {
        case 4:
        {
            // x86/x86_64 has no native support for floating point operations on floats so
            // we wrap and call the cmath fmod function.
            // Since we are pushing the args onto the stack apparently they have to be
            // in reverse order.
            LIns* float_args[3] = { right_result.ins(), left_result.ins(), NULL };
            return LirGenResult(
                left_result.symbol(),
                this->current_writer_->insCall(&CI_FLOAT_MOD, float_args));
        }
        }
        break;
    }

    // Unhandled type format.
    THROW_NOT_IMPLEMENTED();
}

// Walks the LOGNOT node and generates code for it.
LirGenResult LIRGenAstWalker::WalkLogNot(
    Node* spec_node,
    Node* log_not_node,
    Node* child_node,
    LirGenResult child_result) {

    return LirGenResult(
        &TYPE_BOOL,
        this->current_writer_->ins2ImmI(
            LIR_xori,
            child_result.ins(),
            1));
}

// Walks the LOGAND node and generates code for it.
LirGenResult LIRGenAstWalker::WalkLogAnd(
    Node* spec_node,
    Node* log_and_node,
    Node* left_node,
    Node* right_node,
    LirGenResult left_result,
    LirGenResult right_result) {

    // TODO: short circuit.
    return LirGenResult(
        &TYPE_BOOL,
        this->current_writer_->ins2(
            LIR_andi,
            left_result.ins(),
            right_result.ins()));
}

// Walks the LOGOR node and generates code for it and its children.
LirGenResult LIRGenAstWalker::WalkLogOr(
    Node* spec_node,
    Node* log_or_node,
    Node* left_node,
    Node* right_node,
    LirGenResult left_result,
    LirGenResult right_result) {

    // TODO: short circuit.
    return LirGenResult(
        &TYPE_BOOL,
        this->current_writer_->ins2(
            LIR_ori,
            left_result.ins(),
            right_result.ins()));
}

// Walks the GREATER node and generates code for it.
LirGenResult LIRGenAstWalker::WalkGreater(
    Node* spec_node,
    Node* greater_node,
    Node* left_node,
    Node* right_node,
    LirGenResult left_result,
    LirGenResult right_result) {

    const TypeSymbol* left_symbol = left_node->symbol()->type_symbol();

    // Left and right are already the same type thanks to the typechecker.
    switch (left_symbol->type_format()) {
    case TypeFormat::INT:
        if (left_symbol->size() == 4 ||
            left_symbol->size() == 1) {
            return LirGenResult(
                &TYPE_BOOL,
                this->current_writer_->ins2(
                    LIR_gti,
                    left_result.ins(),
                    right_result.ins()));
        }
        break;
    case TypeFormat::FLOAT:
        if (left_symbol->size() == 4) {
            return LirGenResult(
                &TYPE_BOOL,
                this->current_writer_->ins2(
                    LIR_gtf,
                    left_result.ins(),
                    right_result.ins()));
        }
        break;
    }

    THROW_NOT_IMPLEMENTED();
}

// Walks the EQUALS node and generates code for it.
LirGenResult LIRGenAstWalker::WalkEquals(
    Node* spec_node,
    Node* equals_node,
    Node* left_node,
    Node* right_node,
    LirGenResult left_result,
    LirGenResult right_result) {

    const TypeSymbol* left_symbol = left_node->symbol()->type_symbol();

    // Left and right are already the same type thanks to the typechecker.
    switch (left_symbol->type_format()) {
    case TypeFormat::POINTER:
        return LirGenResult(
            &TYPE_BOOL,
            this->current_writer_->ins2(
                LIR_eqp,
                left_result.ins(),
                right_result.ins()));
    case TypeFormat::INT:
        if (left_symbol->size() == 4 ||
            left_symbol->size() == 1) {
            return LirGenResult(
                &TYPE_BOOL,
                this->current_writer_->ins2(
                    LIR_eqi,
                    left_result.ins(),
                    right_result.ins()));
        }
        break;
    case TypeFormat::FLOAT:
        if (left_symbol->size() == 4) {
            return LirGenResult(
                &TYPE_BOOL,
                this->current_writer_->ins2(
                    LIR_eqf,
                    left_result.ins(),
                    right_result.ins()));
        }
        break;
    }

    // Unhandled type.
    THROW_NOT_IMPLEMENTED();
}

// Walks the NOT_EQUALS node and generates code for it.
LirGenResult LIRGenAstWalker::WalkNotEquals(
    Node* spec_node,
    Node* not_equals_node,
    Node* left_node,
    Node* right_node,
    LirGenResult left_result,
    LirGenResult right_result) {

    const TypeSymbol* left_symbol = left_node->symbol()->type_symbol();

    // Left and right are already the same type thanks to the typechecker.
    switch (left_symbol->type_format()) {
    case TypeFormat::POINTER:
        return LirGenResult(
            &TYPE_BOOL,
            this->current_writer_->ins2(
                LIR_xori,
                this->current_writer_->insImmI(1),
                this->current_writer_->ins2(
                    LIR_eqp,
                    left_result.ins(),
                    right_result.ins())));
    case TypeFormat::INT:
        if (left_symbol->size() == 4 ||
            left_symbol->size() == 1) {

            // TODO: can we do this with one instruction??
            // XORing 1 or 0 by 1 flips.
            return LirGenResult(
                &TYPE_BOOL,
                this->current_writer_->ins2(
                    LIR_xori,
                    this->current_writer_->insImmI(1),
                    this->current_writer_->ins2(
                        LIR_eqi,
                        left_result.ins(),
                        right_result.ins())));
        }
        break;
    case TypeFormat::FLOAT:
        if (left_symbol->size() == 4) {

            // TODO: can we do this with one instruction??
            // XORing 1 or 0 by 1 flips.
            return LirGenResult(
                &TYPE_BOOL,
                this->current_writer_->ins2(
                    LIR_xori,
                    this->current_writer_->insImmI(1),
                    this->current_writer_->ins2(
                        LIR_eqf,
                        left_result.ins(),
                        right_result.ins())));
        }
        break;
    }

    // Unhandled type.
    THROW_NOT_IMPLEMENTED();
}

// Walks the LESS node and generates code for it.
LirGenResult LIRGenAstWalker::WalkLess(
    Node* spec_node,
    Node* less_node,
    Node* left_node,
    Node* right_node,
    LirGenResult left_result,
    LirGenResult right_result) {

    const TypeSymbol* left_symbol = left_node->symbol()->type_symbol();

    // Left and right are already the same type thanks to the typechecker.
    switch (left_symbol->type_format()) {
    case TypeFormat::INT:
        if (left_symbol->size() == 4 ||
            left_symbol->size() == 1) {
            return LirGenResult(
                &TYPE_BOOL,
                this->current_writer_->ins2(
                    LIR_lti,
                    left_result.ins(),
                    right_result.ins()));
        }
        break;
    case TypeFormat::FLOAT:
        if (left_symbol->size() == 4) {
            return LirGenResult(
                &TYPE_BOOL,
                this->current_writer_->ins2(
                    LIR_ltf,
                    left_result.ins(),
                    right_result.ins()));
        }
        break;
    }

    // Unhandled type.
    THROW_NOT_IMPLEMENTED();
}

// Walks the GREATER_EQUALS node and generates code for it.
LirGenResult LIRGenAstWalker::WalkGreaterEquals(
    Node* spec_node,
    Node* greater_equals_node,
    Node* left_node,
    Node* right_node,
    LirGenResult left_result,
    LirGenResult right_result) {

    const TypeSymbol* left_symbol = left_node->symbol()->type_symbol();

    // Left and right are already the same type thanks to the typechecker.
    switch (left_symbol->type_format()) {
    case TypeFormat::INT:
        if (left_symbol->size() == 4 ||
            left_symbol->size() == 1) {
            return LirGenResult(
                &TYPE_BOOL,
                this->current_writer_->ins2(
                    LIR_gei,
                    left_result.ins(),
                    right_result.ins()));
        }
        break;
    case TypeFormat::FLOAT:
        if (left_symbol->size() == 4) {
            return LirGenResult(
                &TYPE_BOOL,
                this->current_writer_->ins2(
                    LIR_gef,
                    left_result.ins(),
                    right_result.ins()));
        }
        break;
    }

    // Unhandled type.
    THROW_NOT_IMPLEMENTED();
}

// Walks the LESS_EQUALS node and generates code for it.
LirGenResult LIRGenAstWalker::WalkLessEquals(
    Node* spec_node,
    Node* less_equals,
    Node* left_node,
    Node* right_node,
    LirGenResult left_result,
    LirGenResult right_result) {

    const TypeSymbol* left_symbol = left_node->symbol()->type_symbol();

    // Left and right are already the same type thanks to the typechecker.
    switch (left_symbol->type_format()) {
    case TypeFormat::INT:
        if (left_symbol->size() == 4 ||
            left_symbol->size() == 1) {
            return LirGenResult(
                &TYPE_BOOL,
                this->current_writer_->ins2(
                    LIR_lei,
                    left_result.ins(),
                    right_result.ins()));
        }
        break;
    case TypeFormat::FLOAT:
        if (left_symbol->size() == 4) {
            return LirGenResult(
                &TYPE_BOOL,
                this->current_writer_->ins2(
                    LIR_lef,
                    left_result.ins(),
                    right_result.ins()));
        }
        break;
    }

    // Unhandled type.
    THROW_NOT_IMPLEMENTED();
}

// Walks the TYPE_BOOL node and generates code for it.
LirGenResult LIRGenAstWalker::WalkBool(
    Node* spec_node,
    Node* function_node,
    Node* property_node,
    PropertyFunction property_function,
    Node* bool_node) {

    // In C++ bool values are usually integers of the value 0 -> false or 1 -> true
    // but we'll use a ternary just in case this is violated for some reason.
    return LirGenResult(bool_node->symbol(),
        this->current_writer_->insImmI(
            bool_node->bool_value() ? 1 : 0));
}

// Walks the TYPE_INT node and generates code for it.
LirGenResult LIRGenAstWalker::WalkInt(
    Node* spec_node,
    Node* function_node,
    Node* property_node,
    PropertyFunction property_function,
    Node* int_node) {

    return LirGenResult(int_node->symbol(),
        this->current_writer_->insImmI((int32_t)int_node->int_value()));
}

// Walks the FLOAT node and generates code for it.
LirGenResult LIRGenAstWalker::WalkFloat(
    Node* spec_node,
    Node* function_node,
    Node* property_node,
    PropertyFunction property_function,
    Node* float_node) {

    return LirGenResult(float_node->symbol(),
        this->current_writer_->insImmF((float)float_node->float_value()));
}

// Walks the STRING node and generates code for it.
LirGenResult LIRGenAstWalker::WalkString(
    Node* spec_node,
    Node* function_node,
    Node* property_node,
    PropertyFunction property_function,
    Node* string_node) {

    THROW_EXCEPTION(1, 1, STATUS_ILLEGAL_STATE);
}

// Walks the CHAR node and generates code for it.
LirGenResult LIRGenAstWalker::WalkChar(
    Node* spec_node,
    Node* function_node,
    Node* property_node,
    PropertyFunction property_function,
    Node* char_node) {

    // Although INT8/CHAR type is only 1 byte, most registers are >= 32 bits
    // so the IMM is still of type int.
    return LirGenResult(char_node->symbol(),
        this->current_writer_->insImmI((int32_t)char_node->int_value()));
}

// Walks the SYMBOL->NAME subtree that represents a variable reference
// and generates code for it.
LirGenResult LIRGenAstWalker::WalkVariable(
    Node* spec_node,
    Node* function_node,
    Node* property_node,
    PropertyFunction property_function,
    Node* variable_node,
    Node* name_node) {

    const SymbolBase* variable_type = variable_node->symbol();

    if (*name_node->string_value() == "this") {
        return LirGenResult(
            variable_type,
            this->current_writer_->insParam(/* _this_ pointer */ 1, /*func param kind*/0));
    }

    const std::tuple<const SymbolBase*, LIns*, int>& reg_tuple
        = this->register_table_.Get(*name_node->string_value());

    // No try/catch here, if register_table_ throws then the type checker made
    // a boo boo and didn't notice the missing variable initialization.
    LIns* load = EmitLoad(
        std::get<0>(reg_tuple),
        std::get<1>(reg_tuple),
        std::get<2>(reg_tuple));

    return LirGenResult(variable_type, load);
}

// Walks the ANY_TYPE node and returns a NULL.
// Walkers that use NONE/ANY must special case this.
LirGenResult LIRGenAstWalker::WalkAnyType(
    Node* spec_node,
    Node* function_node,
    Node* property_node,
    PropertyFunction property_function,
    Node* any_type_node) {

    // TODO: Get rid of the ANY/NONE type.
    // this is a hack associated with Github issue #101 that needs to be fixed.
    return LirGenResult(&TYPE_NONE, NULL);
}

// Optional implemented function that overrides base class implementation.
// In SemanticAstWalker, this function overrides default action for walking
// a spec and pushes another layer to the symbol_table_ to allow for declaration
// of types and fields private to a spec, namely, Generic params.
void LIRGenAstWalker::WalkSpec(Node* spec_node, PrescanMode scan_mode) {

    this->register_table_.Push();

    // Offset of next declared property from beginning of the object.
    // Set to zero, we're starting a new object.
    this->property_offset_ = 0;

    // Call parent class implementation.
    AstWalker::WalkSpec(spec_node, scan_mode);

    this->register_table_.Pop();
}

// Optional implemented function that overrides base class implementation.
// In LIRGenAstWalker, this function allocates a NanoJIT fragment to contain
// LIR for the function.
void LIRGenAstWalker::WalkFunctionChildren(
    Node* spec_node,
    Node* function_node,
    bool prescan) {

    this->param_offset_ = 0;

    // If we're doing code generation now:
    if (!prescan) {
        // TODO: make sure that this is freed.
        LirBuffer* buf = new (alloc_) LirBuffer(alloc_);
        buf->abi = ABI_CDECL;

        // Allocate a fragment for the function
        this->current_fragment_ = new Fragment(NULL verbose_only(,0));
        this->current_fragment_->lirbuf = buf;
        this->current_writer_ = new LirBufWriter(buf, this->config_);

        // Write function start:
        this->current_writer_->ins0(LIR_start);
    }

    // Push new level to the register table so we don't have variables
    // collide across scopes.
    this->register_table_.Push();

    // Walk the function and insert function body instructions:
    AstWalker::WalkFunctionChildren(spec_node, function_node, prescan);

    this->register_table_.Pop();

    const FunctionSymbol* function_symbol = SYMBOL_TO_FUNCTION(function_node->symbol());

    // If we're doing code generation now:
    if (prescan) {
        GS_ASSERT_TRUE(
            this->current_function_index_ < this->debug_functions_count_,
            "Not enough space in function pointer table.");

        // NanoJIT requires function pointers to know where to jump at a function call.
        // Unfortunately there IS no pointer until after assembly, so we work around this
        // restriction by storing function pointers to the functions in a list in the Module.
        // Now, instead of knowing a pointer to the function (where the function is),
        // all we need is to know where we WILL be able to find the pointer (where we can look
        // to get the pointer). These are stored in a contiguous buffer and are identified
        // positionally to correspond with the location of the function in the symbols_vector.
        // NOTE: this code working properly is dependent upon the assumption that the prescan
        // and the code gen steps are done in EXACTLY the same order every time.
        this->register_table_.PutBottom(
            function_symbol->symbol_name(),
            std::make_tuple(
                &TYPE_FUNCTION,
                reinterpret_cast<LIns*>(&(this->func_table_[this->current_function_index_++])),
                0));
    } else {

        // This is the end of the function, emit default return of zero.
        // Gunderscript 2.0 has no control flow analysis so this ensures that every function
        // returns.
        // For integer and floating point types this means default is numeric zero (0 or 0.0).
        // For BOOL default is false and for object default is NULL.
        LIns* ret_inst = NULL;
        const TypeSymbol* function_type_symbol = function_symbol->type_symbol();
        switch (function_type_symbol->size())
        {
        case 8:
            GS_ASSERT_TRUE(function_type_symbol->type_format() == TypeFormat::POINTER,
                "Expected POINTER");
            ret_inst = this->current_writer_->ins1(LIR_retp, this->current_writer_->insImmP(NULL));
            break;
        case 4:
        case 1:
            switch (function_type_symbol->type_format())
            {
            case TypeFormat::INT:
            case TypeFormat::BOOL:
                ret_inst = this->current_writer_->ins1(LIR_reti, this->current_writer_->insImmI(0));
                break;
            case TypeFormat::FLOAT:
                ret_inst = this->current_writer_->ins1(LIR_retf, this->current_writer_->insImmF(0.0f));
                break;
            case TypeFormat::POINTER:
                ret_inst = this->current_writer_->ins1(LIR_retp, this->current_writer_->insImmP(NULL));
                break;
            default:
                GS_ASSERT_FAIL("Unimplemented default return");
            }
            break;
        case 0:
            GS_ASSERT_TRUE(*function_type_symbol == TYPE_VOID, "Expected void type");
            ret_inst = this->current_writer_->ins1(LIR_reti, this->current_writer_->insImmI(0));
            break;
        default:
            GS_ASSERT_FAIL("Unimplemented default return");
        }
        this->current_fragment_->lastIns = ret_inst;

        // Record the symbol in the vector that's gonna go inside of the module.
        // None of these calls should throw or return incorrect values. If they do,
        // there is probably a problem with the lexer, parser, or typechecker.
        this->symbols_vector_->push_back(
            ModuleImplSymbol(
                function_symbol->symbol_name(),
                function_symbol->type_symbol()->Clone(),
                this->current_fragment_));

        // TODO: delete this somehow?? delete this->current_fragment_->lirbuf;
        delete this->current_writer_;
        this->current_fragment_ = NULL;
        this->current_writer_ = NULL;
    }
}

// Walks children of the if statement node.
// This is an optional override of the default functionality that gives the code
// generator more granular control of the iteration process.
void LIRGenAstWalker::WalkIfStatementChildren(
    Node* spec_node,
    Node* function_node,
    Node* property_node,
    PropertyFunction property_function,
    Node* if_node,
    std::vector<LirGenResult>* arguments_result) {

    GS_ASSERT_OPTIONAL_NODE_RULE(spec_node, NodeRule::SPEC);
    GS_ASSERT_OPTIONAL_NODE_RULE(function_node, NodeRule::FUNCTION);
    GS_ASSERT_OPTIONAL_NODE_RULE(property_node, NodeRule::PROPERTY);

    // Walk condition expression.
    LirGenResult condition_result = AstWalker<LirGenResult>::WalkExpressionChildren(
        spec_node,
        function_node,
        property_node,
        property_function,
        if_node->child(0));

    // Jump to the else BLOCK if the condition is false. The label will be backpatched later.
    // TODO: can we do this without the comparison? I have it here because some cases fail without it.
    LIns* jump_false_ins = this->current_writer_->insBranch(LIR_jt,
        this->current_writer_->ins2(LIR_eqi,
            this->current_writer_->insImmI(0),
            condition_result.ins()), NULL);

    // Walk true block and generate instructions.
    WalkBlockChildren(
        spec_node,
        function_node,
        property_node,
        property_function,
        if_node->child(1),
        arguments_result);

    // At the end of the true block jump past the end of the if/else.
    LIns* jump_end_ins = this->current_writer_->insBranch(LIR_j, NULL, NULL);

    // Walk false block and generate instructions.
    jump_false_ins->setTarget(this->current_writer_->ins0(LIR_label));
    WalkBlockChildren(
        spec_node,
        function_node,
        property_node,
        property_function,
        if_node->child(2),
        arguments_result);

    jump_end_ins->setTarget(this->current_writer_->ins0(LIR_label));
}

// Walks a new Spec(arg1, arg2, ...) expression.
LirGenResult LIRGenAstWalker::WalkNewExpression(
    Node* new_node,
    Node* type_node,
    std::vector<LirGenResult>& arguments_result) {

    // Lookup the spec's size and allocate enough memory to hold its properties.
    // NOTE: there is no need to catch exceptions from this table. If it throws
    // then the AstWalker is walking in the incorrect order.
    int alloc_size = this->type_size_table_.at(type_node->symbol()->symbol_name());
    LIns* size_arg[] = { this->current_writer_->insImmI(alloc_size) };

    LIns* alloc_call_inst = this->current_writer_->insCall(&runtime::CI_GC_ALLOC, size_arg);

    LirGenResult alloc_result(
        type_node->symbol(),
        alloc_call_inst);

    // Call the constructor on our object.
    WalkFunctionCall(
        new_node->symbol(),
        arguments_result,
        &alloc_result);

    // Register allocation algorithm is imperfect. Turns out that in this particular
    // case it treats RBX and EBX as separate registers on x64 when, in actuality,
    // they overlap and are 32 bit and 64 bit respectively. When we called into the
    // constructor above, writing to EBX overwrote the address we saved in RBX.
    // LIR_regfence indicates to the reg allocator that it needs to start over
    // and fixes this.
    this->current_writer_->ins0(LIR_regfence);

    return alloc_result;
}

// Walks a default() expression.
LirGenResult LIRGenAstWalker::WalkDefaultExpression(
    Node* default_node,
    Node* type_node) {

    const TypeSymbol* type_symbol = default_node->symbol()->type_symbol();

    LIns* value = NULL;
    switch (type_symbol->type_format())
    {
    case TypeFormat::BOOL:
    case TypeFormat::INT:
        if (type_symbol->size() == 4 || type_symbol->size() == 1) {
            value = this->current_writer_->insImmI(0);
        }
        break;
    case TypeFormat::FLOAT:
        if (type_symbol->size() == 4) {
            value = this->current_writer_->insImmF(0.0);
        }
        break;
    case TypeFormat::POINTER:
        value = this->current_writer_->insImmP(NULL);
        break;
    default:
        GS_ASSERT_FAIL("Unhandled default value case");
    }

    return LirGenResult(type_symbol, value);
}

// Walks children of the FOR statement node.
// This is an optional override of the default functionality that gives the code
// generator more granular control of the walking process.
void LIRGenAstWalker::WalkForStatementChildren(
    Node* spec_node,
    Node* function_node,
    Node* property_node,
    PropertyFunction property_function,
    Node* for_node,
    std::vector<LirGenResult>* arguments_result) {

    if (spec_node != NULL) {
        GS_ASSERT_TRUE(spec_node->rule() == NodeRule::SPEC,
            "Expected SPEC in typechecker WalkForStatementChildren");
    }
    if (function_node != NULL) {
        GS_ASSERT_TRUE(function_node->rule() == NodeRule::FUNCTION,
            "Expected FUNCTION in typechecker WalkForStatementChildren");
    }
    if (property_node != NULL) {
        GS_ASSERT_TRUE(property_node->rule() == NodeRule::PROPERTY,
            "Expected CALL in typechecker WalkForStatementChildren");
    }

    // Generate code for optional init expression if given.
    Node* init_node = for_node->child(0);
    GS_ASSERT_TRUE(init_node->rule() == NodeRule::LOOP_INITIALIZE,
        "Expected LOOP_INITIALIZE in typecheker WalkForStatementChildren");
    if (init_node->child_count() > 0) {
        WalkExpressionChildren(
            spec_node,
            function_node,
            property_node,
            property_function,
            init_node->child(0));
    }

    // Mark start of condition block.
    LIns* cond_label_ins = this->current_writer_->ins0(LIR_label);
    LIns* cond_ins = NULL;

    // Generate code for optional condition expression if given.
    // If not given, default behavior is to loop infinitely.
    Node* cond_node = for_node->child(1);
    GS_ASSERT_TRUE(cond_node->rule() == NodeRule::LOOP_CONDITION,
        "Expected LOOP_CONDITION in typecheker WalkForStatementChildren");
    if (cond_node->child_count() > 0) {
        LirGenResult cond_result = WalkExpressionChildren(
            spec_node,
            function_node,
            property_node,
            property_function,
            cond_node->child(0));

        // If the loop condition is false, jump out. This jump is backpatched later on.
        // TODO: can we get rid of this extra EQI?
        cond_ins = this->current_writer_->insBranch(LIR_jt,
            this->current_writer_->ins2(LIR_eqi,
                this->current_writer_->insImmI(0),
                cond_result.ins()), NULL);
    }
    // else: No loop condition, no jump, do the loop infinitely many times.

    // Generate for loop body block code.
    WalkBlockChildren(
        spec_node,
        function_node,
        property_node,
        property_function,
        for_node->child(3),
        arguments_result);

    // Codegen update expression if provided.
    Node* update_node = for_node->child(2);
    GS_ASSERT_TRUE(update_node->rule() == NodeRule::LOOP_UPDATE,
        "Expected LOOP_UPDATE in typecheker WalkForStatementChildren");
    if (update_node->child_count() > 0) {
        WalkExpressionChildren(
            spec_node,
            function_node,
            property_node,
            property_function,
            update_node->child(0));
    }

    // Jump to loop condition to repeat the loop.
    this->current_writer_->insBranch(LIR_j, NULL, cond_label_ins);

    // Backpatch jumps out of the loop.
    if (cond_ins != NULL) {
        cond_ins->setTarget(this->current_writer_->ins0(LIR_label));
    }
}

// Optional implemented function that overrides base class implementation.
// In LIRGenAstWalker, this function pushes a new table to the register_table_
// to introduce new context for each BLOCK ('{' to '}') entered, limiting the
// scope of Local LIns* so they don't collide across scopes.
void LIRGenAstWalker::WalkBlockChildren(
    Node* spec_node,
    Node* function_node,
    Node* property_node,
    PropertyFunction property_function,
    Node* block,
    std::vector<LirGenResult>* arguments_result) {

    // Push new scope.
    this->register_table_.Push();

    // Walk the Block.
    AstWalker::WalkBlockChildren(
        spec_node,
        function_node,
        property_node,
        property_function,
        block,
        arguments_result);

    // Pop the scope.
    this->register_table_.Pop();
}

// Emits a load instruction.
LIns* LIRGenAstWalker::EmitLoad(const SymbolBase* symbol, LIns* base, int offset) {

    const TypeSymbol* type_symbol = symbol->type_symbol();

    switch (type_symbol->type_format()) {
    case TypeFormat::POINTER:
        return this->current_writer_->insLoad(LIR_ldp, base, offset, ACCSET_ALL, LoadQual::LOAD_VOLATILE);
    case TypeFormat::BOOL:
    case TypeFormat::INT:
        // BOOL types are simply integers that contain either 1 or 0.
        switch (type_symbol->size()) {
        case 4:
            // TODO: tighter access sets.
            return this->current_writer_->insLoad(LIR_ldi, base, offset, ACCSET_ALL, LoadQual::LOAD_VOLATILE);
        case 1:
            // Load 1 byte INT8/CHAR value and sign extend to fill the 32 bit register.
            // For those unfamilar with low level ops, all registers are >= 32 bit on our target
            // machines. There are no 1 byte registers.
            return this->current_writer_->insLoad(LIR_ldc2i, base, offset, ACCSET_ALL, LoadQual::LOAD_VOLATILE);
        default:
            THROW_NOT_IMPLEMENTED();
        }
        break;
    case TypeFormat::FLOAT:
        switch (type_symbol->size()) {
        case 4:
            // TODO: tighter access sets.
            return this->current_writer_->insLoad(LIR_ldf, base, offset, ACCSET_ALL, LoadQual::LOAD_VOLATILE);
        default:
            THROW_NOT_IMPLEMENTED();
        }
        break;

    default:
        THROW_NOT_IMPLEMENTED();
    }
}

// Emits a store instruction.
LIns* LIRGenAstWalker::EmitStore(const SymbolBase* symbol, LIns* base, int offset, LIns* value) {

    const TypeSymbol* type_symbol = symbol->type_symbol();

    switch (type_symbol->type_format()) {
    case TypeFormat::POINTER:
        return this->current_writer_->insStore(LIR_stp, value, base, offset, ACCSET_ALL);
    case TypeFormat::BOOL:
    case TypeFormat::INT:
        // BOOL types are simply integers that contain either 1 or 0.
        switch (type_symbol->size()) {
        case 4:
            // TODO: tighter access sets.
            return this->current_writer_->insStore(LIR_sti, value, base, offset, ACCSET_ALL);
        case 1:
            // Store 4 byte INT32/INT value and truncate to the 1 byte buffer.
            return this->current_writer_->insStore(LIR_sti2c, value, base, offset, ACCSET_ALL);
        default:
            THROW_NOT_IMPLEMENTED();
        }
        break;
    case TypeFormat::FLOAT:
        switch (type_symbol->size()) {
        case 4:
            // TODO: tighter access sets.
            return this->current_writer_->insStore(LIR_stf, value, base, offset, ACCSET_ALL);
        default:
            THROW_NOT_IMPLEMENTED();
        }
        break;

    default:
        THROW_NOT_IMPLEMENTED();
    }
}

// Counts the total number of functions produced by this module.
int LIRGenAstWalker::CountFunctions() {
    size_t count = 0;

    // Count static functions.
    count += this->root().child(3)->child_count();

    // Count member functions.
    Node* specs_node = this->root().child(2);
    for (size_t i = 0; i < specs_node->child_count(); i++) {
        Node* current_spec_node = specs_node->child(i);
        Node* current_spec_functions_node = current_spec_node->child(2);
        for (size_t j = 0; j < current_spec_functions_node->child_count(); j++, count++);
    }

    // TODO: may need to modify to support additional functions for properties setters/getters.

    return (int)count;
}

} // namespace compiler
} // namespace gunderscript
