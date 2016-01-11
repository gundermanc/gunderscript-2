// Gunderscript-2 Parse/AST Node
// (C) 2014-2016 Christian Gunderman

#include "gunderscript/exceptions.h"
#include "type.h"

#include "ast_walker.h"

using gunderscript::IllegalStateException;

namespace gunderscript {
namespace library {

// Instantiate template with Type so we can link from external module.
template class AstWalker<Type>;

// Walks through all expected children of the MODULE
// AST node (the root of the AST). Expected children of this
// node are walked recursively. Unexpected children cause an
// IllegalStateException indicating that there is a bug.
template <typename ReturnType>
void AstWalker<ReturnType>::WalkModuleChildren() {
    // We can't have a non-module node as the root. If we do, indicate
    // that this is a bug.
    CheckNodeRule(&this->root(), NodeRule::MODULE);

    // Check that we have the proper number of MODULE node children.
    if (this->root().child_count() != 3) {
        THROW_EXCEPTION(
            this->root().line(),
            this->root().column(),
            STATUS_ILLEGAL_STATE);
    }

    // Call public pure virtual function implemented by child class
    // to give them a chance to walk/modify the AST.
    WalkModule(&this->root());

    Node* name_node = this->root().child(0);
    Node* depends_node = this->root().child(1);
    Node* specs_node = this->root().child(2);

    // Check child node types.
    CheckNodeRule(name_node, NodeRule::NAME);
    CheckNodeRule(depends_node, NodeRule::DEPENDS);
    CheckNodeRule(specs_node, NodeRule::SPECS);

    // Walk children.
    WalkModuleName(name_node);
    WalkModuleDependsChildren(depends_node);
    WalkModuleSpecsChildren(specs_node);
}

// Walks all child nodes of the DEPENDS node (nodes that indicate
// which files this script depends on). Children will all be NAME nodes
// with the name of another script file.
template <typename ReturnType>
void AstWalker<ReturnType>::WalkModuleDependsChildren(Node* depends_node) {

    // Iterate through all modules that this modules depends on.
    for (int i = 0; i < depends_node->child_count(); i++) {

        // Check that each node is a NAME node and then walk it.
        CheckNodeRule(depends_node->child(i), NodeRule::NAME);
        WalkModuleDependsName(depends_node->child(i));
    }
}

// Walks all child nodes of the SPECS node (nodes defining the specs/classes
// defined in this module.
template <typename ReturnType>
void AstWalker<ReturnType>::WalkModuleSpecsChildren(Node* specs_node) {

    // Iterate through all specs defined by this class.
    for (int i = 0; i < specs_node->child_count(); i++) {

        // Check that each node is a SPEC node and then walk it.
        CheckNodeRule(specs_node->child(i), NodeRule::SPEC);
        WalkSpec(specs_node->child(i));
    }
}

// Walks a SPEC node and its children recursively defining a 
// spec.
template <typename ReturnType>
void AstWalker<ReturnType>::WalkSpec(Node* spec_node) {

    CheckNodeRule(spec_node, NodeRule::SPEC);

    Node* access_modifier_node = spec_node->child(0);
    Node* name_node = spec_node->child(1);
    Node* functions_node = spec_node->child(2);
    Node* properties_node = spec_node->child(3);

    CheckNodeRule(access_modifier_node, NodeRule::ACCESS_MODIFIER);
    CheckNodeRule(name_node, NodeRule::NAME);
    CheckNodeRule(functions_node, NodeRule::FUNCTIONS);
    CheckNodeRule(properties_node, NodeRule::PROPERTIES);

    WalkSpecDeclaration(access_modifier_node, name_node);
    WalkSpecPropertiesFunctionsPrescanChildren(spec_node, functions_node, properties_node);
    WalkSpecFunctionsChildren(spec_node, functions_node);
    WalkSpecPropertiesChildren(spec_node, properties_node);
}

// Walks the children of the FUNCTION node child of the SPEC node.
template <typename ReturnType>
void AstWalker<ReturnType>::WalkSpecFunctionsChildren(Node* spec_node, Node* functions_node) {
    CheckNodeRule(spec_node, NodeRule::SPEC);
    CheckNodeRule(functions_node, NodeRule::FUNCTIONS);

    // Iterates through all function declarations in the spec,
    // this time looking only at the function implentation.
    for (int i = 0; i < functions_node->child_count(); i++) {
        Node* function_node = functions_node->child(i);

        WalkSpecFunctionChildren(spec_node, function_node, false);
    }
}

// Walks the children of a FUNCTION node.
// This function is a two step process: step one has prescan set to true
// indicating that we wish to only read the function header and declaration.
// Step two has prescan set to true, indicating that this time we will look
// at just the implementation. Breaking this process into two steps allows
// for us to call functions out of the order that they appear in the AST.
template <typename ReturnType>
void AstWalker<ReturnType>::WalkSpecFunctionChildren(
    Node* spec_node,
    Node* function_node, 
    bool prescan) {
    CheckNodeRule(spec_node, NodeRule::SPEC);
    CheckNodeRule(function_node, NodeRule::FUNCTION);

    // Get function attribute objects.
    Node* access_modifier_node = function_node->child(0);
    Node* native_node = function_node->child(1);
    Node* type_node = function_node->child(2);
    Node* name_node = function_node->child(3);
    Node* function_params_node = function_node->child(4);
    Node* block_node = function_node->child(5);

    // Naively check the node rules for basic troubleshooting.
    CheckNodeRule(access_modifier_node, NodeRule::ACCESS_MODIFIER);
    CheckNodeRule(native_node, NodeRule::NATIVE);
    CheckNodeRule(type_node, NodeRule::TYPE);
    CheckNodeRule(name_node, NodeRule::NAME);
    CheckNodeRule(function_params_node, NodeRule::FUNCTION_PARAMETERS);
    CheckNodeRule(block_node, NodeRule::BLOCK);

    // Dispatch the arguments walker to subclass.
    std::vector<ReturnType> arguments_result;
    WalkSpecFunctionDeclarationParametersChildren(
        spec_node,
        function_node,
        function_params_node,
        arguments_result,
        prescan);

    // Dispatch to subclass.
    WalkSpecFunctionDeclaration(
        spec_node,
        access_modifier_node,
        native_node,
        type_node,
        name_node,
        block_node,
        arguments_result,
        prescan);

    // This is the prescan iteration, return without iterating the function body.
    if (prescan) {
        return;
    }

    // Walk the BLOCK node and its children in the block.
    WalkBlockChildren(
        spec_node,
        function_node,
        NULL,
        PropertyFunction::NONE,
        block_node,
        &arguments_result);
}

// Walks the Functions and Properties and prescans them so that subclasses
// that inherit from ASTWalker will have an opportunity to define symbols for
// functions and properties before walking the function and property bodies.
template <typename ReturnType>
void AstWalker<ReturnType>::WalkSpecPropertiesFunctionsPrescanChildren(
    Node* spec_node,
    Node* functions_node,
    Node* properties_node) {

    CheckNodeRule(spec_node, NodeRule::SPEC);
    CheckNodeRule(functions_node, NodeRule::FUNCTIONS);
    CheckNodeRule(properties_node, NodeRule::PROPERTIES);

    // Iterates through all function declarations in the spec
    // reading only the function headers and definitions.
    for (int i = 0; i < functions_node->child_count(); i++) {
        Node* function_node = functions_node->child(i);

        WalkSpecFunctionChildren(spec_node, function_node, true);
    }

    // Iterate through all properties in the SPEC's PROPERTIES node.
    for (int i = 0; i < properties_node->child_count(); i++) {
        Node* property_node = properties_node->child(i);

        WalkSpecPropertyChildren(spec_node, property_node, true);
    }
}

// Walks the children of the the FUNCTION_PARAMS node.
template <typename ReturnType>
void AstWalker<ReturnType>::WalkSpecFunctionDeclarationParametersChildren(
    Node* spec_node,
    Node* function_node, 
    Node* function_params_node,
    std::vector<ReturnType>& argument_result,
    bool prescan) {

    CheckNodeRule(spec_node, NodeRule::SPEC);
    CheckNodeRule(function_node, NodeRule::FUNCTION);
    CheckNodeRule(function_params_node, NodeRule::FUNCTION_PARAMETERS); 

    // Iterate all FUNCTION_PARAMETER nodes in the FUNCTION_PARAMETERS node
    // and dispatch to subclass function.
    for (int i = 0; i < function_params_node->child_count(); i++) {
        Node* function_param_node = function_params_node->child(i);

        CheckNodeRule(function_param_node, NodeRule::FUNCTION_PARAMETER);

        Node* type_node = function_param_node->child(0);
        Node* name_node = function_param_node->child(1);

        CheckNodeRule(type_node, NodeRule::TYPE);
        CheckNodeRule(name_node, NodeRule::NAME);

        argument_result.push_back(
            WalkSpecFunctionDeclarationParameter(
                spec_node,
                function_node, 
                type_node,
                name_node,
                prescan));
    }
}

// Walks the children of the PROPERTIES node child of the SPEC node.
template <typename ReturnType>
void AstWalker<ReturnType>::WalkSpecPropertiesChildren(Node* spec_node, Node* properties_node) {
    CheckNodeRule(spec_node, NodeRule::SPEC);
    CheckNodeRule(properties_node, NodeRule::PROPERTIES);

    // Iterate through all properties in the SPEC's PROPERTIES node.
    for (int i = 0; i < properties_node->child_count(); i++) {
        Node* property_node = properties_node->child(i);

        WalkSpecPropertyChildren(spec_node, property_node, false);
    }
}

// Walks the children of a single PROPERTY node that represents a SPEC property.
// This is a two step process: step one, prescan is true, indicating that we are
// doing an initial run through of the properties, just defining them based upon
// their signatures. Step two, prescan is false and we walk the property function
// bodies too. This multistep process allows us to declare symbols for all properties
// before type checking the bodies, imbuing the properties with awareness of one
// another regardless of their order in the AST.
// TODO: complete this.
template <typename ReturnType>
void AstWalker<ReturnType>::WalkSpecPropertyChildren(
    Node* spec_node,
    Node* property_node,
    bool prescan) {

    // Get all mandatory child nodes.
    Node* type_node = property_node->child(0);
    Node* name_node = property_node->child(1);
    Node* get_property_function_node = property_node->child(2);
    Node* set_property_function_node = property_node->child(3);
    Node* get_access_modifier_node = get_property_function_node->child(0);
    Node* set_access_modifier_node = set_property_function_node->child(0);

    // Check mandatory child node types.
    CheckNodeRule(type_node, NodeRule::TYPE);
    CheckNodeRule(name_node, NodeRule::NAME);
    CheckNodeRule(get_property_function_node, NodeRule::PROPERTY_FUNCTION);
    CheckNodeRule(set_property_function_node, NodeRule::PROPERTY_FUNCTION);
    CheckNodeRule(get_access_modifier_node, NodeRule::ACCESS_MODIFIER);
    CheckNodeRule(set_access_modifier_node, NodeRule::ACCESS_MODIFIER);

    // Check the types of the optional child nodes.
    // The null check is because we don't want to throw if these
    // nodes were not provided.
    Node* get_block_node = NULL;
    Node* set_block_node = NULL;
    if (get_property_function_node->child_count() >= 2) {
        get_block_node = get_property_function_node->child(1);
        CheckNodeRule(get_block_node, NodeRule::BLOCK);
    }
    if (set_property_function_node->child_count() >= 2) {
        set_block_node = set_property_function_node->child(1);
        CheckNodeRule(set_block_node, NodeRule::BLOCK);
    }

    // Dispatch to subclass function.
    WalkSpecPropertyDeclaration(
        spec_node,
        type_node,
        name_node,
        get_access_modifier_node,
        set_access_modifier_node,
        prescan);

    // This is a prescan, stop after the property declaration.
    if (prescan) {
        return;
    }

    // Walk property function blocks.
    if (get_block_node != NULL) {
        WalkBlockChildren(spec_node, NULL, property_node,
            PropertyFunction::GET, get_block_node, NULL);
    }
    if (set_block_node != NULL) {
        WalkBlockChildren(spec_node, NULL, property_node,
            PropertyFunction::SET, set_block_node, NULL);
    }
}

// Walks the Children of the BLOCK AST nodes.
// property_function_node can be either a property or a function node.
// TODO: Complete this.
template <typename ReturnType>
void AstWalker<ReturnType>::WalkBlockChildren(
    Node* spec_node, 
    Node* function_node,
    Node* property_node,
    PropertyFunction property_function,
    Node* block_node,
    std::vector<ReturnType>* arguments_result) {

    CheckNodeRule(block_node, NodeRule::BLOCK);

    // Iterate through all statements in the block.
    for (int i = 0; i < block_node->child_count(); i++) {
        Node* statement_node = block_node->child(i);

        switch (statement_node->rule())
        {
        case NodeRule::CALL:
            WalkFunctionCallChildren(spec_node, function_node, statement_node);
            break;
        case NodeRule::ASSIGN:
            WalkAssignChildren(spec_node, function_node, property_node, property_function, statement_node);
            break;
        case NodeRule::RETURN:
            WalkReturnChildren(
                spec_node,
                function_node, 
                property_node,
                property_function,
                statement_node,
                arguments_result);
            break;
        case NodeRule::BLOCK:
            WalkBlockChildren(
                spec_node,
                function_node,
                property_node,
                property_function,
                statement_node,
                arguments_result);
            break;
        default:
            THROW_EXCEPTION(
                this->root().line(),
                this->root().column(),
                STATUS_ILLEGAL_STATE);
        }
    }
}

// Walks through a CALL Node's children.
template <typename ReturnType>
void AstWalker<ReturnType>::WalkFunctionCallChildren(
    Node* spec_node, 
    Node* function_node, 
    Node* call_node) {

    CheckNodeRule(spec_node, NodeRule::SPEC);
    CheckNodeRule(call_node, NodeRule::CALL);

    Node* name_node = call_node->child(0);
    Node* arguments_node = call_node->child(1);
    std::vector<ReturnType> arguments_result;

    CheckNodeRule(name_node, NodeRule::NAME);
    CheckNodeRule(arguments_node, NodeRule::CALL_PARAMETERS);

    // Iterate through all call param expressions and evaluate their
    // types.
    for (int i = 0; i < arguments_node->child_count(); i++) {
        Node* expression_node = arguments_node->child(i);

        // Walk the parameter expressions and add the results
        // to the params vector.
        arguments_result.push_back(
            WalkExpressionChildren(
                spec_node,
                function_node,
                NULL,
                PropertyFunction::NONE,
                expression_node));
    }

    // Walk the function call and provide it with the results of our
    // walk of the arguments.
    WalkFunctionCall(spec_node, name_node, arguments_result);
}

// Walks through an ASSIGN Node's children.
template <typename ReturnType>
ReturnType AstWalker<ReturnType>::WalkAssignChildren(
    Node* spec_node,
    Node* function_node,
    Node* property_node,
    PropertyFunction property_function,
    Node* assign_node) {

    CheckNodeRule(spec_node, NodeRule::SPEC);
    CheckNodeRule(assign_node, NodeRule::ASSIGN);
    
    Node* symbol_node = assign_node->child(0);
    CheckNodeRule(symbol_node, NodeRule::SYMBOL);

    // OK, so, binary_operation_node is a bit weird. When ASSIGN is used as
    // a statement you would expect the value being assigned to be an
    // EXPRESSION node, but expressions are top level and assigns can
    // be embedded in an EXPRESSION, so instead the value being assigned
    // is simply any binary operation and has no specific NodeRule to check.
    Node* name_node = symbol_node->child(0);
    Node* binary_operation_node = assign_node->child(1);

    CheckNodeRule(name_node, NodeRule::NAME);

    // Walk the binary operation and obtain the result.
    ReturnType binary_operation_result = WalkBinaryOperationChildren(
        spec_node,
        function_node,
        property_node,
        property_function,
        binary_operation_node);
    
    // Dispatch assignment walker to child class and feed in result of
    // of the binary operation walk.
    return WalkAssign(
        spec_node,
        name_node,
        binary_operation_result);
}

// Walks through a RETURN statement Node's children.
template <typename ReturnType>
void AstWalker<ReturnType>::WalkReturnChildren(
    Node* spec_node,
    Node* function_node,
    Node* property_node,
    PropertyFunction property_function,
    Node* return_node,
    std::vector<ReturnType>* arguments_result) {

    // Check mandatory nodes.
    CheckNodeRule(spec_node, NodeRule::SPEC);
    CheckNodeRule(return_node, NodeRule::RETURN);

    // Check optional nodes.
    if (function_node != NULL) {
        CheckNodeRule(function_node, NodeRule::FUNCTION);
    }
    if (property_node != NULL) {
        CheckNodeRule(property_node, NodeRule::PROPERTY);
    }

    Node* expression_node = return_node->child(0);
    CheckNodeRule(expression_node, NodeRule::EXPRESSION);

    // Walk the return expression.
    ReturnType expression_result = WalkExpressionChildren(
        spec_node,
        function_node,
        property_node,
        property_function,
        expression_node);

    // Dispatch the results of walking the expression to the child class.
    WalkReturn(
        spec_node,
        function_node,
        property_node,
        property_function,
        expression_result,
        arguments_result);
}

// Walks all children of the EXPRESSION node.
template <typename ReturnType>
ReturnType AstWalker<ReturnType>::WalkExpressionChildren(
    Node* spec_node,
    Node* function_node,
    Node* property_node,
    PropertyFunction property_function,
    Node* expression_node) {

    // Check mandatory nodes used by this walker.
    CheckNodeRule(expression_node, NodeRule::EXPRESSION);

    return WalkBinaryOperationChildren(
        spec_node,
        function_node,
        property_node,
        property_function,
        expression_node->child(0));
}

// Walks all children of binary expressions.
// TODO: complete this.
template <typename ReturnType>
ReturnType AstWalker<ReturnType>::WalkBinaryOperationChildren(
    Node* spec_node,
    Node* function_node,
    Node* property_node,
    PropertyFunction property_function,
    Node* binary_operation_node) {

    // This node has children, treat it as a binary operation.
    if (binary_operation_node->child_count() == 2) {

        // The ASSIGN expression special case.
        // Although we COULD treat the assign operator as a normal
        // binary operation, we instead special case it to make
        // the ASTWalker have it's own assign statement walker
        // function, making writing an interpreter easier than if
        // we had separate walkers for the symbol reference and the
        // binary_operation_node.
        if (binary_operation_node->rule() == NodeRule::ASSIGN) {
            return WalkAssignChildren(
                spec_node,
                function_node,
                property_node,
                property_function,
                binary_operation_node);
        }

        Node* left_node = binary_operation_node->child(0);
        Node* right_node = binary_operation_node->child(1);

        ReturnType left_result = WalkBinaryOperationChildren(
            spec_node,
            function_node,
            property_node,
            property_function,
            left_node);

        ReturnType right_result = WalkBinaryOperationChildren(
            spec_node,
            function_node,
            property_node,
            property_function,
            right_node);

        // Switch all binary operations.
        switch (binary_operation_node->rule()) {
        case NodeRule::ADD:
            return WalkAdd(
                spec_node,
                left_node,
                right_node,
                left_result,
                right_result);
        case NodeRule::SUB:
            return WalkSub(
                spec_node,
                left_node,
                right_node,
                left_result,
                right_result);
        case NodeRule::MUL:
            return WalkMul(
                spec_node,
                left_node,
                right_node,
                left_result,
                right_result);
        case NodeRule::DIV:
            return WalkDiv(
                spec_node,
                left_node,
                right_node,
                left_result,
                right_result);
        case NodeRule::MOD:
            return WalkMod(
                spec_node,
                left_node,
                right_node,
                left_result,
                right_result);
        case NodeRule::LOGAND:
            return WalkLogAnd(
                spec_node,
                left_node,
                right_node,
                left_result,
                right_result);
        case NodeRule::LOGOR:
            return WalkLogOr(
                spec_node,
                left_node,
                right_node,
                left_result,
                right_result);
        case NodeRule::GREATER:
            return WalkGreater(
                spec_node,
                left_node,
                right_node,
                left_result,
                right_result);
        case NodeRule::EQUALS:
            return WalkEquals(
                spec_node,
                left_node,
                right_node,
                left_result,
                right_result);
        case NodeRule::NOT_EQUALS:
            return WalkNotEquals(
                spec_node,
                left_node,
                right_node,
                left_result,
                right_result);
        case NodeRule::LESS:
            return WalkLess(
                spec_node,
                left_node,
                right_node,
                left_result,
                right_result);
        case NodeRule::GREATER_EQUALS:
            return WalkGreaterEquals(
                spec_node,
                left_node,
                right_node,
                left_result,
                right_result);
        case NodeRule::LESS_EQUALS:
            return WalkLessEquals(
                spec_node,
                left_node,
                right_node,
                left_result,
                right_result);
        default:
            THROW_EXCEPTION(
                this->root().line(),
                this->root().column(),
                STATUS_ILLEGAL_STATE);
        }
    }

    // Not a binary operation, it's an atomic expression.
    return WalkAtomicExpressionChildren(
        spec_node,
        function_node,
        property_node,
        property_function,
        binary_operation_node);
}

// Parses any atomic expression such as STRING, BOOL, INT, and FLOAT.
template <typename ReturnType>
ReturnType AstWalker<ReturnType>::WalkAtomicExpressionChildren(
    Node* spec_node,
    Node* function_node,
    Node* property_node,
    PropertyFunction property_function,
    Node* atomic_node) {

    // Check mandatory nodes.
    CheckNodeRule(spec_node, NodeRule::SPEC);

    // Check optional nodes. Property and function are optional
    // because expressions can operate in the context of either.
    if (function_node != NULL) {
        CheckNodeRule(function_node, NodeRule::FUNCTION);
    }
    if (property_node != NULL) {
        CheckNodeRule(property_node, NodeRule::PROPERTY);
    }

    switch (atomic_node->rule()) {
    case NodeRule::LOGNOT:
        return WalkLogNot(
            spec_node,
            atomic_node->child(0),
            WalkBinaryOperationChildren(
                spec_node,
                function_node,
                property_node,
                property_function,
                atomic_node->child(0)));
    case NodeRule::BOOL:
        return WalkBool(
            spec_node,
            function_node,
            property_node,
            property_function,
            atomic_node);
    case NodeRule::INT:
        return WalkInt(
            spec_node,
            function_node,
            property_node,
            property_function,
            atomic_node);
    case NodeRule::FLOAT:
        return WalkFloat(
            spec_node,
            function_node,
            property_node,
            property_function,
            atomic_node);
    case NodeRule::STRING:
        return WalkString(
            spec_node,
            function_node,
            property_node,
            property_function,
            atomic_node);
    case NodeRule::CHAR:
        return WalkChar(
            spec_node,
            function_node,
            property_node,
            property_function,
            atomic_node);
    case NodeRule::SYMBOL:
        return WalkVariable(
            spec_node,
            function_node,
            property_node,
            property_function,
            atomic_node->child(0));
    case NodeRule::ANY_TYPE:
        return WalkAnyType(
            spec_node,
            function_node,
            property_node,
            property_function,
            atomic_node);
    default:
        THROW_EXCEPTION(
            this->root().line(),
            this->root().column(),
            STATUS_ILLEGAL_STATE);
    }
}

// Checks that the given node is for the given rule. If it is not,
// throws IllegalStateException to indicate a bug somewhere.
template <typename ReturnType>
void AstWalker<ReturnType>::CheckNodeRule(Node* node, NodeRule rule) {
    if (node->rule() != rule) {
        THROW_EXCEPTION(
            this->root().line(),
            this->root().column(),
            STATUS_ILLEGAL_STATE);
    }
}

} // namespace library
} // namespace gunderscript
