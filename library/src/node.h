// Gunderscript-2 Parse/AST Node
// (C) 2014 Christian Gunderman

#ifndef GUNDERSCRIPT_NODE__H__
#define GUNDERSCRIPT_NODE__H__

#include <string>
#include <vector>

namespace gunderscript {
namespace library {

enum class NodeRule {
  MODULE,
    NAME
};

class Node {
  // public:
  Node(NodeRule rule) { rule_ = rule; }
  Node(NodeRule rule, long value);
  Node(NodeRule rule, double value);
  Node(NodeRule rule, std::string& value);
  ~Node();
  void AddChild(Node& child);
  Node* GetChild (int child);
  int child_count() const { return this->children_.size(); }
  long long_value() const { return num_value_.int_value; }
  double float_value() const { return num_value_.float_value; }
  std::string* string_value() const { return string_value_; }
  NodeRule rule() { return rule_; }

 private:
  std::vector<Node*> children_;
  std::string* string_value_;

  union {
    long int_value;
    double float_value;
  } num_value_;
  NodeRule rule_;
};

} // namespace library
} // namespace gunderscript

#endif // GUNDERSCRIPT_NODE__H__