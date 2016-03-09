// Gunderscript-2 Parse/AST Node
// (C) 2014 Christian Gunderman

#ifndef GUNDERSCRIPT_NODE__H__
#define GUNDERSCRIPT_NODE__H__

#include <cstddef>
#include <string>
#include <vector>

#include "lexer_resources.h"
#include "symbol.h"

namespace gunderscript {

// Abstract syntax tree entities that a node may represent in the syntax tree.
// POTENTIAL BUG BUG BUG: If you update this enum you must also update the array
// enum in the .cc file to be identical. If you don't things will break!
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
    IF,
    FOR,
    LOOP_INITIALIZE,
    LOOP_CONDITION,
    LOOP_UPDATE,

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
    STRING,

    // Special NodeRule that matches with any type in the SemanticAstWalker.
    // Acts like a NULL/int(0) and may have to be special cased.
    ANY_TYPE
};

// Maps a NodeRule enum value to its string representation.
const std::string NodeRuleString(NodeRule rule);

// The abstract syntax tree node object. Each Node is a subtree containing a single
// language construct. Child nodes represent properties of the language constructs
// or sub-constructs.
class Node {
public:
    Node(NodeRule rule, int line, int column);
    Node(NodeRule rule, int line, int column, bool value);
    Node(NodeRule rule, int line, int column, long value);
    Node(NodeRule rule, int line, int column, double value);
    Node(NodeRule rule, int line, int column, LexerSymbol symbol);
    Node(NodeRule rule, int line, int column, const std::string* value);
    ~Node();

    void AddChild(Node* child);
    Node* child(size_t child) const;
    size_t child_count() const { return this->children_.size(); }
    bool bool_value() const { return num_value_.bool_value; }
    long int_value() const { return num_value_.int_value; }
    double float_value() const { return num_value_.float_value; }
    LexerSymbol symbol_value() const { return num_value_.symbol_value; }
    const std::string* string_value() const { return string_value_; }
    NodeRule rule() const { return rule_; }
    int line() { return line_; }
    int column() { return column_; }

    void set_symbol(Symbol* symbol) {
        symbol_ = symbol;
    }
    const Symbol* symbol() const { return symbol_; }

private:
    std::vector<Node*> children_;
    const std::string* string_value_;
    const int line_;
    const int column_;

    union {
        bool bool_value;
        long int_value;
        double float_value;
        LexerSymbol symbol_value;
    } num_value_;
    NodeRule rule_;

    // Mutable Properties:

    Symbol* symbol_;
};

} // namespace gunderscript

#endif // GUNDERSCRIPT_NODE__H__
