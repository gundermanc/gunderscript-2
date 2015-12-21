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
    std::vector<LexerSymbol>& arguments_result) {

    Node* spec_name_node = spec_node->child(1);

    // Format the symbol name {class}::{function}$arg1$arg2...
    std::ostringstream name_buf;
    name_buf << *spec_name_node->string_value();
    name_buf << "::";
    name_buf << *name_node->string_value();

    // Append arguments to the symbol name.
    for (size_t i = 0; i < arguments_result.size(); i++) {
        name_buf << "$";
        name_buf << LexerSymbolString(arguments_result[i]);
    }

    return name_buf.str();
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
    Symbol spec_symbol(access_modifier_node->symbol_value(), *name_node->string_value());

    this->symbol_table_.Put(*name_node->string_value(), spec_symbol);
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
    std::vector<LexerSymbol>& arguments_result) {

    Node* spec_name_node = spec_node->child(1);

    FunctionSymbol function_symbol(
        access_modifier_node->symbol_value(),
        *spec_name_node->string_value(),
        *name_node->string_value(),
        native_node->bool_value(),
        type_node->symbol_value());

    this->symbol_table_.Put(
        MangleFunctionSymbolName(spec_node, name_node, arguments_result),
        function_symbol);
}

// Walks a single parameter in a spec function declaration.
// Returns it to the Function Declaration walker.
LexerSymbol SemanticAstWalker::WalkSpecFunctionDeclarationParameter(
    Node* spec_node,
    Node* function_node,
    Node* type_node,
    Node* name_node) {

    return type_node->symbol_value();
}

// Walks a single property in a spec property declaration.
// Defines it in the symbol table.
void SemanticAstWalker::WalkSpecPropertyDeclaration(
    Node* spec_node,
    Node* type_node,
    Node* name_node,
    Node* get_access_modifier_node,
    Node* set_access_modifier_node) {

    Node* spec_name_node = spec_node->child(1);

    // Format the getter symbol name {class}::{function}$arg1$arg2...
    std::ostringstream name_buf;
    name_buf << *spec_name_node->string_value();
    name_buf << "<-";
    name_buf << *name_node->string_value();

    // Create the symbol table symbol for the getter.
    FunctionSymbol get_function_symbol(
        get_access_modifier_node->symbol_value(),
        *spec_name_node->string_value(),
        *name_node->string_value(),
        false,
        type_node->symbol_value());

    // Define the getter symbol.
    this->symbol_table_.Put(name_buf.str(), get_function_symbol);

    // Format the setter symbol name {class}::{function}$arg1$arg2...
    name_buf.clear();
    name_buf << *spec_name_node->string_value();
    name_buf << "->";
    name_buf << *name_node->string_value();

    // Create the symbol table symbol for the getter.
    FunctionSymbol set_function_symbol(
        set_access_modifier_node->symbol_value(),
        *spec_name_node->string_value(),
        *name_node->string_value(),
        false,
        type_node->symbol_value());

    // Define the setter symbol.
    this->symbol_table_.Put(name_buf.str(), set_function_symbol);
}

// Walks a function call and checks to make sure that the types
// of the function matches the context.
LexerSymbol SemanticAstWalker::WalkFunctionCall(
    Node* spec_node,
    Node* name_node,
    std::vector<LexerSymbol>& arguments_result) {

    Node* spec_name_node = spec_node->child(1);

    // Lookup the function. Throws if there isn't a function with the correct arguments.
    const Symbol& symbol = this->symbol_table_.Get(
        MangleFunctionSymbolName(spec_node, name_node, arguments_result));

    // NOTE: unsafe cast here, this depends on the integrity and encapsulation
    // of the symbol_table_ being maintained.
    const FunctionSymbol* function_symbol = static_cast<const FunctionSymbol*>(&symbol);

    // Check for access to the callee function.
    // TODO: as of the moment this does nothing because we don't support calls between classes
    // yet. Implement calls between classes.
    CheckAccessModifier(
        *spec_name_node->string_value(),
        function_symbol->class_name(),
        function_symbol->access_modifier());

    return function_symbol->type();
}

// Checks to see if the given module name is valid. If it is not, throws
// an exception.
void SemanticAstWalker::CheckValidModuleName(const std::string& module_name) {
    if (!std::regex_match(module_name, module_name_pattern)) {
        throw SemanticAstWalkerInvalidPackageNameException(*this);
    }
}

// Walks the ADD node and calculates it's return type.
LexerSymbol SemanticAstWalker::WalkAdd(
    Node* spec_node,
    Node* left_node,
    Node* right_node,
    LexerSymbol left_result,
    LexerSymbol right_result) {

    return CalculateResultantType(left_result, right_result);
}

// Walks the SUB node and calculates it's return type.
LexerSymbol SemanticAstWalker::WalkSub(
    Node* spec_node,
    Node* left_node,
    Node* right_node,
    LexerSymbol left_result,
    LexerSymbol right_result) {

    return CalculateNumericResultantType(left_result, right_result);
}

// Walks the MUL node and calculates it's return type.
LexerSymbol SemanticAstWalker::WalkMul(
    Node* spec_node,
    Node* left_node,
    Node* right_node,
    LexerSymbol left_result,
    LexerSymbol right_result) {

    return CalculateNumericResultantType(left_result, right_result);
}

// Walks the DIV node and calculates it's return type.
LexerSymbol SemanticAstWalker::WalkDiv(
    Node* spec_node,
    Node* left_node,
    Node* right_node,
    LexerSymbol left_result,
    LexerSymbol right_result) {

    return CalculateNumericResultantType(left_result, right_result);
}
// Walks the MOD node and calculates it's return type.
LexerSymbol SemanticAstWalker::WalkMod(
    Node* spec_node,
    Node* left_node,
    Node* right_node,
    LexerSymbol left_result,
    LexerSymbol right_result) {

    return CalculateNumericResultantType(left_result, right_result);
}

// Walks the LOGAND node and calculates it's return type.
LexerSymbol SemanticAstWalker::WalkLogAnd(
    Node* spec_node,
    Node* left_node,
    Node* right_node,
    LexerSymbol left_result,
    LexerSymbol right_result) {

    return CalculateBoolResultantType(left_result, right_result);
}
// Walks the LOGOR node and calculates it's return type.
LexerSymbol SemanticAstWalker::WalkLogOr(
    Node* spec_node,
    Node* left_node,
    Node* right_node,
    LexerSymbol left_result,
    LexerSymbol right_result) {

    return CalculateBoolResultantType(left_result, right_result);
}

// Walks the BOOL node and returns the type for it.
LexerSymbol SemanticAstWalker::WalkBool(
    Node* spec_node,
    Node* function_node,
    Node* property_node,
    Node* bool_node) {

    return LexerSymbol::BOOL;
}

// Walks the INT node and returns the type for it.
LexerSymbol SemanticAstWalker::WalkInt(
    Node* spec_node,
    Node* function_node,
    Node* property_node,
    Node* int_node) {

    return LexerSymbol::INT;
}

// Walks the FLOAT node and returns the type for it.
LexerSymbol SemanticAstWalker::WalkFloat(
    Node* spec_node,
    Node* function_node,
    Node* property_node,
    Node* float_node) {

    return LexerSymbol::FLOAT;
}

// Walks the STRING node and returns the type for it.
LexerSymbol SemanticAstWalker::WalkString(
    Node* spec_node,
    Node* function_node,
    Node* property_node,
    Node* string_node) {

    return LexerSymbol::STRING;
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
        // What exactly 'internal'is currently up in the air.
        // Internal is INTENDED to mean that it is internal to the file,
        // but there isn't support for multifile lex/parse/typecheck yet.
        // TODO: complete this.
        throw new NotImplementedException();
    default:
        // Throw If someone adds a new access modifier that we don't know of.
        throw IllegalStateException();
    }
}

// Calculates the type of a binary operator expression from the types of its
// operands.
LexerSymbol SemanticAstWalker::CalculateResultantType(LexerSymbol left, LexerSymbol right) {

    // We're going to take a stickler model in Gunderscript:
    // you must explicitly typecast everything. There are absolutely no
    // auto conversions between types.
    if (left == right) {
        return right;
    }

    // Types don't match. Might require a typecast. (e.g.: float to int).
    throw SemanticAstWalkerTypeMismatchException(*this);
}

// Calculates the type of a binary operator expression from the types of its
// operands. Operands must both be numeric.
LexerSymbol SemanticAstWalker::CalculateNumericResultantType(LexerSymbol left, LexerSymbol right) {

    // Disallow non-numerical operands.
    if (left != LexerSymbol::INT && left != LexerSymbol::FLOAT &&
        right != LexerSymbol::INT && right != LexerSymbol::FLOAT) {
        throw SemanticAstWalkerTypeMismatchException(*this);
    }

    // Do other checks.
    CalculateResultantType(left, right);
}

// Calculates the type of a binary operator expression from the types of its
// operands. Operands must both be numeric.
LexerSymbol SemanticAstWalker::CalculateBoolResultantType(LexerSymbol left, LexerSymbol right) {

    // Both operands must be BOOL.
    if (left != LexerSymbol::BOOL || right != LexerSymbol::BOOL) {
        throw SemanticAstWalkerTypeMismatchException(*this);
    }

    return right;
}



} // namespace library
} // namespace gunderscript
