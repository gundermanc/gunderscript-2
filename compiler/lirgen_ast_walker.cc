// Gunderscript-2 NanoJIT LIR Generator
// (C) 2016 Christian Gunderman

#include "gunderscript/exceptions.h"

#include "lirgen_ast_walker.h"

using namespace nanojit;

namespace gunderscript {
namespace compiler {

void LIRGenAstWalker::Generate(Module& module) {
    
    // Check if a module has already been compiled into this module file.
    if (module.compiled()) {
        THROW_EXCEPTION(1, 1, STATUS_INVALID_CALL);
    }

    // Set the output Module symbol vector as the current symbol vector.
    // This stores all exported symbols in the vector.
    this->symbols_vector_ = &module.pimpl()->symbols_vector();

    // Walk through the AST and find everything we need for our new Module.
    this->Walk();
}

// Walks the MODULE node in the abstract syntax tree.
// Since there is no code gen information we can ignore it.
void LIRGenAstWalker::WalkModule(Node* module_node) {
    // Module itself has no properties to check.
    // We instead check its child elements individually.
}

// Walks the MODULE node's NAME node. This node defines the module name
// (analogous to the Java package name) of a script file.
// We save a reference to the name for later.
void LIRGenAstWalker::WalkModuleName(Node* name_node) {
    this->module_name_ = name_node->string_value();
}

// Handles depends statements.
void LIRGenAstWalker::WalkModuleDependsName(Node* name_node) {
    // TODO: implement support for depends statements.
    THROW_EXCEPTION(
        name_node->line(),
        name_node->column(),
        STATUS_ILLEGAL_STATE);
}

// Attempts to declare a new spec in the given scope. Throws if spec
// name is taken in this context.
void LIRGenAstWalker::WalkSpecDeclaration(
    Node* spec_node,
    Node* access_modifier_node,
    Node* name_node) {

    // TODO: do we need to export anything for a spec, or just for the elements of the spec?
}

// Walks a single function declaration inside of a SPEC and generates the code for it.
void LIRGenAstWalker::WalkSpecFunctionDeclaration(
    Node* spec_node,
    Node* function_node,
    Node* access_modifier_node,
    Node* native_node,
    Node* type_node,
    Node* name_node,
    Node* block_node,
    std::vector<LirGenResult>& arguments_result,
    bool prescan) {

    // TODO: do we need anything else here?
}

// Walks a single parameter in a spec function declaration.
// Returns it to the Function Declaration walker.
LirGenResult LIRGenAstWalker::WalkSpecFunctionDeclarationParameter(
    Node* spec_node,
    Node* function_node,
    Node* type_node,
    Node* function_param_node,
    Node* name_node,
    bool prescan) {

    return LirGenResult();
}

// Walks a single property in a spec property declaration.
// Defines it in the symbol table.
void LIRGenAstWalker::WalkSpecPropertyDeclaration(
    Node* spec_node,
    Node* type_node,
    Node* name_node,
    Node* get_property_function_node,
    Node* set_property_function_node,
    Node* get_access_modifier_node,
    Node* set_access_modifier_node,
    bool prescan) {

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

    // TODO: implement function call code generation.
    THROW_EXCEPTION(1, 1, STATUS_ILLEGAL_STATE);
}

// Walks a function-like typecast and generates code for the type conversions.
LirGenResult LIRGenAstWalker::WalkFunctionLikeTypecast(
    Node* spec_node,
    Node* name_node,
    Node* call_node,
    LirGenResult argument_result) {

    // If types are the same, no conversion neccessary.
    if (argument_result.type_symbol()->type() == call_node->symbol()->type()) {
        return argument_result;
    }

    switch (call_node->symbol()->type().type_format()) {
    case TypeFormat::INT:
        if (argument_result.type_symbol()->type().type_format() == TypeFormat::FLOAT) {
            return LirGenResult(call_node->symbol(),
                this->current_writer_->ins1(LIR_f2i, argument_result.ins()));
        }
        else if (argument_result.type_symbol()->type().type_format() == TypeFormat::INT) {

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
            return argument_result;
        }
        break;

    case TypeFormat::FLOAT:
        if (argument_result.type_symbol()->type() == TYPE_INT) {
            return LirGenResult(call_node->symbol(),
                this->current_writer_->ins1(LIR_i2f, argument_result.ins()));
        }
        break;

    case TypeFormat::BOOL:
        break;
    }

    // Not implemented.
    THROW_EXCEPTION(1, 1, STATUS_ILLEGAL_STATE);
}

// Walks an assignment statement or expression and checks to make sure
// that the types match the context in which it was used.
LirGenResult LIRGenAstWalker::WalkAssign(
    Node* spec_node,
    Node* name_node,
    Node* assign_node,
    LirGenResult operations_result) {

    LIns* variable_ptr = NULL;

    // Look in the table to see if we have already alloc-ed space for this variable.
    try {
        variable_ptr = this->register_table_.Get(*name_node->string_value());
    }
    catch (const Exception& ex) {

        // Check to see if the exception thrown is a SYMBOL_UNDEFINED, if so, alloc it.
        if (ex.status().code() == STATUS_SYMBOLTABLE_UNDEFINED_SYMBOL.code()) {
            variable_ptr = this->current_writer_->insAlloc(operations_result.type_symbol()->type().size());
            this->register_table_.Put(*name_node->string_value(), variable_ptr);
        }
        else {
            // Unrecognized exception, rethrow.
            throw;
        }
    }

    LIns* store_ins;
    switch (operations_result.type_symbol()->type().type_format())
    {
    case TypeFormat::BOOL:
    case TypeFormat::INT:
        // BOOL types are simply INT values that are either 1 or 0.
        switch (operations_result.type_symbol()->type().size())
        {
        case 4:
            // TODO: tighter access sets.
            store_ins = this->current_writer_->insStore(LIR_sti, operations_result.ins(), variable_ptr, 0, ACCSET_ALL);
            break;
        case 1:
            // Although 1 byte INT8/CHAR data type is treated like an INT32/INT in the
            // other operations storage is where the differences in its size come to view.
            // It is stored in a 1 byte footprint and extended to an INT32/INT only for purposes
            // of filling a 32 bit register.
            store_ins = this->current_writer_->insStore(LIR_sti2c, operations_result.ins(), variable_ptr, 0, ACCSET_ALL);
            break;
        default:
            // Unknown integer size.
            THROW_EXCEPTION(1, 1, STATUS_ILLEGAL_STATE);
        }
        break;
    case TypeFormat::FLOAT:
        switch (operations_result.type_symbol()->type().size())
        {
        case 4:
            // TODO: tighter access sets.
            store_ins = this->current_writer_->insStore(LIR_stf, operations_result.ins(), variable_ptr, 0, ACCSET_ALL);
            break;
        default:
            // Unknown double size.
            THROW_EXCEPTION(1, 1, STATUS_ILLEGAL_STATE);
        }
        break;

    default:
        // Unknown type format size.
        THROW_EXCEPTION(1, 1, STATUS_ILLEGAL_STATE);
    }

    // HACK: in order to support nesting of assign statements like this:
    // y <- (x <- 3 + 2)
    // we generate a load instruction right after our assign and return it
    // to the caller.
    return LirGenResult(operations_result.type_symbol(),
        GenerateLoad(operations_result.type_symbol(), variable_ptr));
}

// Walks and validates a return value type for a function or property.
// TODO: make this work for properties.
LirGenResult LIRGenAstWalker::WalkReturn(
    Node* spec_node,
    Node* function_node,
    Node* property_node,
    PropertyFunction property_function,
    LirGenResult expression_result,
    std::vector<LirGenResult>* arguments_result) {

    switch (expression_result.type_symbol()->type().type_format())
    {
    case TypeFormat::INT:
        switch (expression_result.type_symbol()->type().size())
        {
        case 4:
        case 1:
            // Return of 1 byte INT8/CHAR values is still LIR_reti because the actual
            // operation occurs in a standard >= 32 bit register on many systems.
            return LirGenResult(NULL, this->current_writer_->ins1(LIR_reti, expression_result.ins()));
        default:
            // Unknown integer size.
            THROW_EXCEPTION(1, 1, STATUS_ILLEGAL_STATE);
        }
        break;
    default:
        // Unknown return type.
        THROW_EXCEPTION(1, 1, STATUS_ILLEGAL_STATE);
    }
}

// Walks the ADD node and calculates it's return type.
LirGenResult LIRGenAstWalker::WalkAdd(
    Node* spec_node,
    Node* left_node,
    Node* right_node,
    LirGenResult left_result,
    LirGenResult right_result) {

    // Ignore right_result types, we already went through the semantic_ast_walker.
    // Types are known.
    switch (left_result.type_symbol()->type().type_format())
    {
    case TypeFormat::INT:
        switch (left_result.type_symbol()->type().size())
        {
        case 4:
        case 1:
            // Addition of 1 INT8/CHAR values is still LIR_addi because the actual
            // operation occurs in a standard >= 32 bit register.
            return LirGenResult(
                left_result.type_symbol(),
                this->current_writer_->ins2(LIR_addi, left_result.ins(), right_result.ins()));
        default:
            // Unhandled size.
            THROW_EXCEPTION(1, 1, STATUS_ILLEGAL_STATE);
        }
        break;
    case TypeFormat::FLOAT:
        switch (left_result.type_symbol()->type().size())
        {
        case 4:
            return LirGenResult(
                left_result.type_symbol(),
                this->current_writer_->ins2(LIR_addf, left_result.ins(), right_result.ins()));
        default:
            // Unhandled size.
            THROW_EXCEPTION(1, 1, STATUS_ILLEGAL_STATE);
        }
        break;
    default:
        // Unhandled type format.
        THROW_EXCEPTION(1, 1, STATUS_ILLEGAL_STATE);
    }
}

// Walks the SUB node and calculates it's return type.
LirGenResult LIRGenAstWalker::WalkSub(
    Node* spec_node,
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
    switch (right_result.type_symbol()->type().type_format())
    {
    case TypeFormat::INT:
        switch (right_result.type_symbol()->type().size())
        {
        case 4:
        case 1:
            // Subtraction of 1 byte INT8/CHAR values is still LIR_subi because the actual
            // operation occurs in a standard >= 32 bit register.
            return LirGenResult(
                right_result.type_symbol(),
                this->current_writer_->ins2(LIR_subi, left_ins, right_result.ins()));
        default:
            // Unhandled size.
            THROW_EXCEPTION(1, 1, STATUS_ILLEGAL_STATE);
        }
        break;
    case TypeFormat::FLOAT:
        switch (right_result.type_symbol()->type().size())
        {
        case 4:
            return LirGenResult(
                right_result.type_symbol(),
                this->current_writer_->ins2(LIR_subf, left_ins, right_result.ins()));
        default:
            // Unhandled size.
            THROW_EXCEPTION(1, 1, STATUS_ILLEGAL_STATE);
        }
        break;
    default:
        // Unhandled type format.
        THROW_EXCEPTION(1, 1, STATUS_ILLEGAL_STATE);
    }
}

// Walks the MUL node and calculates it's return type.
LirGenResult LIRGenAstWalker::WalkMul(
    Node* spec_node,
    Node* left_node,
    Node* right_node,
    LirGenResult left_result,
    LirGenResult right_result) {

    // Ignore right_result types, we already went through the semantic_ast_walker.
    // Types are known.
    switch (left_result.type_symbol()->type().type_format())
    {
    case TypeFormat::INT:
        switch (left_result.type_symbol()->type().size())
        {
        case 4:
        case 1:
            // Multiplication of 1 INT8/CHAR value is still LIR_muli because the actual
            // operation occurs in a standard >= 32 bit register.
            return LirGenResult(
                left_result.type_symbol(),
                this->current_writer_->ins2(LIR_muli, left_result.ins(), right_result.ins()));
        default:
            // Unhandled size.
            THROW_EXCEPTION(1, 1, STATUS_ILLEGAL_STATE);
        }
        break;
    case TypeFormat::FLOAT:
        switch (left_result.type_symbol()->type().size())
        {
        case 4:
            return LirGenResult(
                left_result.type_symbol(),
                this->current_writer_->ins2(LIR_mulf, left_result.ins(), right_result.ins()));
        default:
            // Unhandled size.
            THROW_EXCEPTION(1, 1, STATUS_ILLEGAL_STATE);
        }
        break;
    default:
        // Unhandled type format.
        THROW_EXCEPTION(1, 1, STATUS_ILLEGAL_STATE);
    }
}

// Walks the DIV node and calculates it's return type.
LirGenResult LIRGenAstWalker::WalkDiv(
    Node* spec_node,
    Node* left_node,
    Node* right_node,
    LirGenResult left_result,
    LirGenResult right_result) {

    // Ignore right_result types, we already went through the semantic_ast_walker.
    // Types are known.
    switch (left_result.type_symbol()->type().type_format())
    {
    case TypeFormat::INT:
        switch (left_result.type_symbol()->type().size())
        {
        case 4:
        case 1:
            // Division of 1 INT8/CHAR values is still LIR_divi because the actual
            // operation occurs in a standard >= 32 bit register.
            return LirGenResult(
                left_result.type_symbol(),
                this->current_writer_->ins2(LIR_divi, left_result.ins(), right_result.ins()));
        default:
            // Unhandled size.
            THROW_EXCEPTION(1, 1, STATUS_ILLEGAL_STATE);
        }
        break;
    case TypeFormat::FLOAT:
        switch (left_result.type_symbol()->type().size())
        {
        case 4:
            return LirGenResult(
                left_result.type_symbol(),
                this->current_writer_->ins2(LIR_divf, left_result.ins(), right_result.ins()));
        default:
            // Unhandled size.
            THROW_EXCEPTION(1, 1, STATUS_ILLEGAL_STATE);
        }
        break;
    default:
        // Unhandled type format.
        THROW_EXCEPTION(1, 1, STATUS_ILLEGAL_STATE);
    }
}

// Walks the MOD node and calculates it's return type.
LirGenResult LIRGenAstWalker::WalkMod(
    Node* spec_node,
    Node* left_node,
    Node* right_node,
    LirGenResult left_result,
    LirGenResult right_result) {

    // Ignore right_result types, we already went through the semantic_ast_walker.
    // Types are known.
    switch (left_result.type_symbol()->type().type_format())
    {
    case TypeFormat::INT:
        switch (left_result.type_symbol()->type().size())
        {
        case 4:
        case 1:
            // Modulo of 1 INT8/CHAR values is still LIR_modi because the actual
            // operation occurs in a standard >= 32 bit register.
#if defined NANOJIT_IA32 || defined NANOJIT_X64
            // LIR_modi instruction is only supported on x86 and x86_64 platforms.
            // TODO: If porting to ARM, SPARC, etc, be sure to fix this.
            return LirGenResult(
                left_result.type_symbol(),
                this->current_writer_->ins1(LIR_modi,
                    this->current_writer_->ins2(LIR_divi, left_result.ins(), right_result.ins())));
#endif
        default:
            // Unhandled size.
            THROW_EXCEPTION(1, 1, STATUS_ILLEGAL_STATE);
        }
        break;
    default:
        // Unhandled type format.
        THROW_EXCEPTION(1, 1, STATUS_ILLEGAL_STATE);
    }
}

// Walks the LOGNOT node and calculates it's return type.
LirGenResult LIRGenAstWalker::WalkLogNot(
    Node* spec_node,
    Node* child_node,
    LirGenResult child_result) {

    return LirGenResult();
}

// Walks the LOGAND node and calculates it's return type.
LirGenResult LIRGenAstWalker::WalkLogAnd(
    Node* spec_node,
    Node* left_node,
    Node* right_node,
    LirGenResult left_result,
    LirGenResult right_result) {

    return LirGenResult();
}

// Walks the LOGOR node and calculates it's return type.
LirGenResult LIRGenAstWalker::WalkLogOr(
    Node* spec_node,
    Node* left_node,
    Node* right_node,
    LirGenResult left_result,
    LirGenResult right_result) {

    return LirGenResult();
}

// Walks the GREATER node and calculates it's return type.
LirGenResult LIRGenAstWalker::WalkGreater(
    Node* spec_node,
    Node* left_node,
    Node* right_node,
    LirGenResult left_result,
    LirGenResult right_result) {

    return LirGenResult();
}

// Walks the EQUALS node and calculates it's return type.
LirGenResult LIRGenAstWalker::WalkEquals(
    Node* spec_node,
    Node* left_node,
    Node* right_node,
    LirGenResult left_result,
    LirGenResult right_result) {

    return LirGenResult();
}

// Walks the NOT_EQUALS node and calculates it's return type.
LirGenResult LIRGenAstWalker::WalkNotEquals(
    Node* spec_node,
    Node* left_node,
    Node* right_node,
    LirGenResult left_result,
    LirGenResult right_result) {

    return LirGenResult();
}

// Walks the LESS node and calculates it's return type.
LirGenResult LIRGenAstWalker::WalkLess(
    Node* spec_node,
    Node* left_node,
    Node* right_node,
    LirGenResult left_result,
    LirGenResult right_result) {

    return LirGenResult();
}

// Walks the GREATER_EQUALS node and calculates it's return type.
LirGenResult LIRGenAstWalker::WalkGreaterEquals(
    Node* spec_node,
    Node* left_node,
    Node* right_node,
    LirGenResult left_result,
    LirGenResult right_result) {

    return LirGenResult();
}

// Walks the LESS_EQUALS node and calculates it's return type.
LirGenResult LIRGenAstWalker::WalkLessEquals(
    Node* spec_node,
    Node* left_node,
    Node* right_node,
    LirGenResult left_result,
    LirGenResult right_result) {

    return LirGenResult();
}

// Walks the TYPE_BOOL node and returns the type for it.
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

// Walks the TYPE_INT node and returns the type for it.
LirGenResult LIRGenAstWalker::WalkInt(
    Node* spec_node,
    Node* function_node,
    Node* property_node,
    PropertyFunction property_function,
    Node* int_node) {

    return LirGenResult(int_node->symbol(),
        this->current_writer_->insImmI((int32_t)int_node->int_value()));
}

// Walks the FLOAT node and returns the type for it.
LirGenResult LIRGenAstWalker::WalkFloat(
    Node* spec_node,
    Node* function_node,
    Node* property_node,
    PropertyFunction property_function,
    Node* float_node) {

    return LirGenResult(float_node->symbol(),
        this->current_writer_->insImmF(float_node->float_value()));
}

// Walks the STRING node and returns the type for it.
LirGenResult LIRGenAstWalker::WalkString(
    Node* spec_node,
    Node* function_node,
    Node* property_node,
    PropertyFunction property_function,
    Node* string_node) {

    return LirGenResult();
}

// Walks the CHAR node and returns the type for it.
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
// and returns the type for it.
LirGenResult LIRGenAstWalker::WalkVariable(
    Node* spec_node,
    Node* function_node,
    Node* property_node,
    PropertyFunction property_function,
    Node* variable_node,
    Node* name_node) {

    // No try/catch here, if register_table_ throws then the type checker made
    // a boo boo and didn't notice the missing variable initialization.
    const Symbol* variable_symbol = variable_node->symbol();
    LIns* load = GenerateLoad(
        variable_symbol,
        this->register_table_.Get(*name_node->string_value()));

    return LirGenResult(variable_symbol, load);
}

// Walks the ANY_TYPE node and returns the type for it.
LirGenResult LIRGenAstWalker::WalkAnyType(
    Node* spec_node,
    Node* function_node,
    Node* property_node,
    PropertyFunction property_function,
    Node* any_type_node) {

    return LirGenResult();
}

// Optional implemented function that overrides base class implementation.
// In LIRGenAstWalker, this function allocates a NanoJIT fragment to contain
// LIR for the function.
void LIRGenAstWalker::WalkSpecFunctionChildren(
    Node* spec_node,
    Node* function_node,
    bool prescan) {

    // Run once.
    if (!prescan) {
        // TODO: make sure that this is freed.
        LirBuffer* buf = new (alloc_) LirBuffer(alloc_);

        // Allocate a fragment for the function.
        this->current_fragment_ = new Fragment(NULL);
        this->current_fragment_->lirbuf = buf;
        this->current_writer_ = new LirBufWriter(buf, this->config_);
        buf->abi = ABI_CDECL;

        // Write function start:
        this->current_writer_->ins0(LIR_start);
    }

    // Push new level to the register table so we don't have variables
    // collide across scopes.
    this->register_table_.Push();

    // Walk the function and insert function body instructions:
    AstWalker::WalkSpecFunctionChildren(spec_node, function_node, prescan);

    this->register_table_.Pop();

    // Run once.
    if (!prescan) {
        const Symbol* function_symbol = function_node->symbol();

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
            if (function_symbol->type().type_format() == TypeFormat::INT) {
                ret_value = this->current_writer_->insImmI(0);
                ret_inst = this->current_writer_->ins1(LIR_reti, ret_value);
                break;
            }
            else if (function_symbol->type().type_format() == TypeFormat::FLOAT) {
                ret_value = this->current_writer_->insImmF(0.0f);
                ret_inst = this->current_writer_->ins1(LIR_retf, ret_value);
                break;
            }
            break;

        case 1:
            // TODO: no idea how to insert a 1 byte CHAR or BOOL. Might have to make this an INT.
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
                *function_symbol,
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

// Generates a load instruction.
LIns* LIRGenAstWalker::GenerateLoad(const Symbol* symbol, LIns* base) {

    switch (symbol->type().type_format()) {
    case TypeFormat::BOOL:
    case TypeFormat::INT:
        // BOOL types are simply integers that contain either 1 or 0.
        switch (symbol->type().size()) {
        case 4:
            // TODO: tighter access sets.
            return this->current_writer_->insLoad(LIR_ldi, base, 0, ACCSET_ALL, LoadQual::LOAD_NORMAL);
        case 1:
            // Load 1 byte INT8/CHAR value and sign extend to fill the 32 bit register.
            // For those unfamilar with low level ops, all registers are >= 32 bit on our target
            // machines. There are no 1 byte registers.
            return this->current_writer_->insLoad(LIR_ldc2i, base, 0, ACCSET_ALL, LoadQual::LOAD_NORMAL);
        default:
            THROW_EXCEPTION(1, 1, STATUS_ILLEGAL_STATE);
        }
        break;
    case TypeFormat::FLOAT:
        switch (symbol->type().size()) {
        case 4:
            // TODO: tighter access sets.
            return this->current_writer_->insLoad(LIR_ldf, base, 0, ACCSET_ALL, LoadQual::LOAD_NORMAL);
        default:
            THROW_EXCEPTION(1, 1, STATUS_ILLEGAL_STATE);
        }
        break;

    default:
        THROW_EXCEPTION(1, 1, STATUS_ILLEGAL_STATE);
    }
}

} // namespace compiler
} // namespace gunderscript
