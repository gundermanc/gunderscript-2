// Gunderscript-2 NanoJIT LIR Generator
// (C) 2016 Christian Gunderman

#include <cmath>

#include "gunderscript/exceptions.h"

#include "gs_assert.h"
#include "lirgen_ast_walker.h"

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
    ABI_CDECL, 1, ACCSET_STORE_ANY verbose_only(, "fmod")};

// Calling conventions for Gunderscript generated methods. One per machine level return type (int, float).
// These methods all take two arguments: A function pointer and a pointer to a vector of arguments in the stack
// and they return the function's return value. Admittedly this is weird but this is how arguments are passed in
// Tamarin as well.
const CallInfo CI_ICALLI = {
    CALL_INDIRECT,
    CallInfo::typeSig2(ARGTYPE_I, ARGTYPE_P, ARGTYPE_P),
    ABI_CDECL, ACCSET_STORE_ANY, 1 verbose_only(, "CallIndirect_int32")};
const CallInfo CI_FCALLI = {
    CALL_INDIRECT,
    CallInfo::typeSig2(ARGTYPE_F, ARGTYPE_P, ARGTYPE_P),
    ABI_CDECL, ACCSET_STORE_ANY, 1 verbose_only(, "CallIndirect_float32" )};

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

    // Store the function lookup table in the module.
    module.pimpl()->set_func_table(this->func_table_);
}

// Handles depends statements.
void LIRGenAstWalker::WalkModuleDependsName(Node* name_node) {
    // TODO: implement support for depends statements.
    THROW_NOT_IMPLEMENTED();
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

    // Only run during actual code generation, not during the prescan for out of order functions.
    if (!prescan) {

        // Determine argument size.
        int arg_size = function_param_node->symbol()->type().size();

        // Store the param load address in the register_table_
        // TODO: make address calculation at compile instead of runtime.
        // I haven't done this yet because it is dependedent upon redesigning the gen code to
        // not store and load at EVERY variable assign and reference.
        this->register_table_.Put(
            function_param_node->symbol()->name(),
            std::make_tuple(
                function_param_node->symbol()->type(),
                RegisterEntry(
                    this->current_writer_->ins2(LIR_addp,
                        this->current_writer_->insParam(/* args vector*/ 0, /*func param kind*/0),
                        this->current_writer_->insImmI(arg_size * this->param_offset_++)))));
    }

    return LirGenResult(function_param_node->symbol()->type(), NULL);
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
    // TODO: declare spec properties code.
}

// Walks a function call and function-like typecasts and emits code.
LirGenResult LIRGenAstWalker::WalkFunctionCall(
    Node* spec_node,
    Node* name_node,
    Node* call_node,
    std::vector<LirGenResult>& arguments_result) {

    const Symbol& function_symbol = call_node->symbol();

    // Check if this is a function-like typecast.
    // If so generate typecast code.
    if (function_symbol.symbol_type() == SymbolType::TYPE) {
        return WalkFunctionLikeTypecast(
            spec_node,
            name_node,
            call_node, arguments_result.at(0));
    }

    // Lookup the function's register entry.
    // Register entry contains register information for variables and 
    // a pointer to a location that will contain a function pointer after compilation
    // for functions. This is done to avoid the need to backpatch function addresses later.
    std::tuple<Type, RegisterEntry> function_reg_tuple = this->register_table_.Get(call_node->symbol()->name());

    // NanoJIT unfortunately depends on having a function pointer to jump to for function calls,
    // however, we don't have one readily available for most functions since compilation hasn't
    // completed. We get around this limitation by keeping a table of function pointers
    // in this->func_table_ (and later in moduleimpl). After compilation this table is populated
    // with pointers to each of our functions. Since we know WHERE the pointers are we can simply
    // write instructions to load them and call them. This also has the added benefit of making
    // it easy to perform partial recompiles by simply recompiling the modified function and
    // updating its pointer.
    LIns* target = this->current_writer_->insLoad(LIR_ldp,
        this->current_writer_->insImmP(std::get<1>(function_reg_tuple).func_), 0, ACCSET_ALL, LOAD_NORMAL);

    // Choose indirect call method. Arguments are passed as a single argument in a vector so
    // only one call method is needed per return type.
    const CallInfo* method = NULL;
    switch (call_node->symbol()->type().type_format())
    {
    case TypeFormat::BOOL:
    case TypeFormat::INT:
        method = &CI_ICALLI;
        break;
    case TypeFormat::FLOAT:
        method = &CI_FCALLI;
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

            EmitStore(argument.type(), arguments_vector, arg_offset, argument.ins());
            arg_offset += argument.type().size();
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
    LIns* args[] = { arguments_vector, target };
    LIns* call = this->current_writer_->insCall(method, args);

    // Ensure the arguments remain live until after the call.
    this->current_writer_->ins1(LIR_livep, arguments_vector);

    return LirGenResult(call_node->symbol()->type(), call);
}

// Walks a function-like typecast and generates code for the type conversions.
LirGenResult LIRGenAstWalker::WalkFunctionLikeTypecast(
    Node* spec_node,
    Node* name_node,
    Node* call_node,
    LirGenResult argument_result) {

    // If types are the same, no conversion neccessary.
    if (argument_result.type() == call_node->symbol()->type()) {
        return argument_result;
    }

    switch (call_node->symbol()->type().type_format()) {
    case TypeFormat::INT:
        if (argument_result.type().type_format() == TypeFormat::FLOAT) {
            return LirGenResult(call_node->symbol()->type(),
                this->current_writer_->ins1(LIR_f2i, argument_result.ins()));
        }
        else if (argument_result.type().type_format() == TypeFormat::INT ||
            argument_result.type().type_format() == TypeFormat::BOOL) {

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
            return LirGenResult(call_node->symbol()->type(), argument_result.ins());
        }
        break;

    case TypeFormat::FLOAT:
        if (argument_result.type().type_format() == TypeFormat::INT ||
            argument_result.type().type_format() == TypeFormat::BOOL) {
            return LirGenResult(call_node->symbol()->type(),
                this->current_writer_->ins1(LIR_i2f, argument_result.ins()));
        }
        break;

    case TypeFormat::BOOL:
        if (argument_result.type().type_format() == TypeFormat::INT) {
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

            return LirGenResult(call_node->symbol()->type(), choose_ins);
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
        variable_ptr = std::get<1>(this->register_table_.GetTopOnly(*name_node->string_value())).ins_;
        goto emit_assign_ins;
    }
    catch (const Exception&) {

        // Variable was not bound in the current scope, check if it was bound in any scope.
        // If so, if it is the same type as our expression then we are assigning to the already
        // declared variable.
        // If not, this variable is a narrower scope variable of the same name and a different
        // type that is masking the old one.
        try {
            std::tuple<Type, RegisterEntry> variable_reg = this->register_table_.Get(*name_node->string_value());

            if (std::get<0>(variable_reg) == operations_result.type()) {
                variable_ptr = std::get<1>(variable_reg).ins_;
                goto emit_assign_ins;
            }
            /* else: Fall into new alloc */
        }
        catch (const Exception&) { /* Fall into new alloc */ }
    }

    // New Alloc: Allocate a new register (NanoJIT register) for this variable name,
    // it has either never been seen before or is a new var of a different type masking
    // the original definition in an enclosing scope.
    variable_ptr = this->current_writer_->insAlloc(operations_result.type().size());
    this->register_table_.Put(*name_node->string_value(), std::make_tuple(operations_result.type(), RegisterEntry(variable_ptr) ));

    // Emit the assignment instruction. Dijkstra doesn't have to agree with me, gotos can be useful.
emit_assign_ins:
    EmitStore(operations_result.type(), variable_ptr, 0, operations_result.ins());

    // HACK: in order to support nesting of assign statements like this:
    // y <- (x <- 3 + 2)
    // we generate a load instruction right after our assign and return it
    // to the caller.
    return LirGenResult(operations_result.type(),
        EmitLoad(operations_result.type(), variable_ptr, 0));
}

// Walks a return statement and generates code for it.
// TODO: make this work for properties.
LirGenResult LIRGenAstWalker::WalkReturn(
    Node* spec_node,
    Node* function_node,
    Node* property_node,
    PropertyFunction property_function,
    LirGenResult expression_result,
    std::vector<LirGenResult>* arguments_result) {

    switch (expression_result.type().type_format())
    {
    case TypeFormat::BOOL:
    case TypeFormat::INT:
        switch (expression_result.type().size())
        {
        case 4:
        case 1:
            // Return of 1 byte INT8/CHAR values is still LIR_reti because the actual
            // operation occurs in a standard >= 32 bit register on many systems.
            return LirGenResult(expression_result.type(), 
                this->current_writer_->ins1(LIR_reti, expression_result.ins()));
        }
        break;
    case TypeFormat::FLOAT:
        switch (expression_result.type().size())
        {
        case 4:
            // Return of 1 byte INT8/CHAR values is still LIR_reti because the actual
            // operation occurs in a standard >= 32 bit register on many systems.
            return LirGenResult(expression_result.type(),
                this->current_writer_->ins1(LIR_retf, expression_result.ins()));
        }
        break;
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

    // Ignore right_result types, we already went through the semantic_ast_walker.
    // Types are known.
    switch (left_result.type().type_format())
    {
    case TypeFormat::INT:
        switch (left_result.type().size())
        {
        case 4:
        case 1:
            // Addition of 1 INT8/CHAR values is still LIR_addi because the actual
            // operation occurs in a standard >= 32 bit register.
            return LirGenResult(
                left_result.type(),
                this->current_writer_->ins2(LIR_addi, left_result.ins(), right_result.ins()));
        }
        break;
    case TypeFormat::FLOAT:
        switch (left_result.type().size())
        {
        case 4:
            return LirGenResult(
                left_result.type(),
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

    LIns* left_ins = left_result.ins();

    // HACK: negative numbers special case.
    if (left_result.ins() == NULL) {
        left_ins = this->current_writer_->insImmI(0);
    }

    // HACK: For the WalkSub case in particular we MUST use ONLY the RIGHT node's type because
    // the SemanticAstWalker returns ANY_TYPE for the left operand to get around the explict
    // typecast requirement for all operands with negative numbers.
    // TODO: see Github Issue #101 for fix for this.
    switch (right_result.type().type_format())
    {
    case TypeFormat::INT:
        switch (right_result.type().size())
        {
        case 4:
        case 1:
            // Subtraction of 1 byte INT8/CHAR values is still LIR_subi because the actual
            // operation occurs in a standard >= 32 bit register.
            return LirGenResult(
                right_result.type(),
                this->current_writer_->ins2(LIR_subi, left_ins, right_result.ins()));
        }
        break;
    case TypeFormat::FLOAT:
        switch (right_result.type().size())
        {
        case 4:
            return LirGenResult(
                right_result.type(),
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

    // Ignore right_result types, we already went through the semantic_ast_walker.
    // Types are known.
    switch (left_result.type().type_format())
    {
    case TypeFormat::INT:
        switch (left_result.type().size())
        {
        case 4:
        case 1:
            // Multiplication of 1 INT8/CHAR value is still LIR_muli because the actual
            // operation occurs in a standard >= 32 bit register.
            return LirGenResult(
                left_result.type(),
                this->current_writer_->ins2(LIR_muli, left_result.ins(), right_result.ins()));
        }
        break;
    case TypeFormat::FLOAT:
        switch (left_result.type().size())
        {
        case 4:
            return LirGenResult(
                left_result.type(),
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

    // Ignore right_result types, we already went through the semantic_ast_walker.
    // Types are known.
    switch (left_result.type().type_format())
    {
    case TypeFormat::INT:
        switch (left_result.type().size())
        {
        case 4:
        case 1:
            // Division of 1 INT8/CHAR values is still LIR_divi because the actual
            // operation occurs in a standard >= 32 bit register.
            return LirGenResult(
                left_result.type(),
                this->current_writer_->ins2(LIR_divi, left_result.ins(), right_result.ins()));
        }
        break;
    case TypeFormat::FLOAT:
        switch (left_result.type().size())
        {
        case 4:
            return LirGenResult(
                left_result.type(),
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

    // Ignore right_result types, we already went through the semantic_ast_walker.
    // Types are known.
    switch (left_result.type().type_format())
    {
    case TypeFormat::INT:
        switch (left_result.type().size())
        {
        case 4:
        case 1:
            // Modulo of 1 INT8/CHAR values is still LIR_modi because the actual
            // operation occurs in a standard >= 32 bit register.
#if defined NANOJIT_IA32 || defined NANOJIT_X64
            return LirGenResult(
                left_result.type(),
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
        switch (left_result.type().size())
        {
        case 4:
        {
            // x86/x86_64 has no native support for floating point operations on floats so
            // we wrap and call the cmath fmod function.
            // Since we are pushing the args onto the stack apparently they have to be
            // in reverse order.
            LIns* float_args[3] = { right_result.ins(), left_result.ins(), NULL };
            return LirGenResult(
                left_result.type(),
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

    // XORing a boolean value by 1 effectively performs effectively performs a NOT
    // operation: 1 xor 1 = 0; 0 xor 1 = 1;
    // This assumption works as long as we force booleans to ALWAYS be 0 or 1 only.
    // TODO: can we do this with one instruction instead of two??
    return LirGenResult(
        TYPE_BOOL,
        this->current_writer_->ins2(LIR_xori,
            this->current_writer_->insImmI(1),
            child_result.ins()));
}

// Walks the LOGAND node and generates code for it.
LirGenResult LIRGenAstWalker::WalkLogAnd(
    Node* spec_node,
    Node* log_and_node,
    Node* left_node,
    Node* right_node,
    LirGenResult left_result,
    LirGenResult right_result) {

    // XORing a boolean value by 1 effectively performs effectively performs a NOT
    // operation: 1 xor 1 = 0; 0 xor 1 = 1;
    // This assumption works as long as we force booleans to ALWAYS be 0 or 1 only.
    // TODO: can we do this with one instruction instead of two??
    LIns* negated_left_inst = this->current_writer_->ins2(
        LIR_xori,
        this->current_writer_->insImmI(1),
        left_result.ins());

    // Short circuit evaluation of AND instruction:
    // If the left condition is true (because we negated it,
    // we return the value true immediately. If it is false (it was true
    // before the negation we instead obtain our true/false value by
    // evaluating the right instruction which may be a true,
    // false, or a tree of additional boolean operations.
    // TODO: can we use CMOV here? CMOV might break short circuit eval.
    return LirGenResult(
        TYPE_BOOL,
        this->current_writer_->insChoose(
            negated_left_inst,
            this->current_writer_->insImmI(0),
            right_result.ins(),
            false));
}

// Walks the LOGOR node and generates code for it and its children.
LirGenResult LIRGenAstWalker::WalkLogOr(
    Node* spec_node,
    Node* log_or_node,
    Node* left_node,
    Node* right_node,
    LirGenResult left_result,
    LirGenResult right_result) {

    // Short circuit evaluation of OR instruction:
    // If the left condition is true, we return the value true
    // immediately. If it is false we instead obtain our true/false
    // value by evaluating the right instruction which may be a true,
    // false, or a tree of additional boolean operations.
    // TODO: can we use CMOV here? CMOV might break short circuit eval.
    return LirGenResult(
        TYPE_BOOL,
        this->current_writer_->insChoose(
            left_result.ins(),
            this->current_writer_->insImmI(1),
            right_result.ins(),
            false));
}

// Walks the GREATER node and generates code for it.
LirGenResult LIRGenAstWalker::WalkGreater(
    Node* spec_node,
    Node* greater_node,
    Node* left_node,
    Node* right_node,
    LirGenResult left_result,
    LirGenResult right_result) {

    // Left and right are already the same type thanks to the typechecker.
    switch (left_result.type().type_format()) {
    case TypeFormat::INT:
        if (left_result.type().size() == 4 ||
            left_result.type().size() == 1) {
            return LirGenResult(
                TYPE_BOOL,
                this->current_writer_->ins2(
                    LIR_gti,
                    left_result.ins(),
                    right_result.ins()));
        }
        break;
    case TypeFormat::FLOAT:
        if (left_result.type().size() == 4) {
            return LirGenResult(
                TYPE_BOOL,
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

    // Left and right are already the same type thanks to the typechecker.
    switch (left_result.type().type_format()) {
    case TypeFormat::INT:
        if (left_result.type().size() == 4 ||
            left_result.type().size() == 1) {
            return LirGenResult(
                TYPE_BOOL,
                this->current_writer_->ins2(
                    LIR_eqi,
                    left_result.ins(),
                    right_result.ins()));
        }
        break;
    case TypeFormat::FLOAT:
        if (left_result.type().size() == 4) {
            return LirGenResult(
                TYPE_BOOL,
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

    // Left and right are already the same type thanks to the typechecker.
    switch (left_result.type().type_format()) {
    case TypeFormat::INT:
        if (left_result.type().size() == 4 ||
            left_result.type().size() == 1) {

            // TODO: can we do this with one instruction??
            // XORing 1 or 0 by 1 flips.
            return LirGenResult(
                TYPE_BOOL,
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
        if (left_result.type().size() == 4) {

            // TODO: can we do this with one instruction??
            // XORing 1 or 0 by 1 flips.
            return LirGenResult(
                TYPE_BOOL,
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

    // Left and right are already the same type thanks to the typechecker.
    switch (left_result.type().type_format()) {
    case TypeFormat::INT:
        if (left_result.type().size() == 4 ||
            left_result.type().size() == 1) {
            return LirGenResult(
                TYPE_BOOL,
                this->current_writer_->ins2(
                    LIR_lti,
                    left_result.ins(),
                    right_result.ins()));
        }
        break;
    case TypeFormat::FLOAT:
        if (left_result.type().size() == 4) {
            return LirGenResult(
                TYPE_BOOL,
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

    // Left and right are already the same type thanks to the typechecker.
    switch (left_result.type().type_format()) {
    case TypeFormat::INT:
        if (left_result.type().size() == 4 ||
            left_result.type().size() == 1) {
            return LirGenResult(
                TYPE_BOOL,
                this->current_writer_->ins2(
                    LIR_gei,
                    left_result.ins(),
                    right_result.ins()));
        }
        break;
    case TypeFormat::FLOAT:
        if (left_result.type().size() == 4) {
            return LirGenResult(
                TYPE_BOOL,
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

    // Left and right are already the same type thanks to the typechecker.
    switch (left_result.type().type_format()) {
    case TypeFormat::INT:
        if (left_result.type().size() == 4 ||
            left_result.type().size() == 1) {
            return LirGenResult(
                TYPE_BOOL,
                this->current_writer_->ins2(
                    LIR_lei,
                    left_result.ins(),
                    right_result.ins()));
        }
        break;
    case TypeFormat::FLOAT:
        if (left_result.type().size() == 4) {
            return LirGenResult(
                TYPE_BOOL,
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
    return LirGenResult(bool_node->symbol()->type(),
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

    return LirGenResult(int_node->symbol()->type(),
        this->current_writer_->insImmI((int32_t)int_node->int_value()));
}

// Walks the FLOAT node and generates code for it.
LirGenResult LIRGenAstWalker::WalkFloat(
    Node* spec_node,
    Node* function_node,
    Node* property_node,
    PropertyFunction property_function,
    Node* float_node) {

    return LirGenResult(float_node->symbol()->type(),
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
    return LirGenResult(char_node->symbol()->type(),
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

    // No try/catch here, if register_table_ throws then the type checker made
    // a boo boo and didn't notice the missing variable initialization.
    const Type& variable_type = variable_node->symbol()->type();
    LIns* load = EmitLoad(
        variable_type,
        std::get<1>(this->register_table_.Get(*name_node->string_value())).ins_,
        0);

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
    return LirGenResult(TYPE_NONE, NULL);
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

    const Symbol* function_symbol = function_node->symbol();

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
        this->register_table_.PutBottom(
            function_symbol->name(),
            std::make_tuple(TYPE_FUNCTION, RegisterEntry(&(this->func_table_[this->current_function_index_++]))));
    } else {

        // This is the end of the function, emit default return of zero.
        // Gunderscript 2.0 has no control flow analysis so this ensures that every function
        // returns.
        // For integer and floating point types this means default is numeric zero (0 or 0.0).
        // For BOOL default is false and for object default is NULL.
        LIns* ret_value = NULL;
        LIns* ret_inst = NULL;
        switch (function_symbol->type().size()) 
        {
        case 4:
        case 1:
            if (function_symbol->type().type_format() == TypeFormat::INT ||
                function_symbol->type().type_format() == TypeFormat::BOOL) {
                ret_value = this->current_writer_->insImmI(0);
                ret_inst = this->current_writer_->ins1(LIR_reti, ret_value);
                break;
            }
            else if (function_symbol->type().type_format() == TypeFormat::FLOAT) {
                ret_value = this->current_writer_->insImmF(0.0f);
                ret_inst = this->current_writer_->ins1(LIR_retf, ret_value);
                break;
            }
        default:
            THROW_EXCEPTION(1, 1, STATUS_ILLEGAL_STATE);
        }
        this->current_fragment_->lastIns = ret_inst;

        // Record the symbol in the vector that's gonna go inside of the module.
        // None of these calls should throw or return incorrect values. If they do,
        // there is probably a problem with the lexer, parser, or typechecker.
        this->symbols_vector_->push_back(
            ModuleImplSymbol(
                function_symbol->name(),
                new Symbol(function_symbol),
                this->current_fragment_));

        // TODO: delete this somehow?? delete this->current_fragment_->lirbuf;
        delete this->current_writer_;
        this->current_fragment_ = NULL;
        this->current_writer_ = NULL;
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
LIns* LIRGenAstWalker::EmitLoad(const Type& type, LIns* base, int offset) {

    switch (type.type_format()) {
    case TypeFormat::BOOL:
    case TypeFormat::INT:
        // BOOL types are simply integers that contain either 1 or 0.
        switch (type.size()) {
        case 4:
            // TODO: tighter access sets.
            return this->current_writer_->insLoad(LIR_ldi, base, offset, ACCSET_ALL, LoadQual::LOAD_NORMAL);
        case 1:
            // Load 1 byte INT8/CHAR value and sign extend to fill the 32 bit register.
            // For those unfamilar with low level ops, all registers are >= 32 bit on our target
            // machines. There are no 1 byte registers.
            return this->current_writer_->insLoad(LIR_ldc2i, base, offset, ACCSET_ALL, LoadQual::LOAD_NORMAL);
        default:
            THROW_NOT_IMPLEMENTED();
        }
        break;
    case TypeFormat::FLOAT:
        switch (type.size()) {
        case 4:
            // TODO: tighter access sets.
            return this->current_writer_->insLoad(LIR_ldf, base, 0, ACCSET_ALL, LoadQual::LOAD_NORMAL);
        default:
            THROW_NOT_IMPLEMENTED();
        }
        break;

    default:
        THROW_NOT_IMPLEMENTED();
    }
}

// Emits a store instruction.
LIns* LIRGenAstWalker::EmitStore(const Type& type, LIns* base, int offset, LIns* value) {

    switch (type.type_format()) {
    case TypeFormat::BOOL:
    case TypeFormat::INT:
        // BOOL types are simply integers that contain either 1 or 0.
        switch (type.size()) {
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
        switch (type.size()) {
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