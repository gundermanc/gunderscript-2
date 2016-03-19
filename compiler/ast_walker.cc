// Gunderscript-2 Parse/AST Node
// (C) 2014-2016 Christian Gunderman

#include "gunderscript/exceptions.h"
#include "gunderscript/type.h"

#include "ast_walker.h"
#include "gs_assert.h"

// HACK: this include is here for explicit template instantiation for LirGenResult.
#include "lirgen_ast_walker.h"

// Debug assertion checks that we have the correct node rule.
// Not compiled in Release configuration.
#define GS_ASSERT_NODE_RULE(node, node_rule) \
    GS_ASSERT_TRUE(((node) != NULL) && ((node)->rule() == (node_rule)), "Null AstWalker node or invalid node rule");
#define GS_ASSERT_OPTIONAL_NODE_RULE(node, node_rule) \
    GS_ASSERT_TRUE(((node) == NULL) || ((node)->rule() == (node_rule)), "Invalid AstWalker node rule");

namespace gunderscript {
namespace compiler {

// Instantiate template with Type and LirGenResult so we can link from external module.
template class AstWalker<Type>;
template class AstWalker<LirGenResult>;

// Walks through all expected children of the MODULE
// AST node (the root of the AST). Expected children of this
// node are walked recursively. Unexpected children cause an
// IllegalStateException indicating that there is a bug.
template <typename ReturnType>
void AstWalker<ReturnType>::WalkModuleChildren() {
    // We can't have a non-module node as the root.
    GS_ASSERT_NODE_RULE(&this->root(), NodeRule::MODULE);
    GS_ASSERT_TRUE(this->root().child_count() == 4, "AstWalker expected 4 nodes in AST root");

    // Call public pure virtual function implemented by child class
    // to give them a chance to walk/modify the AST.
    WalkModule(&this->root());

    Node* name_node = this->root().child(0);
    Node* depends_node = this->root().child(1);
    Node* specs_node = this->root().child(2);
    Node* functions_node = this->root().child(3);

    // Check child node types.
    GS_ASSERT_NODE_RULE(name_node, NodeRule::NAME);
    GS_ASSERT_NODE_RULE(depends_node, NodeRule::DEPENDS);
    GS_ASSERT_NODE_RULE(specs_node, NodeRule::SPECS);
    GS_ASSERT_NODE_RULE(functions_node, NodeRule::FUNCTIONS);

    // Walk children.
    // Call to WalkPropertiesFunctionsPrescanChildren() must come before WalkModuleSpecsChildren()
    // because it is prescanning the static functions before so the non-static functions
    // are aware of them.
    WalkModuleName(name_node);
    WalkModuleDependsChildren(depends_node);
    WalkPropertiesFunctionsPrescanChildren(NULL, functions_node, NULL);
    WalkModuleSpecsChildren(specs_node);
    WalkFunctionsChildren(NULL, functions_node);
}

// Walks all child nodes of the DEPENDS node (nodes that indicate
// which files this script depends on). Children will all be NAME nodes
// with the name of another script file.
template <typename ReturnType>
void AstWalker<ReturnType>::WalkModuleDependsChildren(Node* depends_node) {
    GS_ASSERT_NODE_RULE(depends_node, NodeRule::DEPENDS);

    // Iterate through all modules that this modules depends on.
    for (size_t i = 0; i < depends_node->child_count(); i++) {

        // Check that each node is a NAME node and then walk it.
        GS_ASSERT_NODE_RULE(depends_node->child(i), NodeRule::NAME);
        WalkModuleDependsName(depends_node->child(i));
    }
}

// Walks all child nodes of the SPECS node (nodes defining the specs/classes
// defined in this module.
template <typename ReturnType>
void AstWalker<ReturnType>::WalkModuleSpecsChildren(Node* specs_node) {
    GS_ASSERT_NODE_RULE(specs_node, NodeRule::SPECS);

    // Iterate through all specs defined by this module.
    for (size_t i = 0; i < specs_node->child_count(); i++) {

        // Check that each node is a SPEC node and then walk it.
        GS_ASSERT_NODE_RULE(specs_node->child(i), NodeRule::SPEC);
        WalkSpec(specs_node->child(i));
    }
}

// Walks a SPEC node and its children recursively defining a 
// spec.
template <typename ReturnType>
void AstWalker<ReturnType>::WalkSpec(Node* spec_node) {

    GS_ASSERT_NODE_RULE(spec_node, NodeRule::SPEC);
    GS_ASSERT_TRUE(spec_node->child_count() == 4, "AstWalker expects SPEC node to have 4 children");

    Node* access_modifier_node = spec_node->child(0);
    Node* name_node = spec_node->child(1);
    Node* functions_node = spec_node->child(2);
    Node* properties_node = spec_node->child(3);

    GS_ASSERT_NODE_RULE(access_modifier_node, NodeRule::ACCESS_MODIFIER);
    GS_ASSERT_NODE_RULE(name_node, NodeRule::TYPE);
    GS_ASSERT_NODE_RULE(functions_node, NodeRule::FUNCTIONS);
    GS_ASSERT_NODE_RULE(properties_node, NodeRule::PROPERTIES);

    WalkSpecDeclaration(spec_node, access_modifier_node, name_node);
    WalkPropertiesFunctionsPrescanChildren(spec_node, functions_node, properties_node);
    WalkFunctionsChildren(spec_node, functions_node);
    WalkSpecPropertiesChildren(spec_node, properties_node);
}

// Walks the children of the FUNCTION nodes.
template <typename ReturnType>
void AstWalker<ReturnType>::WalkFunctionsChildren(Node* spec_node, Node* functions_node) {
    GS_ASSERT_OPTIONAL_NODE_RULE(spec_node, NodeRule::SPEC);
    GS_ASSERT_NODE_RULE(functions_node, NodeRule::FUNCTIONS);

    // Iterates through all function declarations in the spec,
    // this time looking only at the function implentation.
    for (size_t i = 0; i < functions_node->child_count(); i++) {
        Node* function_node = functions_node->child(i);

        WalkFunctionChildren(spec_node, function_node, false);
    }
}

// Walks the children of a FUNCTION node.
// This function is a two step process: step one has prescan set to true
// indicating that we wish to only read the function header and declaration.
// Step two has prescan set to true, indicating that this time we will look
// at just the implementation. Breaking this process into two steps allows
// for us to call functions out of the order that they appear in the AST.
template <typename ReturnType>
void AstWalker<ReturnType>::WalkFunctionChildren(
    Node* spec_node,
    Node* function_node, 
    bool prescan) {
    GS_ASSERT_OPTIONAL_NODE_RULE(spec_node, NodeRule::SPEC);
    GS_ASSERT_NODE_RULE(function_node, NodeRule::FUNCTION);
    GS_ASSERT_TRUE(function_node->child_count() == 6, "AstWalker expects FUNCTION to have 6 children");

    // Get function attribute objects.
    Node* access_modifier_node = function_node->child(0);
    Node* native_node = function_node->child(1);
    Node* type_node = function_node->child(2);
    Node* name_node = function_node->child(3);
    Node* function_params_node = function_node->child(4);
    Node* block_node = function_node->child(5);

    // Naively check the node rules for basic troubleshooting.
    GS_ASSERT_NODE_RULE(access_modifier_node, NodeRule::ACCESS_MODIFIER);
    GS_ASSERT_NODE_RULE(native_node, NodeRule::NATIVE);
    GS_ASSERT_NODE_RULE(type_node, NodeRule::TYPE);
    GS_ASSERT_NODE_RULE(name_node, NodeRule::NAME);
    GS_ASSERT_NODE_RULE(function_params_node, NodeRule::FUNCTION_PARAMETERS);
    GS_ASSERT_NODE_RULE(block_node, NodeRule::BLOCK);

    // Dispatch the arguments walker to subclass.
    std::vector<ReturnType> arguments_result;
    WalkFunctionDeclarationParametersChildren(
        spec_node,
        function_node,
        function_params_node,
        arguments_result,
        prescan);

    // Dispatch to subclass.
    WalkFunctionDeclaration(
        spec_node,
        function_node,
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
void AstWalker<ReturnType>::WalkPropertiesFunctionsPrescanChildren(
    Node* spec_node,
    Node* functions_node,
    Node* properties_node) {

    GS_ASSERT_OPTIONAL_NODE_RULE(spec_node, NodeRule::SPEC);
    GS_ASSERT_NODE_RULE(functions_node, NodeRule::FUNCTIONS);
    GS_ASSERT_OPTIONAL_NODE_RULE(properties_node, NodeRule::PROPERTIES);

    // Iterates through all function declarations in the spec
    // reading only the function headers and definitions.
    for (size_t i = 0; i < functions_node->child_count(); i++) {
        Node* function_node = functions_node->child(i);

        WalkFunctionChildren(spec_node, function_node, true);
    }

    if (properties_node != NULL) {
        // Iterate through all properties in the SPEC's PROPERTIES node.
        for (size_t i = 0; i < properties_node->child_count(); i++) {
            Node* property_node = properties_node->child(i);

            WalkSpecPropertyChildren(spec_node, property_node, true);
        }
    }
}

// Walks the children of the the FUNCTION_PARAMS node.
template <typename ReturnType>
void AstWalker<ReturnType>::WalkFunctionDeclarationParametersChildren(
    Node* spec_node,
    Node* function_node, 
    Node* function_params_node,
    std::vector<ReturnType>& argument_result,
    bool prescan) {
    
    GS_ASSERT_OPTIONAL_NODE_RULE(spec_node, NodeRule::SPEC);
    GS_ASSERT_NODE_RULE(function_node, NodeRule::FUNCTION);
    GS_ASSERT_NODE_RULE(function_params_node, NodeRule::FUNCTION_PARAMETERS); 

    // Iterate all FUNCTION_PARAMETER nodes in the FUNCTION_PARAMETERS node
    // and dispatch to subclass function.
    for (size_t i = 0; i < function_params_node->child_count(); i++) {
        Node* function_param_node = function_params_node->child(i);

        GS_ASSERT_NODE_RULE(function_param_node, NodeRule::FUNCTION_PARAMETER);
        GS_ASSERT_TRUE(function_param_node->child_count() == 2,
            "AstWalker expected FUNCTION to have to children");

        Node* type_node = function_param_node->child(0);
        Node* name_node = function_param_node->child(1);

        GS_ASSERT_NODE_RULE(type_node, NodeRule::TYPE);
        GS_ASSERT_NODE_RULE(name_node, NodeRule::NAME);

        argument_result.push_back(
            WalkSpecFunctionDeclarationParameter(
                spec_node,
                function_node, 
                type_node,
                function_param_node,
                name_node,
                prescan));
    }
}

// Walks the children of the PROPERTIES node child of the SPEC node.
template <typename ReturnType>
void AstWalker<ReturnType>::WalkSpecPropertiesChildren(Node* spec_node, Node* properties_node) {
    GS_ASSERT_NODE_RULE(spec_node, NodeRule::SPEC);
    GS_ASSERT_NODE_RULE(properties_node, NodeRule::PROPERTIES);

    // Iterate through all properties in the SPEC's PROPERTIES node.
    for (size_t i = 0; i < properties_node->child_count(); i++) {
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

    GS_ASSERT_OPTIONAL_NODE_RULE(spec_node, NodeRule::SPEC);
    GS_ASSERT_OPTIONAL_NODE_RULE(property_node, NodeRule::PROPERTY);
    GS_ASSERT_TRUE(property_node->child_count() == 4,
        "AstWalker expects PROPERTY to have 4 children");

    // Get all mandatory child nodes.
    Node* type_node = property_node->child(0);
    Node* name_node = property_node->child(1);
    Node* get_property_function_node = property_node->child(2);
    Node* set_property_function_node = property_node->child(3);
    Node* get_access_modifier_node = get_property_function_node->child(0);
    Node* set_access_modifier_node = set_property_function_node->child(0);

    // Check mandatory child node types.
    GS_ASSERT_NODE_RULE(type_node, NodeRule::TYPE);
    GS_ASSERT_NODE_RULE(name_node, NodeRule::NAME);
    GS_ASSERT_NODE_RULE(get_property_function_node, NodeRule::PROPERTY_FUNCTION);
    GS_ASSERT_NODE_RULE(set_property_function_node, NodeRule::PROPERTY_FUNCTION);
    GS_ASSERT_NODE_RULE(get_access_modifier_node, NodeRule::ACCESS_MODIFIER);
    GS_ASSERT_NODE_RULE(set_access_modifier_node, NodeRule::ACCESS_MODIFIER);

    // Get the property function block nodes if they were provided.
    // These nodes are optional. If they are not present it implies that the
    // property is an auto (as opposed to explicit) property.
    Node* get_block_node = NULL;
    Node* set_block_node = NULL;
    if (get_property_function_node->child_count() >= 2) {
        get_block_node = get_property_function_node->child(1);
        GS_ASSERT_NODE_RULE(get_block_node, NodeRule::BLOCK);
    }
    if (set_property_function_node->child_count() >= 2) {
        set_block_node = set_property_function_node->child(1);
        GS_ASSERT_NODE_RULE(set_block_node, NodeRule::BLOCK);
    }

    // Dispatch to subclass function.
    WalkSpecPropertyDeclaration(
        spec_node,
        type_node,
        name_node,
        get_property_function_node,
        set_property_function_node,
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

    GS_ASSERT_OPTIONAL_NODE_RULE(spec_node, NodeRule::SPEC);
    GS_ASSERT_OPTIONAL_NODE_RULE(function_node, NodeRule::FUNCTION);
    GS_ASSERT_OPTIONAL_NODE_RULE(property_node, NodeRule::PROPERTY);
    GS_ASSERT_NODE_RULE(block_node, NodeRule::BLOCK);

    // Iterate through all statements in the block.
    for (size_t i = 0; i < block_node->child_count(); i++) {
        Node* statement_node = block_node->child(i);

        switch (statement_node->rule())
        {
        case NodeRule::CALL:
            WalkFunctionCallChildren(spec_node, function_node, statement_node);
            break;
        case NodeRule::IF:
            WalkIfStatementChildren(
                spec_node,
                function_node,
                property_node,
                property_function,
                statement_node,
                arguments_result);
            break;
        case NodeRule::FOR:
            WalkForStatementChildren(
                spec_node,
                function_node,
                property_node,
                property_function,
                statement_node,
                arguments_result);
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
            THROW_NOT_IMPLEMENTED();
        }
    }
}

// Walks through a CALL Node's children.
template <typename ReturnType>
ReturnType AstWalker<ReturnType>::WalkFunctionCallChildren(
    Node* spec_node, 
    Node* function_node, 
    Node* call_node) {
    
    GS_ASSERT_OPTIONAL_NODE_RULE(spec_node, NodeRule::SPEC);
    GS_ASSERT_OPTIONAL_NODE_RULE(function_node, NodeRule::FUNCTION);
    GS_ASSERT_NODE_RULE(call_node, NodeRule::CALL);

    Node* name_node = call_node->child(0);
    Node* arguments_node = call_node->child(1);
    std::vector<ReturnType> arguments_result;

    GS_ASSERT_NODE_RULE(name_node, NodeRule::NAME);
    GS_ASSERT_NODE_RULE(arguments_node, NodeRule::CALL_PARAMETERS);

    // Iterate through all call param expressions and evaluate their
    // types.
    for (size_t i = 0; i < arguments_node->child_count(); i++) {
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
    return WalkFunctionCall(spec_node, name_node, call_node, arguments_result);
}

// Walks through an IF Node's children.
template <typename ReturnType>
void AstWalker<ReturnType>::WalkIfStatementChildren(
    Node* spec_node,
    Node* function_node,
    Node* property_node,
    PropertyFunction property_function,
    Node* if_node,
    std::vector<ReturnType>* arguments_result) {

    GS_ASSERT_OPTIONAL_NODE_RULE(spec_node, NodeRule::SPEC);
    GS_ASSERT_OPTIONAL_NODE_RULE(function_node, NodeRule::FUNCTION);
    GS_ASSERT_OPTIONAL_NODE_RULE(property_node, NodeRule::PROPERTY);
    GS_ASSERT_NODE_RULE(if_node, NodeRule::IF);

    // Walk condition expression.
    ReturnType condition_result = WalkExpressionChildren(
        spec_node,
        function_node,
        property_node,
        property_function,
        if_node->child(0));

    // Walk true block.
    // TODO: why is arguments_result passed to this and other block children?????
    WalkBlockChildren(
        spec_node,
        function_node,
        property_node,
        property_function,
        if_node->child(1),
        arguments_result);

    // Walk false block.
    WalkBlockChildren(
        spec_node,
        function_node,
        property_node,
        property_function,
        if_node->child(2),
        arguments_result);

    // Dispatch to subclass.
    WalkIfStatement(spec_node, if_node, condition_result);
}

// Walks through a FOR Node's children.
template <typename ReturnType>
void AstWalker<ReturnType>::WalkForStatementChildren(
    Node* spec_node,
    Node* function_node,
    Node* property_node,
    PropertyFunction property_function,
    Node* for_node,
    std::vector<ReturnType>* arguments_result) {

    GS_ASSERT_OPTIONAL_NODE_RULE(spec_node, NodeRule::SPEC);
    GS_ASSERT_OPTIONAL_NODE_RULE(function_node, NodeRule::FUNCTION);
    GS_ASSERT_OPTIONAL_NODE_RULE(property_node, NodeRule::PROPERTY);
    GS_ASSERT_NODE_RULE(for_node, NodeRule::FOR);
    GS_ASSERT_TRUE(for_node->child_count() == 4, "AstWalker expects FOR to have 4 children");

    // Walk initialize expression.
    Node* init_node = for_node->child(0);
    GS_ASSERT_NODE_RULE(init_node, NodeRule::LOOP_INITIALIZE);

    // Init expression is optional.
    if (init_node->child_count() > 0) {
        WalkExpressionChildren(
            spec_node,
            function_node,
            property_node,
            property_function,
            init_node->child(0));
    }

    // Walk condition expression.
    Node* cond_node = for_node->child(1);
    GS_ASSERT_NODE_RULE(cond_node, NodeRule::LOOP_CONDITION);

    if (cond_node->child_count() > 0) {
        ReturnType cond_result = WalkExpressionChildren(
            spec_node,
            function_node,
            property_node,
            property_function,
            cond_node->child(0));

        // Dispatch to child class.
        WalkForStatement(
            spec_node,
            for_node,
            cond_result);
    }

    // Walk update expression.
    Node* update_node = for_node->child(2);
    GS_ASSERT_NODE_RULE(update_node, NodeRule::LOOP_UPDATE);

    if (update_node->child_count() > 0) {
        WalkExpressionChildren(
            spec_node,
            function_node,
            property_node,
            property_function,
            update_node->child(0));
    }

    // Walk for loop condition true block.
    WalkBlockChildren(
        spec_node,
        function_node,
        property_node,
        property_function,
        for_node->child(3),
        arguments_result);
}

// Walks through an ASSIGN Node's children.
template <typename ReturnType>
ReturnType AstWalker<ReturnType>::WalkAssignChildren(
    Node* spec_node,
    Node* function_node,
    Node* property_node,
    PropertyFunction property_function,
    Node* assign_node) {
    GS_ASSERT_OPTIONAL_NODE_RULE(spec_node, NodeRule::SPEC);
    GS_ASSERT_NODE_RULE(assign_node, NodeRule::ASSIGN);
    GS_ASSERT_TRUE(assign_node->child_count() == 2,
        "AstWalker expects ASSIGN to have 2 children");
    
    Node* symbol_node = assign_node->child(0);
    GS_ASSERT_NODE_RULE(symbol_node, NodeRule::SYMBOL);
    GS_ASSERT_TRUE(symbol_node->child_count() == 1,
        "AstWalker expects SYMBOL to have 1 child");

    // OK, so, binary_operation_node is a bit weird. When ASSIGN is used as
    // a statement you would expect the value being assigned to be an
    // EXPRESSION node, but expressions are top level and assigns can
    // be embedded in an EXPRESSION, so instead the value being assigned
    // is simply any binary operation and has no specific NodeRule to check.
    Node* name_node = symbol_node->child(0);
    Node* binary_operation_node = assign_node->child(1);

    GS_ASSERT_NODE_RULE(name_node, NodeRule::NAME);

    // Walk the binary operation and obtain the result.
    ReturnType binary_operation_result = WalkSubExpressionChildren(
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
        symbol_node,
        assign_node,
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

    GS_ASSERT_OPTIONAL_NODE_RULE(spec_node, NodeRule::SPEC);
    GS_ASSERT_OPTIONAL_NODE_RULE(function_node, NodeRule::FUNCTION);
    GS_ASSERT_OPTIONAL_NODE_RULE(property_node, NodeRule::PROPERTY);
    GS_ASSERT_NODE_RULE(return_node, NodeRule::RETURN);

    Node* expression_node = return_node->child(0);
    GS_ASSERT_NODE_RULE(expression_node, NodeRule::EXPRESSION);

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

    GS_ASSERT_OPTIONAL_NODE_RULE(spec_node, NodeRule::SPEC);
    GS_ASSERT_OPTIONAL_NODE_RULE(function_node, NodeRule::FUNCTION);
    GS_ASSERT_OPTIONAL_NODE_RULE(property_node, NodeRule::PROPERTY);
    GS_ASSERT_NODE_RULE(expression_node, NodeRule::EXPRESSION);

    return WalkSubExpressionChildren(
        spec_node,
        function_node,
        property_node,
        property_function,
        expression_node->child(0));
}

// Walks top level subexpressions.
template <typename ReturnType>
ReturnType AstWalker<ReturnType>::WalkSubExpressionChildren(
    Node* spec_node,
    Node* function_node,
    Node* property_node,
    PropertyFunction property_function,
    Node* subexpression_node) {

    GS_ASSERT_OPTIONAL_NODE_RULE(spec_node, NodeRule::SPEC);
    GS_ASSERT_OPTIONAL_NODE_RULE(function_node, NodeRule::FUNCTION);
    GS_ASSERT_OPTIONAL_NODE_RULE(property_node, NodeRule::PROPERTY);

    switch (subexpression_node->rule()) {
    case NodeRule::CALL:
        return WalkFunctionCallChildren(
            spec_node,
            function_node,
            subexpression_node);
    case NodeRule::ASSIGN:
        return WalkAssignChildren(
            spec_node,
            function_node,
            property_node,
            property_function,
            subexpression_node);
    default:
        return WalkBinaryOperationChildren(
            spec_node,
            function_node,
            property_node,
            property_function,
            subexpression_node);
    }
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

    GS_ASSERT_OPTIONAL_NODE_RULE(spec_node, NodeRule::SPEC);
    GS_ASSERT_OPTIONAL_NODE_RULE(function_node, NodeRule::FUNCTION);
    GS_ASSERT_OPTIONAL_NODE_RULE(property_node, NodeRule::PROPERTY);

    // This node has children, treat it as a binary operation.
    if (binary_operation_node->child_count() == 2) {

        Node* left_node = binary_operation_node->child(0);
        Node* right_node = binary_operation_node->child(1);

        ReturnType left_result = WalkSubExpressionChildren(
            spec_node,
            function_node,
            property_node,
            property_function,
            left_node);

        ReturnType right_result = WalkSubExpressionChildren(
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
                binary_operation_node,
                left_node,
                right_node,
                left_result,
                right_result);
        case NodeRule::SUB:
            return WalkSub(
                spec_node,
                binary_operation_node,
                left_node,
                right_node,
                left_result,
                right_result);
        case NodeRule::MUL:
            return WalkMul(
                spec_node,
                binary_operation_node,
                left_node,
                right_node,
                left_result,
                right_result);
        case NodeRule::DIV:
            return WalkDiv(
                spec_node,
                binary_operation_node,
                left_node,
                right_node,
                left_result,
                right_result);
        case NodeRule::MOD:
            return WalkMod(
                spec_node,
                binary_operation_node,
                left_node,
                right_node,
                left_result,
                right_result);
        case NodeRule::LOGAND:
            return WalkLogAnd(
                spec_node,
                binary_operation_node,
                left_node,
                right_node,
                left_result,
                right_result);
        case NodeRule::LOGOR:
            return WalkLogOr(
                spec_node,
                binary_operation_node,
                left_node,
                right_node,
                left_result,
                right_result);
        case NodeRule::GREATER:
            return WalkGreater(
                spec_node,
                binary_operation_node,
                left_node,
                right_node,
                left_result,
                right_result);
        case NodeRule::EQUALS:
            return WalkEquals(
                spec_node,
                binary_operation_node,
                left_node,
                right_node,
                left_result,
                right_result);
        case NodeRule::NOT_EQUALS:
            return WalkNotEquals(
                spec_node,
                binary_operation_node,
                left_node,
                right_node,
                left_result,
                right_result);
        case NodeRule::LESS:
            return WalkLess(
                spec_node,
                binary_operation_node,
                left_node,
                right_node,
                left_result,
                right_result);
        case NodeRule::GREATER_EQUALS:
            return WalkGreaterEquals(
                spec_node,
                binary_operation_node,
                left_node,
                right_node,
                left_result,
                right_result);
        case NodeRule::LESS_EQUALS:
            return WalkLessEquals(
                spec_node,
                binary_operation_node,
                left_node,
                right_node,
                left_result,
                right_result);
        default:
            THROW_NOT_IMPLEMENTED();
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

    GS_ASSERT_OPTIONAL_NODE_RULE(spec_node, NodeRule::SPEC);
    GS_ASSERT_OPTIONAL_NODE_RULE(function_node, NodeRule::FUNCTION);
    GS_ASSERT_OPTIONAL_NODE_RULE(property_node, NodeRule::PROPERTY);

    switch (atomic_node->rule()) {
    case NodeRule::LOGNOT:
        return WalkLogNot(
            spec_node,
            atomic_node,
            atomic_node->child(0),
            WalkSubExpressionChildren(
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
            atomic_node,
            atomic_node->child(0));
    case NodeRule::ANY_TYPE:
        return WalkAnyType(
            spec_node,
            function_node,
            property_node,
            property_function,
            atomic_node);
    default:
        THROW_NOT_IMPLEMENTED();
    }
}

} // namespace compiler
} // namespace gunderscript
