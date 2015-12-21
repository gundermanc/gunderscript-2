// Gunderscript-2 Parse/AST Node
// (C) 2014-2015 Christian Gunderman

#include "ast_walker.h"

namespace gunderscript {
namespace library {

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
        throw IllegalStateException();
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

// Walks a SPEC node and it's children recursively defining a 
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
    WalkSpecFunctionsChildren(spec_node, functions_node);
    WalkSpecPropertiesChildren(spec_node, properties_node);
}

// Walks the children of the FUNCTION node child of the SPEC node.
template <typename ReturnType>
void AstWalker<ReturnType>::WalkSpecFunctionsChildren(Node* spec_node, Node* functions_node) {
    CheckNodeRule(spec_node, NodeRule::SPEC);
    CheckNodeRule(functions_node, NodeRule::FUNCTIONS);

    // Iterates through all function declarations in the spec.
    for (int i = 0; i < functions_node->child_count(); i++) {
        Node* function_node = functions_node->child(i);

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
            arguments_result);

        // Dispatch to subclass.
        WalkSpecFunctionDeclaration(
            spec_node,
            access_modifier_node,
            native_node,
            type_node,
            name_node,
            block_node,
            arguments_result);
    }
}

// Walks the children of the the FUNCTION_PARAMS node.
template <typename ReturnType>
void AstWalker<ReturnType>::WalkSpecFunctionDeclarationParametersChildren(
    Node* spec_node,
    Node* function_node, 
    Node* function_params_node,
    std::vector<ReturnType>& argument_result) {

    CheckNodeRule(spec_node, NodeRule::SPEC);
    CheckNodeRule(function_node, NodeRule::FUNCTION);
    CheckNodeRule(function_params_node, NodeRule::FUNCTION_PARAMETERS); 

    // Iterate all FUNCTION_PARAMETER nodes in the FUNCTION_PARAMETERS node
    // and dispatch to subclass function.
    for (int i = 0; i < function_params_node->child_count(); i++) {
        Node* function_param_node = function_params_node->child(i);

        CheckNodeRule(function_param_node, NodeRule::FUNCTION_PARAMETER);

        argument_result.push_back(
            WalkSpecFunctionDeclarationParameter(
                spec_node,
                function_node, 
                function_param_node));
    }
}

// Walks the children of the PROPERTIES node child of the SPEC node.
// TODO: complete this.
template <typename ReturnType>
void AstWalker<ReturnType>::WalkSpecPropertiesChildren(Node* spec_node, Node* properties_node) {
    throw NotImplementedException();
}

// Walks the Children of the BLOCK AST nodes.
// TODO: Complete this.
template <typename ReturnType>
void AstWalker<ReturnType>::WalkBlockChildren(Node* function_node, Node* block_node) {
    throw NotImplementedException();
}

// Checks that the given node is for the given rule. If it is not,
// throws IllegalStateException to indicate a bug somewhere.
template <typename ReturnType>
void AstWalker<ReturnType>::CheckNodeRule(Node* node, NodeRule rule) {
    if (node->rule() != rule) {
        throw IllegalStateException();
    }
}

} // namespace library
} // namespace gunderscript
