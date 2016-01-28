// Gunderscript-2 Debug Functions
// (C) 2015-2016 Christian Gunderman

#include <cstring>

#include "debug.h"

namespace gunderscript {
namespace cli {

// Prints the debug representation of the given token to the console.
void DebugPrintLexerToken(const LexerToken& token) {
    const int kMaxValueLen = 255;
    char value[kMaxValueLen] = "";

    switch (token.type) {
    case LexerTokenType::CHAR:
        snprintf(value, kMaxValueLen, "'%c'", token.char_const);
        break;
    case LexerTokenType::FLOAT:
        snprintf(value, kMaxValueLen, "%f", token.float_const);
        break;
    case LexerTokenType::INT:
        snprintf(value, kMaxValueLen, "%i", token.char_const);
        break;
    case LexerTokenType::ACCESS_MODIFIER:
    case LexerTokenType::KEYWORD:
    case LexerTokenType::SYMBOL:
        snprintf(value, kMaxValueLen, "%s", LexerSymbolString(token.symbol).c_str());
        break;
    case LexerTokenType::NAME:
    case LexerTokenType::STRING:
        snprintf(value, kMaxValueLen, "\"%s\"", token.string_const->c_str());
        break;
    default:
        // Indicative of a bug.
        printf("**UNKNOWN_LEXER_TOKEN**");
        return;
    }

    printf("%s: %s\n", LexerTokenTypeString(token.type).c_str(), value);
}

// Prints the debug representation of an abstract syntax tree node
// to the console along with all of its children.
// The target_node is marked with an arrow.
void DebugPrintNode(const Node* node) {

    if (node == NULL) {
        printf("** NULL NODE **");
    }

    // TODO: use C++ streams or otherwise eliminate unsafe typecasts from size_t to int.

    switch (node->rule()) {
    case NodeRule::NAME:
    case NodeRule::STRING:
        printf("%i:%s, \"%s\"\n",
            (int)node->child_count(),
            NodeRuleString(node->rule()).c_str(),
            node->string_value()->c_str());
        break;

    case NodeRule::TYPE:
        printf("%i:%s, %s\n",
            (int)node->child_count(),
            NodeRuleString(node->rule()).c_str(),
            node->string_value()->c_str());
        break;

    case NodeRule::ACCESS_MODIFIER:
        printf("%i:%s, %s\n",
            (int)node->child_count(),
            NodeRuleString(node->rule()).c_str(),
            LexerSymbolString(node->symbol_value()).c_str());
        break;

    case NodeRule::BOOL:
    case NodeRule::NATIVE:
        printf("%i:%s, %s\n",
            (int)node->child_count(),
            NodeRuleString(node->rule()).c_str(),
            node->bool_value() ? "true" : "false");
        break;

    case NodeRule::CHAR:
    case NodeRule::INT:
        printf("%i:%s, %lu\n",
            (int)node->child_count(),
            NodeRuleString(node->rule()).c_str(),
            node->int_value());
        break;

    case NodeRule::FLOAT:
        printf("%i:%s, %lf\n",
            (int)node->child_count(),
            NodeRuleString(node->rule()).c_str(),
            node->float_value());
        break;

    case NodeRule::SPEC:
    case NodeRule::FUNCTION:
    case NodeRule::PROPERTY_FUNCTION:
        printf("%i:%s, %s\n",
            (int)node->child_count(),
            NodeRuleString(node->rule()).c_str(),
            node->symbol() == NULL ? "[SYMBOL]" : node->symbol()->name().c_str());
        break;

    default:
        printf("%i:%s\n",
            (int)node->child_count(),
            NodeRuleString(node->rule()).c_str());
    }

    // Print child nodes.
    for (size_t i = 0; i < node->child_count(); i++) {
        DebugPrintNode(node->child(i));
    }
}

} // namespace cli
} // namespace gunderscript
