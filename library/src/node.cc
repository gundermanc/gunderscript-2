// Gunderscript-2 Parse/AST Node
// (C) 2014 Christian Gunderman

#include "node.h"

#include <cstdlib>

namespace gunderscript {
namespace library {

Node::Node(NodeRule rule, long value) { 
  rule_ = rule;
  num_value_.int_value = value;
  string_value_ = NULL;
}

Node::Node(NodeRule rule, double value) {
  rule_ = rule;
  num_value_.float_value = value;
  string_value_ = NULL;
}

Node::Node(NodeRule rule, LexerSymbol symbol) {
  rule_ = rule;
  num_value_.symbol_value = symbol;
  string_value_ = NULL;
}

Node::Node(NodeRule rule, const std::string* value) { 
  rule_ = rule;
  num_value_.int_value = 0;
  string_value_ = new std::string(*value);
}

Node::~Node() {
  if (string_value_ != NULL) {
    delete string_value_;
  }
}

void Node::AddChild(Node* child) {
  this->children_.push_back(child);
}

Node* Node::GetChild(int child) {
  if (this->child_count() > child) {
    return this->children_[child];
  }

  return NULL;
}

} // namespace library
} // namespace gunderscript
