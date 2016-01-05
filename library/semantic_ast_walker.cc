// Gunderscript-2 Semantic (type) Checker for Abstract Syntax Tree
// (C) 2015 Christian Gunderman

#include <regex>
#include <sstream>

#include "lexer.h"
#include "semantic_ast_walker.h"

namespace gunderscript {
namespace library {

// The pattern for checking module names.
const std::regex module_name_pattern = std::regex("^([A-Z]|[a-z])+(\\.([A-Z]|[a-z])+)?$");

// Mangles function symbol name to include class name and arguments so that
// symbol table guarantees uniqueness for functions while allowing overloads
// with different argument types.
static const std::string MangleFunctionSymbolName(
    Node* spec_node, 
    Node* name_node,
    std::vector<Type>& arguments_result) {

    Node* spec_name_node = spec_node->child(1);

    // Format the symbol name {class}::{function}$arg1$arg2...
    std::ostringstream name_buf;
    name_buf << *spec_name_node->string_value();
    name_buf << "::";
    name_buf << *name_node->string_value();

    // Append arguments to the symbol name.
    for (size_t i = 0; i < arguments_result.size(); i++) {
        name_buf << "$";
        name_buf << arguments_result[i].symbol_name();
    }

    return name_buf.str();
}

// Mangles local variable symbol name to the format Local%%{variable}
// so that they do not collide with class names in the symbol table.
static const std::string MangleLocalVariableSymbolName(Node* name_node) {
    std::ostringstream name_buf;
    name_buf << "Local%%";
    name_buf << *name_node->string_value();

    return name_buf.str();
}

// Gets the symbol name 
static const std::string ManglePropertyFunctionSymbolName(
    Node* spec_node,
    Node* name_node,
    PropertyFunction function) {

    Node* spec_name_node = spec_node->child(1);

    // Format the getter symbol name {class}<-{function}
    std::ostringstream name_buf;
    name_buf << *spec_name_node->string_value();
    name_buf << (function == PropertyFunction::SET ? "->" : "<-");
    name_buf << *name_node->string_value();

    return name_buf.str();
}

// Constructor, populates symbol table with Types.
SemanticAstWalker::SemanticAstWalker(Node& node) : AstWalker(node), symbol_table_() {

    // Add all default types to the Symbol table.
    for (unsigned int i = 0; i < TYPES.size(); i++) {
        const Type& current_type = TYPES[i];

        // Create a symbol for the Type.
        // Extraneous fields are left empty or filled with defaults.
        Symbol type_symbol(
            LexerSymbol::PUBLIC,
            false,
            current_type,
            current_type.symbol_name(),
            std::string());

        this->symbol_table_.PutBottom(current_type.symbol_name(), type_symbol);
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
    CheckValidModuleName(*name_node->string_value());
}

// Checks that the given dependency module name is valid.
// If not, throws an exception.
// TODO: calculate dependency graph and lex/parse/typecheck the
// dependency first.
void SemanticAstWalker::WalkModuleDependsName(Node* name_node) {
    CheckValidModuleName(*name_node->string_value());
}

// Attempts to declare a new spec in the given scope. Throws if spec
// name is taken in this context.
void SemanticAstWalker::WalkSpecDeclaration(Node* access_modifier_node, Node* name_node) {
    // Construct symbol with extranous fields filled in arbitrarily.
    Symbol spec_symbol(
        access_modifier_node->symbol_value(),
        false,
        NONE,
        std::string(),
        *name_node->string_value());

    this->symbol_table_.PutBottom(*name_node->string_value(), spec_symbol);
}

// Walks a single function declaration inside of a SPEC.
// Throws if the function already exists in the symbol table.
void SemanticAstWalker::WalkSpecFunctionDeclaration(
    Node* spec_node,
    Node* access_modifier_node,
    Node* native_node,
    Node* type_node,
    Node* name_node,
    Node* block_node,
    std::vector<Type>& arguments_result,
    bool prescan) {

    Node* spec_name_node = spec_node->child(1);

    Symbol function_symbol(
        access_modifier_node->symbol_value(),
        native_node->bool_value(),
        ResolveTypeNode(type_node),
        *spec_name_node->string_value(),
        *name_node->string_value());

    // Only define the symbol during the prescan portion of the walking process.
    // The prescan allows for us to iterate and define symbols for all functions
    // before we type check the function body so functions can reference one another
    // regardless of the order in which they are declared.
    if (prescan) {
        this->symbol_table_.PutBottom(
            MangleFunctionSymbolName(spec_node, name_node, arguments_result),
            function_symbol);
    }
}

// Walks a single parameter in a spec function declaration.
// Returns it to the Function Declaration walker.
Type SemanticAstWalker::WalkSpecFunctionDeclarationParameter(
    Node* spec_node,
    Node* function_node,
    Node* type_node,
    Node* name_node,
    bool prescan) {

    Node* spec_name_node = spec_node->child(1);

    // Create a new symbol for the variable.
    // Extraneous values are arbitrary.
    Symbol variable_symbol(
        LexerSymbol::CONCEALED,
        false,
        ResolveTypeNode(type_node),
        *spec_name_node->string_value(),
        *name_node->string_value());

    // Insert the symbol into the table.
    this->symbol_table_.Put(
        MangleLocalVariableSymbolName(name_node),
        variable_symbol);

    // Return the type.
    return ResolveTypeNode(type_node);
}

// Walks a single property in a spec property declaration.
// Defines it in the symbol table.
void SemanticAstWalker::WalkSpecPropertyDeclaration(
    Node* spec_node,
    Node* type_node,
    Node* name_node,
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

    Node* spec_name_node = spec_node->child(1);

    // Determine the getter function symbol name.
    std::string get_function_symbol_name
        = ManglePropertyFunctionSymbolName(spec_node, name_node, PropertyFunction::GET);

    // Create the symbol table symbol for the getter.
    Symbol get_function_symbol(
        get_access_modifier_node->symbol_value(),
        false,
        ResolveTypeNode(type_node),
        *spec_name_node->string_value(),
        *name_node->string_value());

    // Define the getter symbol.
    this->symbol_table_.PutBottom(get_function_symbol_name, get_function_symbol);

    // Determine the setter function symbol name.
    std::string set_function_symbol_name
        = ManglePropertyFunctionSymbolName(spec_node, name_node, PropertyFunction::SET);

    // Create the symbol table symbol for the getter.
    Symbol set_function_symbol(
        set_access_modifier_node->symbol_value(),
        false,
        ResolveTypeNode(type_node),
        *spec_name_node->string_value(),
        *name_node->string_value());

    // Define the setter symbol.
    this->symbol_table_.PutBottom(set_function_symbol_name, set_function_symbol);
}

// Walks a function call and checks to make sure that the types
// of the function matches the context.
Type SemanticAstWalker::WalkFunctionCall(
    Node* spec_node,
    Node* name_node,
    std::vector<Type>& arguments_result) {

    Node* spec_name_node = spec_node->child(1);

    // Lookup the function. Throws if there isn't a function with the correct arguments.
    const Symbol& symbol = this->symbol_table_.Get(
        MangleFunctionSymbolName(spec_node, name_node, arguments_result));

    // Check for access to the callee function.
    // TODO: as of the moment this does nothing because we don't support calls between classes
    // yet. Implement calls between classes.
    CheckAccessModifier(
        *spec_name_node->string_value(),
        symbol.spec_name(),
        symbol.access_modifier());

    return symbol.type();
}

// Walks an assignment statement or expression and checks to make sure
// that the types match the context in which it was used.
Type SemanticAstWalker::WalkAssign(
    Node* spec_node,
    Node* name_node,
    Type operations_result) {

    std::string symbol_name = MangleLocalVariableSymbolName(name_node);

    // Try to add the variable symbol to the top level of the symbol table
    // (most recent scope). If the value exists in a lower scope it will
    // be masked by this new definition until the newer scope is popped.
    try {
        this->symbol_table_.Put(
            symbol_name,
            Symbol(
                LexerSymbol::CONCEALED,
                false,
                operations_result,
                std::string(),
                *name_node->string_value()));

        return operations_result;
    }
    catch (const SymbolTableDuplicateKeyException&) {
        // Value already exists in the current level of the symbol table
        // so look up the existing value and its type and make sure that the new type
        // matches the existing type.

        const Symbol& variable_symbol = this->symbol_table_.Get(symbol_name);

        // Check to make sure that type of new assignment matches original declared type.
        if (variable_symbol.type() != operations_result) {
            throw SemanticAstWalkerTypeMismatchException(*this);
        }

        return variable_symbol.type();
    }
}

// Walks and validates a return value type for a function or property.
// TODO: make this work for properties.
Type SemanticAstWalker::WalkReturn(
    Node* spec_node,
    Node* function_node,
    Node* property_node,
    PropertyFunction property_function,
    Type expression_result,
    std::vector<Type>* arguments_result) {

    Node* name_node = NULL;
    std::string symbol_name;

    // Load the symbol for the respective function or property.
    switch (property_function) {
    case PropertyFunction::NONE: 
        // Determine the symbol name of the function.
        name_node = function_node->child(3);
        symbol_name = MangleFunctionSymbolName(
            spec_node, name_node, *arguments_result);
        break;
    case PropertyFunction::GET:
        // Determine the symbol name of the GETTER property function.
        name_node = property_node->child(1);
        symbol_name = ManglePropertyFunctionSymbolName(
            spec_node,
            name_node,
            property_function);
        break;
    case PropertyFunction::SET:
        // Property setter function cannot return value.
        throw SemanticAstWalkerTypeMismatchException(*this);
    default:
        // Unhandled switch case.
        throw new IllegalStateException();
    }

    // Lookup the function symbol. Shouldn't be able to throw since the
    // caller of this method is the function containing the statement.
    const Symbol& symbol = this->symbol_table_.Get(symbol_name);

    // Check to make sure that the type of the function symbol matches the type
    // of the return statement expression.
    if (symbol.type() != expression_result) {
        throw SemanticAstWalkerTypeMismatchException(*this);
    }

    return symbol.type();
}

// Checks to see if the given module name is valid. If it is not, throws
// an exception.
void SemanticAstWalker::CheckValidModuleName(const std::string& module_name) {
    if (!std::regex_match(module_name, module_name_pattern)) {
        throw SemanticAstWalkerInvalidPackageNameException(*this);
    }
}

// Walks the ADD node and calculates it's return type.
Type SemanticAstWalker::WalkAdd(
    Node* spec_node,
    Node* left_node,
    Node* right_node,
    Type left_result,
    Type right_result) {

    return CalculateResultantType(left_result, right_result);
}

// Walks the SUB node and calculates it's return type.
Type SemanticAstWalker::WalkSub(
    Node* spec_node,
    Node* left_node,
    Node* right_node,
    Type left_result,
    Type right_result) {

    return CalculateNumericResultantType(left_result, right_result);
}

// Walks the MUL node and calculates it's return type.
Type SemanticAstWalker::WalkMul(
    Node* spec_node,
    Node* left_node,
    Node* right_node,
    Type left_result,
    Type right_result) {

    return CalculateNumericResultantType(left_result, right_result);
}

// Walks the DIV node and calculates it's return type.
Type SemanticAstWalker::WalkDiv(
    Node* spec_node,
    Node* left_node,
    Node* right_node,
    Type left_result,
    Type right_result) {

    return CalculateNumericResultantType(left_result, right_result);
}
// Walks the MOD node and calculates it's return type.
Type SemanticAstWalker::WalkMod(
    Node* spec_node,
    Node* left_node,
    Node* right_node,
    Type left_result,
    Type right_result) {

    return CalculateNumericResultantType(left_result, right_result);
}

// Walks the LOGNOT node and calculates it's return type.
Type SemanticAstWalker::WalkLogNot(
    Node* spec_node,
    Node* child_node,
    Type child_result) {

    // Check for boolean type. NOT works only with booleans.
    if (child_result != BOOL) {
        throw SemanticAstWalkerTypeMismatchException(*this);
    }

    return child_result;
}

// Walks the LOGAND node and calculates it's return type.
Type SemanticAstWalker::WalkLogAnd(
    Node* spec_node,
    Node* left_node,
    Node* right_node,
    Type left_result,
    Type right_result) {

    return CalculateBoolResultantType(left_result, right_result);
}

// Walks the LOGOR node and calculates it's return type.
Type SemanticAstWalker::WalkLogOr(
    Node* spec_node,
    Node* left_node,
    Node* right_node,
    Type left_result,
    Type right_result) {

    return CalculateBoolResultantType(left_result, right_result);
}

// Walks the GREATER node and calculates it's return type.
Type SemanticAstWalker::WalkGreater(
    Node* spec_node,
    Node* left_node,
    Node* right_node,
    Type left_result,
    Type right_result) {

    // Check that comparision types are the same.
    // Function will throw if different.
    CalculateNumericResultantType(left_result, right_result);

    // Resultant type is a BOOL telling whether comparision is true or false.
    return BOOL;
}

// Walks the EQUALS node and calculates it's return type.
Type SemanticAstWalker::WalkEquals(
    Node* spec_node,
    Node* left_node,
    Node* right_node,
    Type left_result,
    Type right_result) {

    // Check that comparision types are the same.
    // Function will throw if different.
    CalculateResultantType(left_result, right_result);

    // Resultant type is a BOOL telling whether comparision is true or false.
    return BOOL;
}

// Walks the NOT_EQUALS node and calculates it's return type.
Type SemanticAstWalker::WalkNotEquals(
    Node* spec_node,
    Node* left_node,
    Node* right_node,
    Type left_result,
    Type right_result) {

    // Check that comparision types are the same.
    // Function will throw if different.
    CalculateResultantType(left_result, right_result);

    // Resultant type is a BOOL telling whether comparision is true or false.
    return BOOL;
}

// Walks the LESS node and calculates it's return type.
Type SemanticAstWalker::WalkLess(
    Node* spec_node,
    Node* left_node,
    Node* right_node,
    Type left_result,
    Type right_result) {

    // Check that comparision types are the same.
    // Function will throw if different.
    CalculateNumericResultantType(left_result, right_result);

    // Resultant type is a BOOL telling whether comparision is true or false.
    return BOOL;
}

// Walks the GREATER_EQUALS node and calculates it's return type.
Type SemanticAstWalker::WalkGreaterEquals(
    Node* spec_node,
    Node* left_node,
    Node* right_node,
    Type left_result,
    Type right_result) {

    // Check that comparision types are the same.
    // Function will throw if different.
    CalculateNumericResultantType(left_result, right_result);

    // Resultant type is a BOOL telling whether comparision is true or false.
    return BOOL;
}

// Walks the LESS_EQUALS node and calculates it's return type.
Type SemanticAstWalker::WalkLessEquals(
    Node* spec_node,
    Node* left_node,
    Node* right_node,
    Type left_result,
    Type right_result) {

    // Check that comparision types are the same.
    // Function will throw if different.
    CalculateNumericResultantType(left_result, right_result);

    // Resultant type is a BOOL telling whether comparision is true or false.
    return BOOL;
}

// Walks the BOOL node and returns the type for it.
Type SemanticAstWalker::WalkBool(
    Node* spec_node,
    Node* function_node,
    Node* property_node,
    PropertyFunction property_function,
    Node* bool_node) {

    return BOOL;
}

// Walks the INT node and returns the type for it.
Type SemanticAstWalker::WalkInt(
    Node* spec_node,
    Node* function_node,
    Node* property_node,
    PropertyFunction property_function,
    Node* int_node) {

    return INT;
}

// Walks the FLOAT node and returns the type for it.
Type SemanticAstWalker::WalkFloat(
    Node* spec_node,
    Node* function_node,
    Node* property_node,
    PropertyFunction property_function,
    Node* float_node) {

    return FLOAT;
}

// Walks the STRING node and returns the type for it.
Type SemanticAstWalker::WalkString(
    Node* spec_node,
    Node* function_node,
    Node* property_node,
    PropertyFunction property_function,
    Node* string_node) {

    return STRING;
}

// Walks the CHAR node and returns the type for it.
Type SemanticAstWalker::WalkChar(
    Node* spec_node,
    Node* function_node,
    Node* property_node,
    PropertyFunction property_function,
    Node* char_node) {

    return CHAR;
}

// Walks the SYMBOL->NAME subtree that represents a variable reference
// and returns the type for it.
Type SemanticAstWalker::WalkVariable(
    Node* spec_node,
    Node* function_node,
    Node* property_node,
    PropertyFunction property_function,
    Node* name_node) {

    // Looks up the variable in the SymbolTable and returns its type.
    // Throws if the symbol is undefined.
    const Symbol& symbol = this->symbol_table_.Get(MangleLocalVariableSymbolName(name_node));
    return symbol.type();
}

// Walks the ANY_TYPE node and returns the type for it.
Type SemanticAstWalker::WalkAnyType(
    Node* spec_node,
    Node* function_node,
    Node* property_node,
    PropertyFunction property_function,
    Node* any_type_node) {

    return NONE;
}

// Compares the access modifier of the member and the calling function's
// class names to prevent access to private members.
void SemanticAstWalker::CheckAccessModifier(
    const std::string& caller_class,
    const std::string& callee_class,
    LexerSymbol callee_access_modifier) {

    switch (callee_access_modifier) {
    case LexerSymbol::PUBLIC:
        return;
    case LexerSymbol::CONCEALED:
        if (caller_class != callee_class) {
            throw SemanticAstWalkerNotAccessibleException(*this);
        }
    case LexerSymbol::PACKAGE:
        // What exactly a 'package' will be is currently up in the air.
        // TODO: complete this.
        throw new NotImplementedException();
    case LexerSymbol::INTERNAL:
        // What exactly 'internal' means is currently up in the air.
        // Internal is INTENDED to mean that it is internal to the file,
        // but there isn't support for multifile lex/parse/typecheck yet.
        // TODO: complete this.
        throw new NotImplementedException();
    default:
        // Throw If someone adds a new access modifier that we don't know of.
        throw IllegalStateException();
    }
}

// Optional implemented function that overrides base class implementation.
// In SemanticAstWalker, this function pushes a new table to the SymbolTable
// to introduce new context for each FUNCTION entered, limiting the
// scope of function arguments.
void SemanticAstWalker::WalkSpecFunctionChildren(
    Node* spec_node,
    Node* function_node,
    bool prescan) {

    // Push new scope.
    this->symbol_table_.Push();

    // Walk the Node via parent class.
    AstWalker::WalkSpecFunctionChildren(spec_node, function_node, prescan);

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
    std::vector<Type>* arguments_result) {

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

// Calculates the type of a binary operator expression from the types of its
// operands.
Type SemanticAstWalker::CalculateResultantType(Type left, Type right) {

    // We're going to take a stickler model in Gunderscript:
    // you must explicitly typecast everything. There are absolutely no
    // auto conversions between types.
    if (left == right) {
        return right;
    }

    // Accept ANY_TYPE nodes as matches for any given type.
    if (left == NONE) {
        return right;
    }
    if (right == NONE) {
        return left;
    }

    // Types don't match. Might require a typecast. (e.g.: float to int).
    throw SemanticAstWalkerTypeMismatchException(*this);
}

// Calculates the type of a binary operator expression from the types of its
// operands. Operands must both be numeric.
Type SemanticAstWalker::CalculateNumericResultantType(Type left, Type right) {

    // Disallow non-numerical operands.
    if ((left.type_format() != TypeFormat::INT && left.type_format() != TypeFormat::FLOAT) ||
        (right.type_format() != TypeFormat::INT && right.type_format() != TypeFormat::FLOAT)) {
        throw SemanticAstWalkerTypeMismatchException(*this);
    }

    // Do other checks.
    return CalculateResultantType(left, right);
}

// Calculates the type of a binary operator expression from the types of its
// operands. Operands must both be numeric.
Type SemanticAstWalker::CalculateBoolResultantType(Type left, Type right) {

    // Both operands must be BOOL.
    if (left != BOOL || right != BOOL) {
        throw SemanticAstWalkerTypeMismatchException(*this);
    }

    return right;
}

// Looks up a type node's name in the symbol table and returns its Type
// object.
Type SemanticAstWalker::ResolveTypeNode(Node* type_node) {
    // TODO: better exception here.
    const Symbol& type_symbol = this->symbol_table_.Get(*type_node->string_value());

    return type_symbol.type();
}

} // namespace library
} // namespace gunderscript
