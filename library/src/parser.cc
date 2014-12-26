// Gunderscript-2 Parser
// (C) 2014 Christian Gunderman

#include "parser.h"

namespace gunderscript {
namespace library {

Node* Parser::ParseModule() {
  Node* module_node = new Node(NodeRule::MODULE);
  Node* depends_node = new Node(NodeRule::DEPENDS);
  Node* specs_node = new Node(NodeRule::SPECS);

  ParsePackageDeclaration(module_node);

  module_node->AddChild(depends_node);

  // Allow end of file after package.
  if (has_next()) {
    AdvanceNext();
    ParseDependsStatements(depends_node);
  }

  module_node->AddChild(specs_node);

  // Allow end of file after depends.
  if (has_next()) {
    ParseSpecDefinitions(specs_node);
  }

  return module_node;
}

void Parser::ParsePackageDeclaration(Node* node) {

  // Check "package" keyword.
  if (!AdvanceKeyword(LexerSymbol::PACKAGE)) {
    throw ParserMalformedPackageException(*this, PARSER_ERR_EXPECTED_PACKAGE);
  }

  // Check package name string.
  if (AdvanceNext()->type != LexerTokenType::STRING) {
    throw ParserMalformedPackageException(*this, PARSER_ERR_BAD_PACKAGE_NAME);
  }

  // Save package name string in first module child.
  node->AddChild(new Node(NodeRule::NAME, CurrentToken()->string_const));

  ParseSemicolon(node);
}

void Parser::ParseDependsStatements(Node* node) {

  // While in a series of "depends" statements, parse as "depends."
  while (CurrentKeyword(LexerSymbol::DEPENDS)) {
    ParseDependsStatement(node);

    // Allow empty file after depends statements.
    if (has_next()) {
      AdvanceNext();
    }
  }
}

void Parser::ParseDependsStatement(Node* node) {

  // Check "depends" statement.
  if (!CurrentKeyword(LexerSymbol::DEPENDS)) {
    throw ParserMalformedDependsException(*this, PARSER_ERR_MALFORMED_DEPENDS);
  }

  // Check for package name to import.
  if (AdvanceNext()->type != LexerTokenType::STRING) {
    throw ParserMalformedDependsException(*this, PARSER_ERR_MALFORMED_DEPENDS);
  }

  // Add package name to the depends tree node.
  node->AddChild(new Node(NodeRule::NAME, CurrentToken()->string_const));

  ParseSemicolon(node);
}

void Parser::ParseSemicolon(Node* node) {

  // Check for terminating semicolon.
  if (!AdvanceSymbol(LexerSymbol::SEMICOLON)) {
    throw ParserUnexpectedTokenException(*this, PARSER_ERR_EXPECTED_SEMICOLON);
  }
}

void Parser::ParseSpecDefinitions(Node* node) {
  while (has_next()) {
    ParseSpecDefinition(node);

    if (has_next()) {
      AdvanceNext();
    }
  }
}

void Parser::ParseSpecDefinition(Node* node) {
  Node* spec_node = new Node(NodeRule::SPEC);
  node->AddChild(spec_node);

  // Check access modifier.
  if (CurrentToken()->type != LexerTokenType::ACCESS_MODIFIER) {
    throw ParserMalformedSpecException(*this, PARSER_ERR_MALFORMED_SPEC);
  }

  // Check for "spec" keyword in declaration.
  if (!AdvanceKeyword(LexerSymbol::SPEC)) {
    throw ParserMalformedSpecException(*this, PARSER_ERR_MALFORMED_SPEC);
  }

  // Check for spec NAME.
  if (AdvanceNext()->type != LexerTokenType::NAME) {
    throw ParserMalformedSpecException(*this, PARSER_ERR_MALFORMED_SPEC);
  }

  spec_node->AddChild(new Node(NodeRule::NAME, CurrentToken()->string_const));

  // Check for opening curly brace.
  if (!AdvanceSymbol(LexerSymbol::LBRACE)) {
    throw ParserMalformedSpecException(*this, PARSER_ERR_MALFORMED_SPEC);
  }

  // Parse spec body.
  ParseSpecBody(spec_node);

  // Check for closing curly brace.
  if (!CurrentSymbol(LexerSymbol::RBRACE)) {
    throw ParserMalformedSpecException(*this, PARSER_ERR_MALFORMED_SPEC);
  }
}

void Parser::ParseSpecBody(Node* node) {
  Node* properties_node = new Node(NodeRule::PROPERTIES);
  Node* functions_node = new Node(NodeRule::FUNCTIONS);

  node->AddChild(functions_node);
  node->AddChild(properties_node);

  // Parse body elements until we reach the closing curly brace.
  while (!AdvanceSymbol(LexerSymbol::RBRACE)) {
    switch (CurrentToken()->type) {
      case LexerTokenType::ACCESS_MODIFIER:
        ParseFunction(functions_node);
        break;
      case LexerTokenType::TYPE:
        ParseProperty(properties_node);
        break;
      default:
        throw ParserMalformedSpecException(*this, PARSER_ERR_MALFORMED_SPEC);
    }
  }
}

void Parser::ParseProperty(Node* node) {
  Node* property_node = new Node(NodeRule::PROPERTY);
  node->AddChild(property_node);

  // Make sure first token is a TYPE.
  if (CurrentToken()->type != LexerTokenType::TYPE) {
    throw ParserMalformedPropertyException(*this, PARSER_ERR_MALFORMED_PROPERTY);
  }

  property_node->AddChild(new Node(NodeRule::TYPE, CurrentToken()->symbol));

  // Make sure second token is a NAME.
  if (AdvanceNext()->type != LexerTokenType::NAME) {
    throw ParserMalformedPropertyException(*this, PARSER_ERR_MALFORMED_PROPERTY);
  }

  property_node->AddChild(new Node(NodeRule::NAME, CurrentToken()->string_const));

  // Check for LBRACE
  if (!AdvanceSymbol(LexerSymbol::LBRACE)) {
    throw ParserMalformedPropertyException(*this, PARSER_ERR_MALFORMED_PROPERTY);
  }

  AdvanceNext();
  ParsePropertyBody(property_node);

  // Check for LBRACE
  if (!CurrentSymbol(LexerSymbol::RBRACE)) {
    throw ParserMalformedPropertyException(*this, PARSER_ERR_MALFORMED_PROPERTY);
  }
}

void Parser::ParsePropertyBody(Node* node) {
  Node* getter_node = new Node(NodeRule::PROPERTY_FUNCTION);
  Node* setter_node = new Node(NodeRule::PROPERTY_FUNCTION);

  node->AddChild(getter_node);
  node->AddChild(setter_node);

  // There can be at most 2 body functions (get/set), parse twice.
  // Functions return if there is no body function.
  ParsePropertyBodyFunction(getter_node, setter_node);
  ParsePropertyBodyFunction(getter_node, setter_node);
}

void Parser::ParsePropertyBodyFunction(Node* getter_node, Node* setter_node) {

  // If next token is RBRACE, no body function here, return.
  if (CurrentSymbol(LexerSymbol::RBRACE)) {
    return;
  }

  // We have a getter/setter here, Check for the access modifier.
  if (CurrentToken()->type != LexerTokenType::ACCESS_MODIFIER) {
    throw ParserMalformedPropertyException(*this, PARSER_ERR_MALFORMED_PROPERTY);
  }

  LexerSymbol access_modifier = CurrentToken()->symbol;

  // Determine node to receive output.
  Node* node = NULL;
  if (AdvanceKeyword(LexerSymbol::GET)) {
    node = getter_node;
  } else if (CurrentKeyword(LexerSymbol::SET)) {
    node = setter_node;
  } else {
    throw ParserMalformedPropertyException(*this, PARSER_ERR_MALFORMED_PROPERTY);
  }

  // Check if property body function was specified twice.
  if (node->child_count() > 0) {
    throw ParserMalformedPropertyException(*this, PARSER_ERR_MALFORMED_PROPERTY);
  }

  node->AddChild(new Node(NodeRule::ACCESS_MODIFIER, access_modifier));

  // If we found a semicolon, it's an auto property and we're done.
  if (AdvanceSymbol(LexerSymbol::SEMICOLON)) {
    AdvanceNext();
    return;
  }

  ParseFunctionBody(node);

  AdvanceNext();
}

void Parser::ParseFunction(Node* node) {
  Node* function_node = new Node(NodeRule::FUNCTION);
  node->AddChild(function_node);

  // Check for ACCESS_MODIFIER.
  if (CurrentToken()->type != LexerTokenType::ACCESS_MODIFIER) {
    throw ParserMalformedFunctionException(*this, PARSER_ERR_MALFORMED_FUNCTION);
  }

  function_node->AddChild(new Node(NodeRule::ACCESS_MODIFIER, CurrentToken()->symbol));

  // Check for NATIVE function token.
  bool native_function = false;
  if (AdvanceKeyword(LexerSymbol::NATIVE)) {
    native_function = true;
    AdvanceNext();
  }

  function_node->AddChild(new Node(NodeRule::NATIVE, native_function));

  // Check for TYPE for return type.
  if (CurrentToken()->type != LexerTokenType::TYPE) {
    throw ParserMalformedFunctionException(*this, PARSER_ERR_MALFORMED_FUNCTION);
  }

  function_node->AddChild(new Node(NodeRule::TYPE, CurrentToken()->symbol));

  // Check for NAME symbol type for function name.
  if (AdvanceNext()->type != LexerTokenType::NAME) {
    throw ParserMalformedFunctionException(*this, PARSER_ERR_MALFORMED_FUNCTION);
  }

  function_node->AddChild(new Node(NodeRule::NAME, CurrentToken()->string_const));

  // Check for opening parenthesis for parameters.
  if (!AdvanceSymbol(LexerSymbol::LPAREN)) {
    throw ParserMalformedFunctionException(*this, PARSER_ERR_MALFORMED_FUNCTION);
  }

  ParseFunctionParameters(function_node);

  // Check for closing parenthesis for params.
  if (!CurrentSymbol(LexerSymbol::RPAREN)) {
    throw ParserMalformedFunctionException(*this, PARSER_ERR_MALFORMED_FUNCTION);
  }

  // If we found a semicolon, function has no body, we're done.
  if (AdvanceSymbol(LexerSymbol::SEMICOLON)) {
    return;
  }

  ParseFunctionBody(function_node);
}

void Parser::ParseFunctionParameters(Node* node) {
  Node* parameters_node = new Node(NodeRule::PARAMETERS);
  node->AddChild(parameters_node);

  // Keep on parsing till we reach the close parenthesis.
  if (!AdvanceSymbol(LexerSymbol::RPAREN)) {
    ParseFunctionParameter(parameters_node);

    while (!AdvanceSymbol(LexerSymbol::RPAREN)) {

      // Check for comma.
      if (!CurrentSymbol(LexerSymbol::COMMA)) {
	throw ParserMalformedFunctionException(*this, PARSER_ERR_MALFORMED_FUNCTION);
      }

      AdvanceNext();
      ParseFunctionParameter(parameters_node);
    }
  }
}

void Parser::ParseFunctionParameter(Node* node) {
  Node* parameter_node = new Node(NodeRule::PARAMETER);
  node->AddChild(parameter_node);

  // Check for a parameter TYPE.
  if (CurrentToken()->type != LexerTokenType::TYPE) {
    throw ParserMalformedFunctionException(*this, PARSER_ERR_MALFORMED_FUNCTION);
  }

  parameter_node->AddChild(new Node(NodeRule::TYPE, CurrentToken()->symbol));

  // Check for a parameter NAME.
  if (AdvanceNext()->type != LexerTokenType::NAME) {
    throw ParserMalformedFunctionException(*this, PARSER_ERR_MALFORMED_FUNCTION);
  }

  parameter_node->AddChild(new Node(NodeRule::NAME, CurrentToken()->string_const));
}

void Parser::ParseFunctionBody(Node* node) {
  Node* block_node = new Node(NodeRule::BLOCK);
  node->AddChild(block_node);

  // No semicolon, this property function must have a body, check opening brace.
  if (!CurrentSymbol(LexerSymbol::LBRACE)) {
    throw ParserMalformedBodyException(*this, PARSER_ERR_MALFORMED_BODY);
  }

  // TODO: ParseFunctionBody.

  // Check closing brace.
  if (!AdvanceSymbol(LexerSymbol::RBRACE)) {
    throw ParserMalformedBodyException(*this, PARSER_ERR_MALFORMED_BODY);
  }
}

const LexerToken* Parser::AdvanceNext() {
  const LexerToken* token = this->lexer_.AdvanceNext();
  ThrowEOFIfNull(token);
  return token;
}

const LexerToken* Parser::CurrentToken() {
  const LexerToken* token = this->lexer_.current_token();
  ThrowEOFIfNull(token);
  return token;
}

const LexerToken* Parser::NextToken() {
  const LexerToken* token = this->lexer_.next_token();
  ThrowEOFIfNull(token);
  return token;
}

bool Parser::AdvanceSymbol(LexerSymbol symbol) {
  return AdvanceNext()->type == LexerTokenType::SYMBOL &&
      CurrentToken()->symbol == symbol;
}

bool Parser::CurrentSymbol(LexerSymbol symbol) {
  return CurrentToken()->type == LexerTokenType::SYMBOL &&
      CurrentToken()->symbol == symbol;
}

bool Parser::AdvanceType(LexerSymbol type) {
  return AdvanceNext()->type == LexerTokenType::TYPE &&
      CurrentToken()->symbol == type;
}

bool Parser::CurrentType(LexerSymbol type) {
  return CurrentToken()->type == LexerTokenType::TYPE &&
      CurrentToken()->symbol == type;
}

bool Parser::AdvanceKeyword(LexerSymbol keyword) {
  return AdvanceNext()->type == LexerTokenType::KEYWORD &&
      CurrentToken()->symbol == keyword;
}

bool Parser::CurrentKeyword(LexerSymbol keyword) {
  return CurrentToken()->type == LexerTokenType::KEYWORD &&
      CurrentToken()->symbol == keyword;
}

bool Parser::AdvanceAccessModifier(LexerSymbol am) {
  return AdvanceNext()->type == LexerTokenType::ACCESS_MODIFIER &&
      CurrentToken()->symbol == am;
}

bool Parser::CurrentAccessModifier(LexerSymbol am) {
  return CurrentToken()->type == LexerTokenType::ACCESS_MODIFIER &&
      CurrentToken()->symbol == am;
}

void Parser::ThrowEOFIfNull(const LexerToken* token) {
  if (token == NULL) {
    throw ParserEndOfFileException(*this);
  }
}

} // namespace library
} // namespace gunderscript
