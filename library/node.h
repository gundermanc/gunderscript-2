// Gunderscript-2 Parse/AST Node
// (C) 2014 Christian Gunderman

#ifndef GUNDERSCRIPT_NODE__H__
#define GUNDERSCRIPT_NODE__H__

#include <string>
#include <vector>

#include "lexer.h"

namespace gunderscript {
namespace library {

enum class NodeRule {
  MODULE,
    DEPENDS,
    NAME,
    TYPE,
    ACCESS_MODIFIER,
    SPECS,
    SPEC,
    PROPERTIES,
    PROPERTY,
    PROPERTY_FUNCTION,
    FUNCTIONS,
    FUNCTION,
    NATIVE,
    FUNCTION_PARAMETERS,
    FUNCTION_PARAMETER,
    BLOCK,

    ASSIGN,

    RETURN,

    EXPRESSION,
    MEMBER,
    CALL,
    CALL_PARAMETERS,
    SYMBOL,
    LOGOR,
    LOGAND,
    LOGNOT,

    EQUALS,
    NOT_EQUALS,
    LESS,
    LESS_EQUALS,
    GREATER,
    GREATER_EQUALS,

    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    BOOL,
    INT,
    FLOAT,
    CHAR,
    STRING
};

class Node {
 public:
  Node(NodeRule rule);
  Node(NodeRule rule, bool value);
  Node(NodeRule rule, long value);
  Node(NodeRule rule, double value);
  Node(NodeRule rule, LexerSymbol symbol);
  Node(NodeRule rule, const std::string* value);
  ~Node();
  void AddChild(Node* child);
  Node* GetChild (int child);
  int child_count() const { return this->children_.size(); }
  bool bool_value() const { return num_value_.bool_value; }
  long int_value() const { return num_value_.int_value; }
  double float_value() const { return num_value_.float_value; }
  LexerSymbol symbol_value() const { return num_value_.symbol_value; }
  const std::string* string_value() const { return string_value_; }
  NodeRule rule() { return rule_; }

 private:
  std::vector<Node*> children_;
  const std::string* string_value_;

  union {
    bool bool_value;
    long int_value;
    double float_value;
    LexerSymbol symbol_value;
  } num_value_;
  NodeRule rule_;
};

} // namespace library
} // namespace gunderscript

#endif // GUNDERSCRIPT_NODE__H__
