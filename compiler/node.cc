// Gunderscript-2 Parse/AST Node
// (C) 2014-2016 Christian Gunderman

#include "gunderscript/exceptions.h"
#include "gunderscript/node.h"

namespace gunderscript {

// POTENTIAL BUG BUG BUG: If you update this array you must also update NodeRule
// enum in the header to be identical. If you don't things will break!
static const std::string kNodeRuleString[] = {
    "MODULE", "DEPENDS", "NAME", "TYPE", "ACCESS_MODIFIER", "SPECS", "SPEC",
    "PROPERTIES", "PROPERTY", "PROPERTY_FUNCTION", "FUNCTIONS", "FUNCTION",
    "NATIVE", "FUNCTION_PARAMETERS", "FUNCTION_PARAMETER", "BLOCK", "ASSIGN",
    "RETURN", "EXPRESSION", "MEMBER", "CALL", "CALL_PARAMETERS", "SYMBOL",
    "LOGOR", "LOGAND", "LOGNOT", "EQUALS", "NOT_EQUALS", "LESS", "LESS_EQUALS",
    "GREATER", "GREATER_EQUALS", "ADD", "SUB", "MUL", "DIV", "MOD", "BOOL", "INT",
    "FLOAT", "CHAR", "STRING", "ANY_TYPE"
};

// Gets a string representation of a NodeRule.
// NOTE: for this method to operate correctly both the string array
// of rules and the enum must match 1:1 and in the same order.
const std::string NodeRuleString(NodeRule rule) {
    return kNodeRuleString[(int)rule];
}

// Constructs a new node with no children and the specified NodeRule.
Node::Node(NodeRule rule, int line, int column) : line_(line), column_(column) {
    rule_ = rule;
    num_value_.int_value = 0;
    string_value_ = NULL;
}

// Constructs a new node with no children, the specified NodeRule, and a boolean value.
Node::Node(NodeRule rule, int line, int column, bool value) : line_(line), column_(column) {
    rule_ = rule;
    num_value_.bool_value = value;
    string_value_ = NULL;
}

// Constructs a new node with no children, the specified NodeRule, and a long value.
Node::Node(NodeRule rule, int line, int column, long value) : line_(line), column_(column) {
    rule_ = rule;
    num_value_.int_value = value;
    string_value_ = NULL;
}

// Constructs a new node with no children, the specified NodeRule, and a double value.
Node::Node(NodeRule rule, int line, int column, double value) : line_(line), column_(column) {
    rule_ = rule;
    num_value_.float_value = value;
    string_value_ = NULL;
}

// Constructs a new node with no children, the specified NodeRule, and a LexerSymbol.
Node::Node(NodeRule rule, int line, int column, LexerSymbol symbol) : line_(line), column_(column) {
    rule_ = rule;
    num_value_.symbol_value = symbol;
    string_value_ = NULL;
}

// Constructs a new node with no children, the specified NodeRule, and a string value.
Node::Node(NodeRule rule, int line, int column, const std::string* value) : line_(line), column_(column) {
    rule_ = rule;
    num_value_.int_value = 0;
    string_value_ = new std::string(*value);
}

// Destroys the current node and all of its data and registered children
// freeing all child nodes recursively.
Node::~Node() {
    if (string_value_ != NULL) {
        delete string_value_;
    }

    for (unsigned int i = 0; i < children_.size(); i++) {
        delete children_[i];
    }
}

// Registers a new child node for this node.
void Node::AddChild(Node* child) {
    this->children_.push_back(child);
}

// Gets a child from this node by its index (added order).
Node* Node::child(size_t child) const {
    if (this->child_count() > child) {
        return this->children_[child];
    }

    // Line and column are arbritrary since this is never thrown in
    // normal operation.
    THROW_EXCEPTION(
        1,
        1,
        STATUS_ILLEGAL_STATE);
}

} // namespace gunderscript
