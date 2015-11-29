// Gunderscript-2 Parse/AST Node
// (C) 2014 Christian Gunderman

#include "node.h"

namespace gunderscript {
namespace library {

// Constructs a new node with no children and the specified NodeRule.
Node::Node(NodeRule rule) {
  rule_ = rule;
  num_value_.int_value = 0;
  string_value_ = NULL;
}

// Constructs a new node with no children, the specified NodeRule, and a boolean value.
Node::Node(NodeRule rule, bool value) {
  rule_ = rule;
  num_value_.bool_value = value;
  string_value_ = NULL;
}

// Constructs a new node with no children, the specified NodeRule, and a long value.
Node::Node(NodeRule rule, long value) { 
  rule_ = rule;
  num_value_.int_value = value;
  string_value_ = NULL;
}

// Constructs a new node with no children, the specified NodeRule, and a double value.
Node::Node(NodeRule rule, double value) {
  rule_ = rule;
  num_value_.float_value = value;
  string_value_ = NULL;
}

// Constructs a new node with no children, the specified NodeRule, and a LexerSymbol.
Node::Node(NodeRule rule, LexerSymbol symbol) {
  rule_ = rule;
  num_value_.symbol_value = symbol;
  string_value_ = NULL;
}

// Constructs a new node with no children, the specified NodeRule, and a string value.
Node::Node(NodeRule rule, const std::string* value) { 
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
Node* Node::GetChild(int child) {
  if (this->child_count() > child) {
    return this->children_[child];
  }

  return NULL;
}

} // namespace library
} // namespace gunderscript