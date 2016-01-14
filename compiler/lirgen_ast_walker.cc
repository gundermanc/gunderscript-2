// Gunderscript-2 NanoJIT LIR Generator
// (C) 2016 Christian Gunderman

#include "lirgen_ast_walker.h"

namespace gunderscript {
namespace compiler {

// Walks the MODULE node in the abstract syntax tree.
// Since there is no type information in this node, we can
// safely do nothing.
void LIRGenAstWalker::WalkModule(Node* module_node) {
    // Module itself has no properties to check.
    // We instead check its child elements individually.
}

// Walks the MODULE node's NAME node. This node defines the module name
// (analogous to the Java package name) of a script file.
// The only verification performed at the moment is simple naive name
// pattern matching.
void LIRGenAstWalker::WalkModuleName(Node* name_node) {

}

// Checks that the given dependency module name is valid.
// If not, throws an exception.
// TODO: calculate dependency graph and lex/parse/typecheck the
// dependency first.
void LIRGenAstWalker::WalkModuleDependsName(Node* name_node) {

}

// Attempts to declare a new spec in the given scope. Throws if spec
// name is taken in this context.
void LIRGenAstWalker::WalkSpecDeclaration(Node* access_modifier_node, Node* name_node) {

}

// Walks a single function declaration inside of a SPEC.
// Throws if the function already exists in the symbol table.
void LIRGenAstWalker::WalkSpecFunctionDeclaration(
    Node* spec_node,
    Node* access_modifier_node,
    Node* native_node,
    Node* type_node,
    Node* name_node,
    Node* block_node,
    std::vector<LirGenResult>& arguments_result,
    bool prescan) {

}

// Walks a single parameter in a spec function declaration.
// Returns it to the Function Declaration walker.
LirGenResult LIRGenAstWalker::WalkSpecFunctionDeclarationParameter(
    Node* spec_node,
    Node* function_node,
    Node* type_node,
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
    Node* get_access_modifier_node,
    Node* set_access_modifier_node,
    bool prescan) {

}

// Walks a function call and checks to make sure that the types
// of the function matches the context.
LirGenResult LIRGenAstWalker::WalkFunctionCall(
    Node* spec_node,
    Node* name_node,
    std::vector<LirGenResult>& arguments_result) {

    return LirGenResult();
}

// Walks an assignment statement or expression and checks to make sure
// that the types match the context in which it was used.
LirGenResult LIRGenAstWalker::WalkAssign(
    Node* spec_node,
    Node* name_node,
    LirGenResult operations_result) {

    return LirGenResult();
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

    return LirGenResult();
}

// Walks the ADD node and calculates it's return type.
LirGenResult LIRGenAstWalker::WalkAdd(
    Node* spec_node,
    Node* left_node,
    Node* right_node,
    LirGenResult left_result,
    LirGenResult right_result) {

    return LirGenResult();
}

// Walks the SUB node and calculates it's return type.
LirGenResult LIRGenAstWalker::WalkSub(
    Node* spec_node,
    Node* left_node,
    Node* right_node,
    LirGenResult left_result,
    LirGenResult right_result) {

    return LirGenResult();
}

// Walks the MUL node and calculates it's return type.
LirGenResult LIRGenAstWalker::WalkMul(
    Node* spec_node,
    Node* left_node,
    Node* right_node,
    LirGenResult left_result,
    LirGenResult right_result) {

    return LirGenResult();
}

// Walks the DIV node and calculates it's return type.
LirGenResult LIRGenAstWalker::WalkDiv(
    Node* spec_node,
    Node* left_node,
    Node* right_node,
    LirGenResult left_result,
    LirGenResult right_result) {

    return LirGenResult();
}
// Walks the MOD node and calculates it's return type.
LirGenResult LIRGenAstWalker::WalkMod(
    Node* spec_node,
    Node* left_node,
    Node* right_node,
    LirGenResult left_result,
    LirGenResult right_result) {

    return LirGenResult();
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

    return LirGenResult();
}

// Walks the TYPE_INT node and returns the type for it.
LirGenResult LIRGenAstWalker::WalkInt(
    Node* spec_node,
    Node* function_node,
    Node* property_node,
    PropertyFunction property_function,
    Node* int_node) {

    return LirGenResult();
}

// Walks the FLOAT node and returns the type for it.
LirGenResult LIRGenAstWalker::WalkFloat(
    Node* spec_node,
    Node* function_node,
    Node* property_node,
    PropertyFunction property_function,
    Node* float_node) {

    return LirGenResult();
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

    return LirGenResult();
}

// Walks the SYMBOL->NAME subtree that represents a variable reference
// and returns the type for it.
LirGenResult LIRGenAstWalker::WalkVariable(
    Node* spec_node,
    Node* function_node,
    Node* property_node,
    PropertyFunction property_function,
    Node* name_node) {

    return LirGenResult();
}

// Walks the ANY_TYPE node and returns the type for it.
LirGenResult LIRGenAstWalker::WalkAnyLirGenResult(
    Node* spec_node,
    Node* function_node,
    Node* property_node,
    PropertyFunction property_function,
    Node* any_type_node) {

    return LirGenResult();
}

// Optional implemented function that overrides base class implementation.
// In LIRGenAstWalker, this function pushes a new table to the SymbolTable
// to introduce new context for each FUNCTION entered, limiting the
// scope of function arguments.
void LIRGenAstWalker::WalkSpecFunctionChildren(
    Node* spec_node,
    Node* function_node,
    bool prescan) {
}

// Optional implemented function that overrides base class implementation.
// In LIRGenAstWalker, this function pushes a new table to the SymbolTable
// to introduce new context for each BLOCK ('{' to '}') entered, limiting the
// scope of block variables.
void LIRGenAstWalker::WalkBlockChildren(
    Node* spec_node,
    Node* function_node,
    Node* property_node,
    PropertyFunction property_function,
    Node* block,
    std::vector<LirGenResult>* arguments_result) {
}

} // namespace compiler
} // namespace gunderscript
