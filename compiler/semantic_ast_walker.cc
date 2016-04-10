// Gunderscript-2 Semantic (type) Checker for Abstract Syntax Tree
// (C) 2015-2016 Christian Gunderman

#include <regex>

#include "gunderscript/node.h"

#include "gs_assert.h"
#include "lexer.h"
#include "parser.h"
#include "semantic_ast_walker.h"
#include "symbolimpl.h"

namespace gunderscript {
namespace compiler {

// The pattern for checking module names.
const std::regex module_name_pattern = std::regex("^([A-Z]|[a-z])+(\\.([A-Z]|[a-z])+)?$");

// Mangles function symbol name to include class name and arguments so that
// symbol table guarantees uniqueness for functions while allowing overloads
// with different argument types.
static const std::string MangleFunctionSymbolName(
    const std::string& spec_name,
    const std::string& function_name,
    std::vector<const SymbolBase*>& arguments_result) {

    // Format the symbol name {spec}::{function}$arg1$arg2...
    // or {function}$arg1$arg2... if there is no spec.
    std::ostringstream name_buf;
    name_buf << spec_name;
    name_buf << "::";
    name_buf << function_name;

    // Append arguments to the symbol name.
    for (size_t i = 0; i < arguments_result.size(); i++) {
        name_buf << "$";
        name_buf << arguments_result[i]->type_symbol()->symbol_name();
    }

    return name_buf.str();
}

// Wrapper for the other MangleFunctionSymbolName.
static const std::string MangleFunctionSymbolName(
    Node* spec_node,
    Node* name_node,
    std::vector<const SymbolBase*>& arguments_result) {

    return MangleFunctionSymbolName(
        spec_node != NULL ? spec_node->symbol()->symbol_name() : "",
        *name_node->string_value(),
        arguments_result);
}

// Mangles local variable symbol name to the format Local%%{variable}
// so that they do not collide with class names in the symbol table.
static const std::string MangleLocalVariableSymbolName(Node* name_node) {
    std::ostringstream name_buf;
    name_buf << "Local%%";
    name_buf << *name_node->string_value();

    return name_buf.str();
}

// Mangles a property function symbol name.
static const std::string ManglePropertyFunctionSymbolName(
    Node* spec_node,
    Node* name_node,
    PropertyFunction function) {

    // Format the getter symbol name {class}<-{function}
    std::ostringstream name_buf;
    name_buf << spec_node->symbol()->symbol_name();
    name_buf << (function == PropertyFunction::SET ? "->" : "<-");
    name_buf << *name_node->string_value();

    return name_buf.str();
}

// Mangles a Spec Template symbol name to indicate a number of generic params.
static const std::string MangleSpecTemplateSymbolName(Node* type_node) {
    std::ostringstream name_buf;
    name_buf << *type_node->string_value();

    // Add type template params.
    for (size_t i = 0; i < type_node->child_count(); i++) {
        name_buf << '~';
    }

    return name_buf.str();
}

// Constructor, populates symbol table with Types.
SemanticAstWalker::SemanticAstWalker(Node& node) : AstWalker(node), symbol_table_() {

    // Add all default types to the Symbol table.
    for (size_t i = 0; i < BUILTIN_TYPES.size(); i++) {
        const SymbolBase* current_type = BUILTIN_TYPES[i];

        this->symbol_table_.PutBottom(current_type->symbol_name(), current_type);
    }
}

// Walks the MODULE node in the abstract syntax tree.
// Since there is no type information in this node, we can
// safely do nothing.
void SemanticAstWalker::WalkModule(Node* module_node) {
    // Module itself has no properties to check.
    // We instead check its child elements individually.
}

// Walks the MODULE node's NAME node. This node defines the module name
// (analogous to the Java package name) of a script file.
// The only verification performed at the moment is simple naive name
// pattern matching.
void SemanticAstWalker::WalkModuleName(Node* name_node) {
    CheckValidModuleName(
        *name_node->string_value(),
        name_node->line(),
        name_node->column());
}

// Checks that the given dependency module name is valid.
// If not, throws an exception.
// TODO: calculate dependency graph and lex/parse/typecheck the
// dependency first.
void SemanticAstWalker::WalkModuleDependsName(Node* name_node) {
    CheckValidModuleName(
        *name_node->string_value(),
        name_node->line(),
        name_node->column());
}

void SemanticAstWalker::WalkSpecDeclarationPrescan(
    Node* spec_node,
    Node* access_modifier_node,
    Node* type_node) {

    const std::string type_symbol_name = MangleSpecTemplateSymbolName(type_node);
    const TypeSymbol* spec_symbol = NULL;

    // Check if this is a generic type. If so, it gets a specialized symbol with params.
    if (type_node->child_count() == 0) {
        spec_symbol = new TypeSymbol(
            access_modifier_node->symbol_value(),
            type_symbol_name);
    }
    else {
        std::vector<const SymbolBase*> type_params;

        // Walk the type params, if any, and store them as template
        // params.
        for (size_t i = 0; i < type_node->child_count(); i++) {
            Node* child_node = type_node->child(i);
            const TypeSymbol* template_type_symbol = new TypeSymbol(
                SymbolType::TYPE_TEMPLATE,
                LexerSymbol::CONCEALED,
                *child_node->string_value(),
                TypeFormat::POINTER,
                sizeof(void*));

            child_node->set_symbol(template_type_symbol);
            type_params.push_back(template_type_symbol);
        }

        // Walk the properties and add space for them

        spec_symbol = new GenericTypeSymbol(
            SymbolType::GENERIC_TYPE_TEMPLATE,
            access_modifier_node->symbol_value(),
            type_symbol_name,
            type_params);
    }

    // Store a reference to the symbol. Node will destroy symbol on cleanup.
    spec_node->set_symbol(spec_symbol);

    try {
        this->symbol_table_.PutBottom(
            type_symbol_name,
            spec_symbol);
    }
    catch (const Exception& ex) {
        // Rethrow as more relevant exception.
        if (ex.status() == STATUS_SYMBOLTABLE_DUPLICATE_SYMBOL) {
            THROW_EXCEPTION(
                type_node->line(),
                type_node->column(),
                STATUS_SEMANTIC_DUPLICATE_SPEC);
        }

        throw;
    }
}

// Attempts to declare a new spec in the given scope. Throws if spec
// name is taken in this context.
void SemanticAstWalker::WalkSpecDeclaration(
    Node* spec_node,
    Node* access_modifier_node,
    Node* type_node,
    PrescanMode scan_mode) {

    switch (scan_mode)
    {
    case PrescanMode::SCAN_SPEC_DEF:
        WalkSpecDeclarationPrescan(
            spec_node,
            access_modifier_node,
            type_node);
        break;
    case PrescanMode::SCAN_IMPL_DEF:
    {
        TypeSymbol* symbol = spec_node->symbol()->symbol_type() == SymbolType::GENERIC_TYPE_TEMPLATE
            ? const_cast<GenericTypeSymbol*>(SYMBOL_TO_GENERIC_TYPE_TEMPLATE(spec_node->symbol()))
            : const_cast<TypeSymbol*>(SYMBOL_TO_TYPE(spec_node->symbol()));

        Node* properties_node = spec_node->child(3);
        GS_ASSERT_TRUE(properties_node->rule() == NodeRule::PROPERTIES, "Expected PROPERTIES node");

        // Retroactively populate Types with their member properties.
        for (size_t i = 0; i < properties_node->child_count(); i++) {
            Node* property_node = properties_node->child(i);
            GS_ASSERT_TRUE(property_node->rule() == NodeRule::PROPERTY, "Expected PROPERTY node");
            symbol->add_member(property_node->symbol());
        }
    }
        
    case PrescanMode::SCAN_PROP_FUNC_DEF:
        if (spec_node->symbol()->symbol_type() == SymbolType::GENERIC_TYPE_TEMPLATE) {

            // Symbol for type_node was added during the prescan, just read it.
            const GenericTypeSymbol* type_symbol
                = SYMBOL_TO_GENERIC_TYPE_TEMPLATE(spec_node->symbol());

            // We overrode the function that calls this one later in this file and pushed a new
            // layer to the stack. Now, we'll define a series of template types in this local
            // scope for this spec. These types go out of scope after we finish.
            for (size_t i = 0; i < type_symbol->type_params().size(); i++) {
                const SymbolBase* template_type_symbol = type_symbol->type_params().at(i);

                try {
                    this->symbol_table_.Put(template_type_symbol->symbol_name(), template_type_symbol);
                }
                catch (const Exception& ex) {

                    // Rethrow as more relevant exception.
                    if (ex.status() == STATUS_SYMBOLTABLE_DUPLICATE_SYMBOL) {
                        THROW_EXCEPTION(
                            type_node->line(),
                            type_node->column(),
                            STATUS_SEMANTIC_GENERIC_DUPLICATE_PARAM);
                    }

                    throw;
                }
            }
        }
        break;
    default:
        GS_ASSERT_FAIL("Unhandled case.");
    }
}

// Walks a single function declaration inside of a SPEC.
// Throws if the function already exists in the symbol table.
void SemanticAstWalker::WalkFunctionDeclaration(
    Node* spec_node,
    Node* function_node,
    Node* access_modifier_node,
    Node* type_node,
    Node* name_node,
    Node* block_node,
    std::vector<const SymbolBase*>& arguments_result,
    bool prescan) {

    const std::string spec_name = spec_node != NULL ? spec_node->symbol()->symbol_name() : "";

    // Only define the symbol during the prescan portion of the walking process.
    // The prescan allows for us to iterate and define symbols for all functions
    // before we type check the function body so functions can reference one another
    // regardless of the order in which they are declared.
    if (prescan) {
        try {
            const std::string symbol_name = MangleFunctionSymbolName(spec_node, name_node, arguments_result);

            const SymbolBase* function_symbol = new FunctionSymbol(
                SymbolType::FUNCTION,
                access_modifier_node->symbol_value(),
                spec_name,
                symbol_name,
                ResolveTypeNode(type_node));

            // Node will destroy symbol on cleanup.
            function_node->set_symbol(function_symbol);

            this->symbol_table_.PutBottom(
                symbol_name,
                function_symbol);
        }
        catch (const Exception& ex) {

            // Throw a more relevant exception.
            if (ex.status() == STATUS_SYMBOLTABLE_DUPLICATE_SYMBOL) {
                THROW_EXCEPTION(
                    name_node->line(),
                    name_node->column(),
                    STATUS_SEMANTIC_DUPLICATE_FUNCTION);
            }
            else {
                throw;
            }
        }
    }
}

// Walks a single parameter in a spec function declaration.
// Returns it to the Function Declaration walker.
const SymbolBase* SemanticAstWalker::WalkSpecFunctionDeclarationParameter(
    Node* spec_node,
    Node* function_node,
    Node* type_node,
    Node* function_param_node,
    Node* name_node,
    bool prescan) {

    const TypeSymbol* type_symbol = ResolveTypeNode(type_node)->type_symbol();

    // Disallow void type as function param.
    if (*type_symbol == TYPE_VOID) {
        THROW_EXCEPTION(
            type_node->line(),
            type_node->column(),
            STATUS_SEMANTIC_VOID_USED_IN_PARAM);
    }

    // Create a new symbol for the parameter.
    const SymbolBase* param_symbol = new FunctionSymbol(
        SymbolType::PARAM,
        LexerSymbol::CONCEALED,
        "",
        *name_node->string_value(),
        type_symbol);

    function_param_node->set_symbol(param_symbol);

    try {
        // Insert the symbol into the table.
        this->symbol_table_.Put(
            MangleLocalVariableSymbolName(name_node),
            param_symbol);
    }
    catch (const Exception& ex) {

        // Throw a more relevant exception.
        if (ex.status() == STATUS_SYMBOLTABLE_DUPLICATE_SYMBOL) {
            THROW_EXCEPTION(
                name_node->line(),
                name_node->column(),
                STATUS_SEMANTIC_DUPLICATE_FUNCTION_PARAM);
        }
        else {
            throw;
        }
    }

    // Return the type.
    return param_symbol;
}

// Walks a single property in a spec property declaration.
// Defines it in the symbol table.
void SemanticAstWalker::WalkSpecPropertyDeclaration(
    Node* spec_node,
    Node* type_node,
    Node* name_node,
    Node* get_property_function_node,
    Node* set_property_function_node,
    Node* get_access_modifier_node,
    Node* set_access_modifier_node,
    bool prescan) {

    // Only define symbols for the property functions if this is the prescan.
    // Prescan is an initial scan of the properties by signature only and is
    // useful because it allows us to define symbols for the properties before
    // we type check their bodies, allowing properties to be aware of one another.
    if (!prescan) {
        return;
    }

    const std::string spec_name = spec_node->symbol()->symbol_name();

    // Determine the getter function symbol name.
    std::string get_function_symbol_name
        = ManglePropertyFunctionSymbolName(spec_node, name_node, PropertyFunction::GET);

    const SymbolBase* type_symbol = ResolveTypeNode(type_node);

    // Create the symbol table symbol for the getter.
    const SymbolBase* get_function_symbol = new FunctionSymbol(
        SymbolType::PROPERTY,
        get_access_modifier_node->symbol_value(),
        spec_name,
        get_function_symbol_name,
        type_symbol);

    get_property_function_node->set_symbol(get_function_symbol);

    // Determine the setter function symbol name.
    std::string set_function_symbol_name
        = ManglePropertyFunctionSymbolName(spec_node, name_node, PropertyFunction::SET);

    // Create the symbol table symbol for the getter.
    const SymbolBase* set_function_symbol = new FunctionSymbol(
        SymbolType::FUNCTION,
        set_access_modifier_node->symbol_value(),
        spec_name,
        set_function_symbol_name,
        type_symbol);

    set_property_function_node->set_symbol(set_function_symbol);

    try {
        // Define the getter symbol.
        this->symbol_table_.PutBottom(get_function_symbol_name, get_function_symbol);

        // Define the setter symbol.
        this->symbol_table_.PutBottom(set_function_symbol_name, set_function_symbol);
    }
    catch (const Exception& ex) {

        // Rethrow as more understandable error.
        if (ex.status() == STATUS_SYMBOLTABLE_DUPLICATE_SYMBOL) {
            THROW_EXCEPTION(
                name_node->line(),
                name_node->column(),
                STATUS_SEMANTIC_DUPLICATE_PROPERTY);
        }

        throw;
    }
}

// Walks a function call and checks to make sure that the types
// of the function matches the context.
const SymbolBase* SemanticAstWalker::WalkFunctionCall(
    Node* spec_node,
    Node* name_node,
    Node* call_node,
    std::vector<const SymbolBase*>& arguments_result) {

    const SymbolBase* function_symbol = NULL;

    // Lookup the function in this class. Throws if there isn't a function with the correct arguments.
    try {
        function_symbol = this->symbol_table_.Get(
            MangleFunctionSymbolName(spec_node, name_node, arguments_result));
    }
    catch (const Exception& ex) {

        // The symbol for this function is unknown. Let's try an out of spec (static) function.
        if (ex.status() != STATUS_SYMBOLTABLE_UNDEFINED_SYMBOL) {
            throw;
        }

        try {
            function_symbol = this->symbol_table_.Get(
                MangleFunctionSymbolName(NULL, name_node, arguments_result));
        }
        catch (const Exception& ex) {

            // The symbol for this function is unknown. Either a typo or a function-like typecast.
            if (ex.status() == STATUS_SYMBOLTABLE_UNDEFINED_SYMBOL) {

                // Check if we have exactly one param. If so, it might be a typecast.
                if (arguments_result.size() != 1) {
                    THROW_EXCEPTION(
                        name_node->line(),
                        name_node->column(),
                        STATUS_SEMANTIC_FUNCTION_OVERLOAD_NOT_FOUND);
                }

                // Try walking it as if it were a typecast.
                return WalkFunctionLikeTypecast(
                    spec_node,
                    name_node,
                    call_node,
                    arguments_result.at(0));
            }

            throw;
        }
    }

    // Copied symbol because ~Node() destroys symbol on exit.
    call_node->set_symbol(function_symbol->Clone());

    // Check for access to the callee function.
    // TODO: as of the moment this does nothing because we don't support calls between classes
    // yet. Implement calls between classes.
    CheckAccessModifier(
        spec_node != NULL ? spec_node->symbol()->symbol_name() : "",
        function_symbol->spec_name(),
        function_symbol->access_modifier(),
        name_node->line(),
        name_node->column());

    return function_symbol->type_symbol();
}

// Walks and typechecks the if statement.
void SemanticAstWalker::WalkIfStatement(
    Node* spec_node,
    Node* if_node,
    const SymbolBase* condition_result) {

    // The body of the if, else, and else_if if present are type checked automatically
    // as they are BLOCK nodes. All we need to explicitly check here is the condition type.
    if (*condition_result != TYPE_BOOL) {
        THROW_EXCEPTION(
            if_node->line(),
            if_node->column(),
            STATUS_SEMANTIC_INVALID_IF_CONDITION_TYPE);
    }
}

// Checks the types in a for statement.
void SemanticAstWalker::WalkForStatement(
    Node* spec_node,
    Node* for_node,
    const SymbolBase* condition_result) {

    // Check for a bool in the condition.
    if (*condition_result != TYPE_BOOL) {
        THROW_EXCEPTION(
            for_node->line(),
            for_node->column(),
            STATUS_SEMANTIC_INVALID_LOOP_CONDITION_TYPE);
    }
}

// Walks a function-like type cast and performs the typecast operation and resolves the types.
// Throws if the typecast is unknown or unsupported.
const SymbolBase* SemanticAstWalker::WalkFunctionLikeTypecast(
    Node* spec_node,
    Node* name_node,
    Node* call_node,
    const SymbolBase* argument_result) {

    // Disallow casting from void function.
    if (*argument_result == TYPE_VOID) {
        THROW_EXCEPTION(
            call_node->line(),
            call_node->column(),
            STATUS_SEMANTIC_VOID_USED_IN_EXPR);
    }

    try {
        // If this fails, user specified invalid function or typecast.
        const TypeSymbol* cast_type_symbol = this->symbol_table_.Get(*name_node->string_value())->type_symbol();

        call_node->set_symbol(cast_type_symbol->Clone());

        const TypeSymbol* result_type_symbol = argument_result->type_symbol();

        // Make the assumption that we support conversions between all non-pointer types.
        // It is up the the LIRGenAstWalker to implement support for the OP codes.
        switch (cast_type_symbol->type_format()) {
        case TypeFormat::FVOID:
            // Disallowed typecast.
            break;
        case TypeFormat::FLOAT:
        case TypeFormat::INT:
            // TODO: put exceptions in here if any.
            // For now, assume all types except string can be cast.
           
            if (result_type_symbol->type_format() != TypeFormat::POINTER) {
                return cast_type_symbol;
            }
            break;
        case TypeFormat::BOOL:
            // Although we support typecasting from INT types to BOOL which is a no-no
            // in other languages (don't care) we aren't going to support casting from
            // floating types because there is too much risk of trouble due to mildly
            // different floating point representations causing near-zero values to
            // be treated as zero and vice versa. If the user wants to cast they must
            // use an if statement.
            if (result_type_symbol->type_format() != TypeFormat::POINTER &&
                result_type_symbol->type_format() != TypeFormat::FLOAT) {
                return cast_type_symbol;
            }
            break;

        default:
            GS_ASSERT_FAIL("Unimplemented cast in WalkFunctionLikeTypecast");
        }
    }
    catch (const Exception& ex) {

        // If the symbol is undefined, this is an invalid function symbol or typecast.
        if (ex.status() == STATUS_SYMBOLTABLE_UNDEFINED_SYMBOL) {
            THROW_EXCEPTION(
                name_node->line(),
                name_node->column(),
                STATUS_SEMANTIC_FUNCTION_OVERLOAD_NOT_FOUND);
        }
    }

    // If we reached this far then the user specified an _unsupported_ typecast.
    THROW_EXCEPTION(
        name_node->line(),
        name_node->column(),
        STATUS_SEMANTIC_UNSUPPORTED_TYPECAST);
}

// Walks an assignment statement or expression and checks to make sure
// that the types match the context in which it was used.
const SymbolBase* SemanticAstWalker::WalkAssign(
    Node* spec_node,
    Node* name_node,
    Node* symbol_node,
    Node* assign_node,
    const SymbolBase* operation_result) {

    // Disallow assigning result of void functions.
    if (*operation_result == TYPE_VOID) {
        THROW_EXCEPTION(
            assign_node->line(),
            assign_node->column(),
            STATUS_SEMANTIC_VOID_USED_IN_EXPR);
    }

    std::string symbol_name = MangleLocalVariableSymbolName(name_node);

    // Try to add the variable symbol to the top level of the symbol table
    // (most recent scope). If the value exists in a lower scope it will
    // be masked by this new definition until the newer scope is popped.
    try {

        SymbolBase* assign_symbol = new FunctionSymbol(
            SymbolType::VARIABLE,
            LexerSymbol::CONCEALED,
            "",
            *name_node->string_value(),
            operation_result);

        this->symbol_table_.Put(
            symbol_name,
            operation_result);
        
        assign_node->set_symbol(assign_symbol);

        // Create a second copy for the other node.
        symbol_node->set_symbol(operation_result->Clone());

        return assign_symbol;
    }
    catch (const Exception& ex) {

        // Throw exception if it's not the expected exception, we don't have handling
        // for this case.
        GS_ASSERT_FALSE(ex.status() != STATUS_SYMBOLTABLE_DUPLICATE_SYMBOL, "Unhandled exception case");

        // Value already exists in the current level of the symbol table
        // so look up the existing value and its type and make sure that the new type
        // matches the existing type.

        const SymbolBase* variable_symbol = this->symbol_table_.Get(symbol_name);
        assign_node->set_symbol(variable_symbol->Clone());
        symbol_node->set_symbol(variable_symbol->Clone());

        // Check to make sure that type of new assignment matches original declared type.
        if (*variable_symbol != *operation_result) {
            THROW_EXCEPTION(
                name_node->line(),
                name_node->column(),
                STATUS_SEMANTIC_TYPE_MISMATCH_IN_ASSIGN);
        }

        return variable_symbol;
    }
}

// Walks and validates a return value type for a function or property.
// TODO: make this work for properties.
const SymbolBase* SemanticAstWalker::WalkReturn(
    Node* spec_node,
    Node* function_node,
    Node* property_node,
    PropertyFunction property_function,
    const SymbolBase** expression_result,
    std::vector<const SymbolBase*>* arguments_result) {

    Node* name_node = NULL;
    std::string symbol_name;

    const SymbolBase* function_symbol;

    // Load the symbol for the respective function or property.
    switch (property_function) {
    case PropertyFunction::NONE: 
        // Determine the symbol name of the function.
        name_node = function_node->child(2);
        symbol_name = MangleFunctionSymbolName(
            spec_node, name_node, *arguments_result);
        function_symbol = this->symbol_table_.Get(symbol_name);
        break;
    case PropertyFunction::GET:
        // Determine the symbol name of the GETTER property function.
        name_node = property_node->child(1);
        symbol_name = ManglePropertyFunctionSymbolName(
            spec_node,
            name_node,
            property_function);
        function_symbol = SYMBOL_TO_PROPERTY(this->symbol_table_.Get(symbol_name));
        break;
    case PropertyFunction::SET:
        // Property setter function cannot return value.
        THROW_EXCEPTION(
            property_node->line(),
            property_node->column(),
            STATUS_SEMANTIC_RETURN_FROM_PROPERTY_SET);
    default:
        // Unhandled switch case.
        THROW_EXCEPTION(
            property_node->line(),
            property_node->column(),
            STATUS_ILLEGAL_STATE);
    }

    // No return expression.
    if (expression_result == NULL) {
        return function_symbol;
    }

    // Check to make sure that the type of the function symbol matches the type
    // of the return statement expression.
    if (*function_symbol->type_symbol() != *(*expression_result)->type_symbol()) {

        // WalkReturn() will have either a property_node or a function_node, not both.
        Node* line_number_node = property_node != NULL ? property_node : function_node;

        // More informative error messages.
        if (*function_symbol->type_symbol() == TYPE_VOID) {
            THROW_EXCEPTION(
                line_number_node->line(),
                line_number_node->column(),
                STATUS_SEMANTIC_RETURN_IN_VOID);
        }
        else if ((**expression_result) == TYPE_VOID) {
            THROW_EXCEPTION(
                line_number_node->line(),
                line_number_node->column(),
                STATUS_SEMANTIC_VOID_USED_IN_EXPR);
        } else {
            THROW_EXCEPTION(
                line_number_node->line(),
                line_number_node->column(),
                STATUS_SEMANTIC_RETURN_TYPE_MISMATCH);
        }
    }

    return function_symbol;
}

// Checks to see if the given module name is valid. If it is not, throws
// an exception.
void SemanticAstWalker::CheckValidModuleName(const std::string& module_name, int line, int column) {
    if (!std::regex_match(module_name, module_name_pattern)) {
        THROW_EXCEPTION(
            line,
            column,
            STATUS_SEMANTIC_INVALID_PACKAGE);
    }
}

// Walks the ADD node and calculates it's return type.
const SymbolBase* SemanticAstWalker::WalkAdd(
    Node* spec_node,
    Node* add_node,
    Node* left_node,
    Node* right_node,
    const SymbolBase* left_result,
    const SymbolBase* right_result) {

    const TypeSymbol* left_symbol = left_result->type_symbol();

    // Only allow numeric and string types.
    if (left_symbol->type_format() != TypeFormat::INT &&
        left_symbol->type_format() != TypeFormat::FLOAT &&
        *left_symbol != TYPE_STRING) {
        THROW_EXCEPTION(
            left_node->line(),
            left_node->column(),
            STATUS_SEMANTIC_INVALID_TYPE_IN_ADD);
    }

    // Alloc a new copy of the symbol because the node frees its own symbol.
    add_node->set_symbol(left_result->Clone());

    return CalculateResultantType(
        left_result,
        right_result,
        left_node->line(),
        right_node->column(),
        STATUS_SEMANTIC_UNMATCHING_TYPE_IN_ADD);
}

// Walks the SUB node and calculates it's return type.
const SymbolBase* SemanticAstWalker::WalkSub(
    Node* spec_node,
    Node* sub_node,
    Node* left_node,
    Node* right_node,
    const SymbolBase* left_result,
    const SymbolBase* right_result) {

    // Alloc a new copy of the symbol because the node frees its own symbol.
    // HACK: unary negative numbers use ANY_TYPE to bypass typechecking, be sure
    // this symbol always pulls from the right side.
    sub_node->set_symbol(right_node->symbol()->Clone());

    return CalculateNumericResultantType(
        left_result,
        right_result,
        left_node->line(),
        left_node->column(),
        STATUS_SEMANTIC_UNMATCHING_TYPE_IN_SUB);
}

// Walks the MUL node and calculates it's return type.
const SymbolBase* SemanticAstWalker::WalkMul(
    Node* spec_node,
    Node* mul_node,
    Node* left_node,
    Node* right_node,
    const SymbolBase* left_result,
    const SymbolBase* right_result) {

    // Alloc a new copy of the symbol because the node frees its own symbol.
    mul_node->set_symbol(left_node->symbol()->Clone());

    return CalculateNumericResultantType(
        left_result,
        right_result,
        left_node->line(),
        left_node->column(),
        STATUS_SEMANTIC_UNMATCHING_TYPE_IN_MUL);
}

// Walks the DIV node and calculates it's return type.
const SymbolBase* SemanticAstWalker::WalkDiv(
    Node* spec_node,
    Node* div_node,
    Node* left_node,
    Node* right_node,
    const SymbolBase* left_result,
    const SymbolBase* right_result) {

    // Alloc a new copy of the symbol because the node frees its own symbol.
    div_node->set_symbol(left_node->symbol()->Clone());

    return CalculateNumericResultantType(
        left_result,
        right_result,
        left_node->line(),
        left_node->column(),
        STATUS_SEMANTIC_UNMATCHING_TYPE_IN_DIV);
}
// Walks the MOD node and calculates it's return type.
const SymbolBase* SemanticAstWalker::WalkMod(
    Node* spec_node,
    Node* mod_node,
    Node* left_node,
    Node* right_node,
    const SymbolBase* left_result,
    const SymbolBase* right_result) {

    // Alloc a new copy of the symbol because the node frees its own symbol.
    mod_node->set_symbol(left_node->symbol()->Clone());

    return CalculateNumericResultantType(
        left_result,
        right_result,
        left_node->line(),
        left_node->column(),
        STATUS_SEMANTIC_UNMATCHING_TYPE_IN_MOD);
}

// Walks the LOGNOT node and calculates it's return type.
const SymbolBase* SemanticAstWalker::WalkLogNot(
    Node* spec_node,
    Node* log_not_node,
    Node* child_node,
    const SymbolBase* child_result) {

    // Check for boolean type. NOT works only with booleans.
    if (*child_result->type_symbol() != TYPE_BOOL) {
        THROW_EXCEPTION(
            child_node->line(),
            child_node->column(),
            STATUS_SEMANTIC_NONBOOL_IN_LOGNOT);
    }

    // Alloc a new copy of the symbol because the node frees its own symbol.
    log_not_node->set_symbol(child_node->symbol()->Clone());

    return child_result;
}

// Walks the LOGAND node and calculates it's return type.
const SymbolBase* SemanticAstWalker::WalkLogAnd(
    Node* spec_node,
    Node* log_and_node,
    Node* left_node,
    Node* right_node,
    const SymbolBase* left_result,
    const SymbolBase* right_result) {

    // Alloc a new copy of the symbol because the node frees its own symbol.
    log_and_node->set_symbol(left_node->symbol()->Clone());

    return CalculateBoolResultantType(
        left_result,
        right_result,
        left_node->line(),
        left_node->column(),
        STATUS_SEMANTIC_UNMATCHING_TYPE_IN_LOGAND);
}

// Walks the LOGOR node and calculates it's return type.
const SymbolBase* SemanticAstWalker::WalkLogOr(
    Node* spec_node,
    Node* log_or_node,
    Node* left_node,
    Node* right_node,
    const SymbolBase* left_result,
    const SymbolBase* right_result) {

    // Alloc a new copy of the symbol because the node frees its own symbol.
    log_or_node->set_symbol(left_node->symbol()->Clone());

    return CalculateBoolResultantType(
        left_result,
        right_result,
        left_node->line(),
        left_node->column(),
        STATUS_SEMANTIC_UNMATCHING_TYPE_IN_LOGOR);
}

// Walks the GREATER node and calculates it's return type.
const SymbolBase* SemanticAstWalker::WalkGreater(
    Node* spec_node,
    Node* greater_node,
    Node* left_node,
    Node* right_node,
    const SymbolBase* left_result,
    const SymbolBase* right_result) {

    // Check that comparision types are the same.
    // Function will throw if different.
    CalculateNumericResultantType(
        left_result,
        right_result,
        left_node->line(),
        left_node->column(),
        STATUS_SEMANTIC_UNMATCHING_TYPE_IN_GREATER);

    // Alloc a new copy of the symbol because the node frees its own symbol.
    greater_node->set_symbol(left_node->symbol()->Clone());

    // Resultant type is a TYPE_BOOL telling whether comparision is true or false.
    return &TYPE_BOOL;
}

// Walks the EQUALS node and calculates it's return type.
const SymbolBase* SemanticAstWalker::WalkEquals(
    Node* spec_node,
    Node* equals_node,
    Node* left_node,
    Node* right_node,
    const SymbolBase* left_result,
    const SymbolBase* right_result) {

    // Check that comparision types are the same.
    // Function will throw if different.
    CalculateResultantType(
        left_result,
        right_result,
        left_node->line(),
        left_node->column(),
        STATUS_SEMANTIC_UNMATCHING_TYPE_IN_EQUALS);

    // Alloc a new copy of the symbol because the node frees its own symbol.
    equals_node->set_symbol(left_node->symbol()->Clone());

    // Resultant type is a TYPE_BOOL telling whether comparision is true or false.
    return &TYPE_BOOL;
}

// Walks the NOT_EQUALS node and calculates it's return type.
const SymbolBase* SemanticAstWalker::WalkNotEquals(
    Node* spec_node,
    Node* not_equals_node,
    Node* left_node,
    Node* right_node,
    const SymbolBase* left_result,
    const SymbolBase* right_result) {

    // Check that comparision types are the same.
    // Function will throw if different.
    CalculateResultantType(
        left_result,
        right_result,
        left_node->line(),
        left_node->column(),
        STATUS_SEMANTIC_UNMATCHING_TYPE_IN_NOT_EQUALS);

    // Alloc a new copy of the symbol because the node frees its own symbol.
    not_equals_node->set_symbol(left_node->symbol()->Clone());

    // Resultant type is a TYPE_BOOL telling whether comparision is true or false.
    return &TYPE_BOOL;
}

// Walks the LESS node and calculates it's return type.
const SymbolBase* SemanticAstWalker::WalkLess(
    Node* spec_node,
    Node* less_node,
    Node* left_node,
    Node* right_node,
    const SymbolBase* left_result,
    const SymbolBase* right_result) {

    // Check that comparision types are the same.
    // Function will throw if different.
    CalculateNumericResultantType(
        left_result,
        right_result,
        left_node->line(),
        left_node->column(),
        STATUS_SEMANTIC_UNMATCHING_TYPE_IN_LESS);

    // Alloc a new copy of the symbol because the node frees its own symbol.
    less_node->set_symbol(left_node->symbol()->Clone());

    // Resultant type is a TYPE_BOOL telling whether comparision is true or false.
    return &TYPE_BOOL;
}

// Walks the GREATER_EQUALS node and calculates it's return type.
const SymbolBase* SemanticAstWalker::WalkGreaterEquals(
    Node* spec_node,
    Node* greater_equals_node,
    Node* left_node,
    Node* right_node,
    const SymbolBase* left_result,
    const SymbolBase* right_result) {

    // Check that comparision types are the same.
    // Function will throw if different.
    CalculateNumericResultantType(
        left_result,
        right_result,
        left_node->line(),
        left_node->column(),
        STATUS_SEMANTIC_UNMATCHING_TYPE_IN_GREATER_EQUALS);


    // Alloc a new copy of the symbol because the node frees its own symbol.
    greater_equals_node->set_symbol(left_node->symbol()->Clone());

    // Resultant type is a TYPE_BOOL telling whether comparision is true or false.
    return &TYPE_BOOL;
}

// Walks the LESS_EQUALS node and calculates it's return type.
const SymbolBase* SemanticAstWalker::WalkLessEquals(
    Node* spec_node,
    Node* less_equals_node,
    Node* left_node,
    Node* right_node,
    const SymbolBase* left_result,
    const SymbolBase* right_result) {

    // Check that comparision types are the same.
    // Function will throw if different.
    CalculateNumericResultantType(
        left_result,
        right_result,
        left_node->line(),
        left_node->column(),
        STATUS_SEMANTIC_UNMATCHING_TYPE_IN_LESS_EQUALS);


    // Alloc a new copy of the symbol because the node frees its own symbol.
    less_equals_node->set_symbol(left_node->symbol()->Clone());

    // Resultant type is a TYPE_BOOL telling whether comparision is true or false.
    return &TYPE_BOOL;
}

// Walks the TYPE_BOOL node and returns the type for it.
const SymbolBase* SemanticAstWalker::WalkBool(
    Node* spec_node,
    Node* function_node,
    Node* property_node,
    PropertyFunction property_function,
    Node* bool_node) {

    // Alloc copy because ~Node() frees it's symbol on destroy.
    bool_node->set_symbol(TYPE_BOOL.Clone());
    return &TYPE_BOOL;
}

// Walks the TYPE_INT node and returns the type for it.
const SymbolBase* SemanticAstWalker::WalkInt(
    Node* spec_node,
    Node* function_node,
    Node* property_node,
    PropertyFunction property_function,
    Node* int_node) {

    // Alloc copy because ~Node() frees it's symbol on destroy.
    int_node->set_symbol(TYPE_INT.Clone());
    return &TYPE_INT;
}

// Walks the FLOAT node and returns the type for it.
const SymbolBase* SemanticAstWalker::WalkFloat(
    Node* spec_node,
    Node* function_node,
    Node* property_node,
    PropertyFunction property_function,
    Node* float_node) {

    float_node->set_symbol(TYPE_FLOAT.Clone());
    return &TYPE_FLOAT;
}

// Walks the STRING node and returns the type for it.
const SymbolBase* SemanticAstWalker::WalkString(
    Node* spec_node,
    Node* function_node,
    Node* property_node,
    PropertyFunction property_function,
    Node* string_node) {

    // Alloc copy because ~Node() destroys the symbol when done.
    string_node->set_symbol(TYPE_STRING.Clone());
    return &TYPE_STRING;
}

// Walks the CHAR node and returns the type for it.
const SymbolBase* SemanticAstWalker::WalkChar(
    Node* spec_node,
    Node* function_node,
    Node* property_node,
    PropertyFunction property_function,
    Node* char_node) {

    // Store the symbol in the char node:
    // Alloc copy because ~Node() destroys the symbol when done.
    char_node->set_symbol(TYPE_INT.Clone());

    return &TYPE_INT8;
}

// Walks the SYMBOL->NAME subtree that represents a variable reference
// and returns the type for it.
const SymbolBase* SemanticAstWalker::WalkVariable(
    Node* spec_node,
    Node* function_node,
    Node* property_node,
    PropertyFunction property_function,
    Node* variable_node,
    Node* name_node) {

    const std::string symbol_name = MangleLocalVariableSymbolName(name_node);

    try {
        const SymbolBase* symbol = this->symbol_table_.Get(symbol_name);

        // Store the symbol in the variable node:
        // Alloc copy because ~Node() destroys the symbol when done.
        variable_node->set_symbol(symbol->Clone());

        // Looks up the variable in the SymbolTable and returns its type.
        // Throws if the symbol is undefined.
        return symbol;
    }
    catch (const Exception& ex) {
        if (ex.status() != STATUS_SYMBOLTABLE_UNDEFINED_SYMBOL) {
            throw;
        }

        // Throw more relevant exception.
        THROW_EXCEPTION(
            variable_node->column(),
            variable_node->line(),
            STATUS_SEMANTIC_UNDEFINED_VARIABLE);
    }
}

// Walks the ANY_TYPE node and returns the type for it.
const SymbolBase* SemanticAstWalker::WalkAnyType(
    Node* spec_node,
    Node* function_node,
    Node* property_node,
    PropertyFunction property_function,
    Node* any_type_node) {

    return &TYPE_NONE;
}

// Compares the access modifier of the member and the calling function's
// class names to prevent access to private members.
void SemanticAstWalker::CheckAccessModifier(
    const std::string& caller_class,
    const std::string& callee_class,
    LexerSymbol callee_access_modifier,
    int line,
    int column) {

    switch (callee_access_modifier) {
    case LexerSymbol::PUBLIC:
        return;
    case LexerSymbol::CONCEALED:
        if (caller_class != callee_class) {
            THROW_EXCEPTION(
                line,
                column,
                STATUS_SEMANTIC_NOT_ACCESSIBLE);
        }
        break;
    case LexerSymbol::PACKAGE:
        // What exactly a 'package' will be is currently up in the air.
        // TODO: complete this.
        THROW_EXCEPTION(
            line,
            column,
            STATUS_ILLEGAL_STATE);
        break;
    case LexerSymbol::INTERNAL:
        // What exactly 'internal' means is currently up in the air.
        // Internal is TYPE_INTENDED to mean that it is internal to the file,
        // but there isn't support for multifile lex/parse/typecheck yet.
        // TODO: complete this.
        THROW_EXCEPTION(
            line,
            column,
            STATUS_ILLEGAL_STATE);
    default:
        // Throw If someone adds a new access modifier that we don't know of.
        GS_ASSERT_FAIL("Unimplemented access modifier.");
    }
}

// Optional implemented function that overrides base class implementation.
// In SemanticAstWalker, this function overrides default action for walking
// a spec and pushes another layer to the symbol_table_ to allow for declaration
// of types and fields private to a spec, namely, Generic params.
void SemanticAstWalker::WalkSpec(Node* spec_node, PrescanMode scan_mode) {
    this->symbol_table_.Push();

    // Call parent class implementation.
    AstWalker::WalkSpec(spec_node, scan_mode);

    this->symbol_table_.Pop();
}

// Optional implemented function that overrides base class implementation.
// In SemanticAstWalker, this function pushes a new table to the SymbolTable
// to introduce new context for each FUNCTION entered, limiting the
// scope of function arguments.
void SemanticAstWalker::WalkFunctionChildren(
    Node* spec_node,
    Node* function_node,
    bool prescan) {

    // Push new scope.
    this->symbol_table_.Push();

    // Walk the Node via parent class.
    AstWalker::WalkFunctionChildren(spec_node, function_node, prescan);

    // Pop the scope.
    this->symbol_table_.Pop();
}

// Optional implemented function that overrides base class implementation.
// In SemanticAstWalker, this function pushes a new table to the SymbolTable
// to introduce new context for each BLOCK ('{' to '}') entered, limiting the
// scope of block variables.
void SemanticAstWalker::WalkBlockChildren(
    Node* spec_node,
    Node* function_node,
    Node* property_node,
    PropertyFunction property_function,
    Node* block,
    std::vector<const SymbolBase*>* arguments_result) {

    // Push new scope.
    this->symbol_table_.Push();
    
    // Walk the Block.
    AstWalker::WalkBlockChildren(
        spec_node,
        function_node,
        property_node,
        property_function,
        block,
        arguments_result);

    // Pop the scope.
    this->symbol_table_.Pop();
}

// Optional implemented function that overrides base class implementation.
// In SemanticAstWalker, this function  assigns a type to the EXPRESSION node.
const SymbolBase* SemanticAstWalker::WalkExpressionChildren(
    Node* spec_node,
    Node* function_node,
    Node* property_node,
    PropertyFunction property_function,
    Node* expression_node) {

    const SymbolBase* expression_type = AstWalker::WalkExpressionChildren(
        spec_node,
        function_node,
        property_node,
        property_function,
        expression_node);

    // TODO: cleaner.
    expression_node->set_symbol(expression_node->child(0)->symbol()->Clone());

    return expression_type;
}

// Walks a new Spec(arg1, arg2, ...) expression.
const SymbolBase* SemanticAstWalker::WalkNewExpression(
    Node* new_node,
    Node* type_node,
    std::vector<const SymbolBase*>& arguments_result) {

    // Lookup type of new statement.
    const SymbolBase* type_symbol = ResolveTypeNode(type_node);
    new_node->set_symbol(type_symbol->Clone());

    // Lookup constructor function.
    const std::string constructor_name = MangleFunctionSymbolName(
        MangleSpecTemplateSymbolName(type_node),
        kConstructorName,
        arguments_result);

    const SymbolBase* constructor_symbol = NULL;
    try {
        constructor_symbol = this->symbol_table_.Get(constructor_name);
    }
    catch (const Exception& ex) {

        // The symbol for this function is unknown. Either a typo or a function-like typecast.
        if (ex.status() == STATUS_SYMBOLTABLE_UNDEFINED_SYMBOL) {
            THROW_EXCEPTION(
                type_node->line(),
                type_node->column(),
                STATUS_SEMANTIC_CONSTRUCTOR_OVERLOAD_NOT_FOUND);
        }

        throw;
    }

    return type_symbol;
}

// Walks a new default(type) expression.
const SymbolBase* SemanticAstWalker::WalkDefaultExpression(
    Node* default_node,
    Node* type_node) {

    const SymbolBase* type_symbol = ResolveTypeNode(type_node);

    default_node->set_symbol(type_symbol->Clone());

    return type_symbol;
}

// Calculates the type of a binary operator expression from the types of its
// operands.
const SymbolBase* SemanticAstWalker::CalculateResultantType(
    const SymbolBase* left,
    const SymbolBase* right,
    int line, 
    int column,
    ExceptionStatus type_mismatch_error) {

    // We're going to take a stickler model in Gunderscript:
    // you must explicitly typecast everything. There are absolutely no
    // auto conversions between types.
    if (*left->type_symbol() == *right->type_symbol()) {
        return right;
    }

    // Accept TYPE_NONE nodes as matches for any given type.
    if (*left == TYPE_NONE) {
        return right;
    }
    if (*right == TYPE_NONE) {
        return left;
    }

    if (*left == TYPE_VOID || *right == TYPE_VOID) {
        // Void function call in expression.
        THROW_EXCEPTION(
            line, column,
            STATUS_SEMANTIC_VOID_USED_IN_EXPR);
    }
    else {
        // Types don't match. Might require a typecast. (e.g.: float to int).
        THROW_EXCEPTION(
            line,
            column,
            type_mismatch_error);
    }

    
}

// Calculates the type of a binary operator expression from the types of its
// operands. Operands must both be numeric.
const SymbolBase* SemanticAstWalker::CalculateNumericResultantType(
    const SymbolBase* left,
    const SymbolBase* right,
    int line,
    int column,
    ExceptionStatus type_mismatch_error) {

    // Do other checks.
    const TypeSymbol* resultant_type
        = CalculateResultantType(left, right, line, column, type_mismatch_error)->type_symbol();

    // Disallow non-numerical operands.
    if (resultant_type->type_format() != TypeFormat::INT &&
        resultant_type->type_format() != TypeFormat::FLOAT) {
        THROW_EXCEPTION(
            line,
            column,
            STATUS_SEMANTIC_NONNUMERIC_OPERANDS);
    }

    return resultant_type;
}

// Calculates the type of a binary operator expression from the types of its
// operands. Operands must both be numeric.
const SymbolBase* SemanticAstWalker::CalculateBoolResultantType(
    const SymbolBase* left,
    const SymbolBase* right,
    int line,
    int column,
    ExceptionStatus type_mismatch_error) {

    // Both operands must be TYPE_BOOL.
    if (*left->type_symbol() != TYPE_BOOL || *right->type_symbol() != TYPE_BOOL) {
        THROW_EXCEPTION(
            line,
            column,
            STATUS_SEMANTIC_NONBOOL_OPERANDS);
    }

    return right;
}

// Looks up a type node's name in the symbol table and returns its Symbol.
// This is also where resolution of GENERIC_TYPE_TEMPLATEs into GENERIC_TYPEs 
// happens.
const SymbolBase* SemanticAstWalker::ResolveTypeNode(Node* type_node) {

    const SymbolBase* type_symbol;

    // Look up symbol in symbol table.
    try {
        type_symbol = this->symbol_table_.Get(MangleSpecTemplateSymbolName(type_node));
    }
    catch (const Exception& ex) {
        // Rethrow as more relevant exception.
        if (ex.status() == STATUS_SYMBOLTABLE_UNDEFINED_SYMBOL) {
            THROW_EXCEPTION(
                type_node->line(),
                type_node->column(),
                STATUS_SEMANTIC_UNDEFINED_TYPE);
        }

        throw;
    }

    // If this is not a generic type, return the raw type.
    if (type_symbol->symbol_type() != SymbolType::GENERIC_TYPE_TEMPLATE) {
        type_node->set_symbol(type_symbol->Clone());
        return type_symbol;
    }
    // else: this is a generic type, continue on to check the type params.

    std::ostringstream name_buf;
    name_buf << *type_node->string_value() << '<';

    // Check that all type params are valid types.
    std::vector<const SymbolBase*> type_params;
    for (size_t i = 0; i < type_node->child_count(); i++) {
        const SymbolBase* param = ResolveTypeNode(type_node->child(i));
        type_params.push_back(param);
        name_buf << param->symbol_name() << ',';
    }

    name_buf << '>';

    // Construct type from the generic type template with the concrete
    // type params in this usage.
    const SymbolBase* generic_type_symbol = new GenericTypeSymbol(
        SymbolType::GENERIC_TYPE,
        type_symbol->access_modifier(),
        name_buf.str(),
        type_params);

    type_node->set_symbol(generic_type_symbol);

    return generic_type_symbol;
}

} // namespace compiler
} // namespace gunderscript
