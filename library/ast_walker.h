// Gunderscript-2 Abstract Syntax Tree Walker Class
// (C) 2015 Christian Gunderman

#ifndef GUNDERSCRIPT_AST_WALKER__H__
#define GUNDERSCRIPT_AST_WALKER__H__

#include <string>
#include <vector>

#include "node.h"

namespace gunderscript {
namespace library {

// Class for walking through the Abstract Syntax Tree Structure.
template<typename ReturnType>
class AstWalker {
public:
    AstWalker(Node& root) : root_(root) { }

    void Walk() { WalkModuleChildren(); }

protected:
    Node& root() const { return this->root_; }

    virtual void WalkModule(Node* module_node) = 0;
    virtual void WalkModuleName(Node* name_node) = 0;
    virtual void WalkModuleDependsName(Node* name_node) = 0;
    virtual void WalkSpecDeclaration(Node* access_modifier_node, Node* name_node) = 0;
    virtual void WalkSpecFunctionDeclaration(
        Node* spec_node,
        Node* access_modifier_node,
        Node* native_node,
        Node* type_node,
        Node* name_node,
        Node* block_node,
        std::vector<ReturnType>& argument_result) = 0;
    virtual ReturnType WalkSpecFunctionDeclarationParameter(
        Node* spec_node,
        Node* function_node,
        Node* type_node,
        Node* name_node) = 0;
    virtual void WalkSpecPropertyDeclaration(
        Node* spec_node,
        Node* type_node,
        Node* name_node,
        Node* get_access_modifier_node,
        Node* set_access_modifier_node) = 0;

private:
    Node& root_;

    void WalkModuleChildren();
    void WalkModuleDependsChildren(Node* depends_node);
    void WalkModuleSpecsChildren(Node* specs_node);
    void WalkSpec(Node* spec_node);
    void WalkSpecFunctionsChildren(Node* spec_node, Node* functions_node);
    void WalkSpecFunctionDeclarationParametersChildren(
        Node* spec_node,
        Node* function_node,
        Node* params_node,
        std::vector<ReturnType>& argument_result);
    void WalkSpecPropertiesChildren(Node* spec_node, Node* properties_node);
    void WalkBlockChildren(
        Node* spec_node,
        Node* function_node,
        Node* property_node,
        Node* block);
    void CheckNodeRule(Node* node, NodeRule rule);
};

// Instantiate template with LexerSymbol so we can unit test from external module
// TODO: ifdef DEBUG here compatible with CMAKE.
template class AstWalker<LexerSymbol>;

} // namespace library
} // namespace gunderscript

#endif // GUNDERSCRIPT_AST_WALKER__H__