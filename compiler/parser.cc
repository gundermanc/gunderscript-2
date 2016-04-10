// Gunderscript-2 Parser
// (C) 2014-2016 Christian Gunderman

#include <functional>

#include "gs_assert.h"
#include "parser.h"

namespace gunderscript {
namespace compiler {

// Parses all input in the parser object's Lexer component into an abstract
// syntax tree of Nodes.
// NOTE: root node must be deleted upon completion or it will never be freed.
// Deleting the root node deletes ALL nodes in the tree recursively so deleting
// the root deletes the tree.
// Throws: Lexer or Parser exceptions from the respective headers if a problem
// is encountered with lexemes or syntax.
Node* Parser::Parse() {

    // Try to Parse the module. Upon failure, catch
    // exception, cleanup, and rethrow.
    try {
        return ParseModule();
    }
    catch (const Exception&) {
        delete module_node_;
        throw;
    }
}

// Parses a module (e.g.: a script).
// Throws: Lexer or Parser exceptions from the respective headers if a problem
// is encountered with lexemes or syntax.
Node* Parser::ParseModule() {
    module_node_ = new Node(
        NodeRule::MODULE,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number());
    ParsePackageDeclaration(module_node_);

    // 0 or more.
    ParseDependsStatements(module_node_);

    // 0 or more.
    ParseModuleBody(module_node_);

    return module_node_;
}

// Parses a package declaration from a file. Package declaration syntax is:
// package "MyPackage";
// Throws: Lexer or Parser exceptions from the respective headers if a problem
// is encountered with lexemes or syntax.
void Parser::ParsePackageDeclaration(Node* node) {

    GS_ASSERT_NODE_RULE(node, NodeRule::MODULE);

    // Check "package" keyword.
    if (!AdvanceKeyword(LexerSymbol::PACKAGE)) {
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_PARSER_MISSING_PACKAGE);
    }

    // Check package name string.
    if (AdvanceNext()->type != LexerTokenType::STRING) {
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_PARSER_INVALID_PACKAGE);
    }

    // Save package name string in first module child.
    node->AddChild(new Node(
        NodeRule::NAME,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number(),
        CurrentToken()->string_const));

    AdvanceNext();
    ParseSemicolon();
}

// Parses all depends statements from a file. Depends statements indicate that this
// file requires code in another file to run. Depends comes right after package declarations
// and may be from 0 to many.
// Throws: Lexer or Parser exceptions from the respective headers if a problem
// is encountered with lexemes or syntax.
void Parser::ParseDependsStatements(Node* node) {

    GS_ASSERT_NODE_RULE(node, NodeRule::MODULE);

    Node* depends_node = new Node(
        NodeRule::DEPENDS,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number());

    node->AddChild(depends_node);

    // While in a series of "depends" statements, parse as "depends."
    while (has_next() && AdvanceKeyword(LexerSymbol::DEPENDS)) {
        ParseDependsStatement(depends_node);
    }
}

// Parses single depends statements from a file. Depends statements indicate that this
// file requires code in another file to run. Looks like:
// depends "OtherScript";
// Throws: Lexer or Parser exceptions from the respective headers if a problem
// is encountered with lexemes or syntax.
void Parser::ParseDependsStatement(Node* node) {

    GS_ASSERT_NODE_RULE(node, NodeRule::DEPENDS);

    // Check "depends" statement.
    if (!CurrentKeyword(LexerSymbol::DEPENDS)) {
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_PARSER_MALFORMED_DEPENDS);
    }

    // Check for package name to import.
    if (AdvanceNext()->type != LexerTokenType::STRING) {
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_PARSER_MALFORMED_DEPENDS);
    }

    // Add package name to the depends tree node.
    node->AddChild(new Node(
        NodeRule::NAME,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number(),
        CurrentToken()->string_const));

    AdvanceNext();
    ParseSemicolon();
}

// Checks that current lexeme is a semicolon, such as at the end of a line.
// Throws: Parser exceptions if current lexeme isn't a semicolon.
void Parser::ParseSemicolon() {

    // Check for terminating semicolon.
    if (!CurrentSymbol(LexerSymbol::SEMICOLON)) {
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_PARSER_EXPECTED_SEMICOLON);
    }
}

// Parses all spec definitions (similar to a class definition, but currently without
// inheritance) or static function definitions (non-class functions).
// Throws: Lexer or Parser exceptions from the respective headers if a problem
// is encountered with lexemes or syntax.
void Parser::ParseModuleBody(Node* module_node) {

    GS_ASSERT_NODE_RULE(module_node, NodeRule::MODULE);

    Node* specs_node = new Node(
        NodeRule::SPECS,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number());
    module_node->AddChild(specs_node);

    Node* functions_node = new Node(
        NodeRule::FUNCTIONS,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number());
    module_node->AddChild(functions_node);

    // Addresses Github issue #64.
    if (CurrentToken()->type == LexerTokenType::ACCESS_MODIFIER && !has_next()) {
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_PARSER_EOF);
    }

    while (has_next()) {

        // Check if current token is an access modifier. If not, this is malformed syntax.
        if (CurrentToken()->type != LexerTokenType::ACCESS_MODIFIER) {
            THROW_EXCEPTION(
                this->lexer_.current_line_number(),
                this->lexer_.current_column_number(),
                STATUS_PARSER_MALFORMED_SPEC_OR_FUNC_ACCESS_MODIFIER_MISSING);
        }

        if (NextKeyword(LexerSymbol::SPEC)) {
            ParseSpecDefinition(specs_node);
        }
        else {
            ParseFunction(functions_node, false);
        }

        // TODO: refactor out this grossness. Would do now but it's proving to be
        // an unneccesary pain.
        if (has_next()) {
            AdvanceNext();
        }
    }
}

// Parses a single spec definition (similar to a class definition, but currently without
// inheritance). Looks like:
// public spec MyClassName { .....
// Throws: Lexer or Parser exceptions from the respective headers if a problem
// is encountered with lexemes or syntax.
void Parser::ParseSpecDefinition(Node* specs_node) {

    GS_ASSERT_NODE_RULE(specs_node, NodeRule::SPECS);

    Node* spec_node = new Node(
        NodeRule::SPEC,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number());
    specs_node->AddChild(spec_node);

    // Check access modifier.
    if (CurrentToken()->type != LexerTokenType::ACCESS_MODIFIER) {
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_PARSER_MALFORMED_SPEC_OR_FUNC_ACCESS_MODIFIER_MISSING);
    }

    spec_node->AddChild(new Node(
        NodeRule::ACCESS_MODIFIER,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number(),
        CurrentToken()->symbol));

    // Check for "spec" keyword in declaration.
    if (!AdvanceKeyword(LexerSymbol::SPEC)) {
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_PARSER_MALFORMED_SPEC_SPEC_KEYWORD_MISSING);
    }

    // Check for spec NAME.
    if (AdvanceNext()->type != LexerTokenType::NAME) {
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_PARSER_MALFORMED_SPEC_NAME_MISSING);
    }

    ParseTypeExpression(spec_node);

    // Check for opening curly brace.
    if (!CurrentSymbol(LexerSymbol::LBRACE)) {
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_PARSER_MALFORMED_SPEC_LBRACE_MISSING);
    }

    // Parse spec body.
    ParseSpecBody(spec_node);

    // Check for closing curly brace.
    if (!CurrentSymbol(LexerSymbol::RBRACE)) {
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_PARSER_MALFORMED_SPEC_RBRACE_MISSING);
    }
}

// Parses the body of a spec, including its functions and properties (stolen from C#).
// Body syntax is everything that follows the LBRACE up to the RBRACE and may contain
// only functions and properties. Fields are disallowed, fields must be implemented
// as properties.
// Throws: Lexer or Parser exceptions from the respective headers if a problem
// is encountered with lexemes or syntax.
void Parser::ParseSpecBody(Node* spec_node) {

    GS_ASSERT_NODE_RULE(spec_node, NodeRule::SPEC);

    Node* properties_node = new Node(
        NodeRule::PROPERTIES,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number());
    Node* functions_node = new Node(
        NodeRule::FUNCTIONS,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number());

    spec_node->AddChild(functions_node);
    spec_node->AddChild(properties_node);

    // Parse body elements until we reach the closing curly brace.
    while (!AdvanceSymbol(LexerSymbol::RBRACE)) {
        switch (CurrentToken()->type) {
        case LexerTokenType::ACCESS_MODIFIER:
            ParseFunction(functions_node, true);
            break;
        case LexerTokenType::NAME:
            ParseProperty(properties_node);
            break;
        default:
            THROW_EXCEPTION(
                this->lexer_.current_line_number(),
                this->lexer_.current_column_number(),
                STATUS_PARSER_MALFORMED_SPEC_UNKNOWN_MEMBER);
        }
    }
}

// Parses a single property definition.
// Looks like:
// int X { public get; concealed set; }
// Throws: Lexer or Parser exceptions from the respective headers if a problem
// is encountered with lexemes or syntax.
void Parser::ParseProperty(Node* properties_node) {

    GS_ASSERT_NODE_RULE(properties_node, NodeRule::PROPERTIES);

    Node* property_node = new Node(
        NodeRule::PROPERTY,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number());
    properties_node->AddChild(property_node);

    // Make sure first token is a TYPE.
    if (CurrentToken()->type != LexerTokenType::NAME) {
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_PARSER_MALFORMED_PROPERTY_TYPE_MISSING);
    }

    ParseTypeExpression(property_node);

    // Make sure second token is a NAME.
    if (CurrentToken()->type != LexerTokenType::NAME) {
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_PARSER_MALFORMED_PROPERTY_NAME_MISSING);
    }

    property_node->AddChild(new Node(
        NodeRule::NAME,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number(),
        CurrentToken()->string_const));

    // Check for LBRACE
    if (!AdvanceSymbol(LexerSymbol::LBRACE)) {
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_PARSER_MALFORMED_PROPERTY_LBRACE_MISSING);
    }

    AdvanceNext();
    ParsePropertyBody(property_node);

    // Check for RBRACE
    if (!CurrentSymbol(LexerSymbol::RBRACE)) {
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_PARSER_MALFORMED_PROPERTY_RBRACE_MISSING);
    }
}

// Parses between the braces of a property definition.
// Throws: Lexer or Parser exceptions from the respective headers if a problem
// is encountered with lexemes or syntax.
void Parser::ParsePropertyBody(Node* property_node) {

    GS_ASSERT_NODE_RULE(property_node, NodeRule::PROPERTY);

    Node* getter_node = new Node(
        NodeRule::PROPERTY_FUNCTION,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number());
    Node* setter_node = new Node(
        NodeRule::PROPERTY_FUNCTION,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number());

    property_node->AddChild(getter_node);
    property_node->AddChild(setter_node);

    // Giving an error message with the line number of the close brace is pointless
    // so lets save the line numbers of the first line of the property body instead.
    int start_property_body_line = this->lexer_.current_line_number();
    int start_property_body_column = this->lexer_.current_column_number();

    // There must be exactly 2 body functions (get/set), parse twice.
    // Functions return if there is no body function.
    ParsePropertyBodyFunction(getter_node, setter_node);
    ParsePropertyBodyFunction(getter_node, setter_node);

    // Properties MUST define both members.
    if (getter_node->child_count() == 0 || setter_node->child_count() == 0) {
        THROW_EXCEPTION(
            start_property_body_line,
            start_property_body_column,
            STATUS_PARSER_MALFORMED_PROPERTY_MISSING_PROPERTY_FUNCTION);
    }
}

// Parses a GET or SET function in a property body (see C# properties for info).
// Throws: Lexer or Parser exceptions from the respective headers if a problem
// is encountered with lexemes or syntax.
void Parser::ParsePropertyBodyFunction(Node* getter_node, Node* setter_node) {

    GS_ASSERT_NODE_RULE(getter_node, NodeRule::PROPERTY_FUNCTION);
    GS_ASSERT_NODE_RULE(setter_node, NodeRule::PROPERTY_FUNCTION);

    // If next token is RBRACE, no body function here, return.
    if (CurrentSymbol(LexerSymbol::RBRACE)) {
        return;
    }

    // We have a getter/setter here, Check for the access modifier.
    if (CurrentToken()->type != LexerTokenType::ACCESS_MODIFIER) {
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_PARSER_MALFORMED_PROPERTYFUNCTION_MISSING_ACCESS_MODIFIER);
    }

    LexerSymbol access_modifier = CurrentToken()->symbol;

    // Determine node to receive output.
    Node* node = NULL;
    if (AdvanceKeyword(LexerSymbol::GET)) {
        node = getter_node;
    }
    else if (CurrentKeyword(LexerSymbol::SET)) {
        node = setter_node;
    }
    else {
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_PARSER_MALFORMED_PROPERTYFUNCTION_INVALID_ACCESSORMUTATOR);
    }

    // Check if property body function was specified twice.
    if (node->child_count() > 0) {
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_PARSER_MALFORMED_PROPERTYFUNCTION_DUPLICATE);
    }

    node->AddChild(new Node(
        NodeRule::ACCESS_MODIFIER,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number(), 
        access_modifier));

    // If we found a semicolon, it's an auto property and we're done.
    if (AdvanceSymbol(LexerSymbol::SEMICOLON)) {
        AdvanceNext();
        return;
    }

    ParseBlockStatement(node);

    AdvanceNext();
}

// Parses a function definition.
// public int sqrt(int value) { ...
// Throws: Lexer or Parser exceptions from the respective headers if a problem
// is encountered with lexemes or syntax.
void Parser::ParseFunction(Node* node, bool in_spec) {
    Node* function_node = new Node(
        NodeRule::FUNCTION,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number());
    node->AddChild(function_node);

    // Check for ACCESS_MODIFIER.
    if (CurrentToken()->type != LexerTokenType::ACCESS_MODIFIER) {
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_PARSER_MALFORMED_FUNCTION_MISSING_ACCESS_MODIFIER);
    }

    function_node->AddChild(new Node(
        NodeRule::ACCESS_MODIFIER, 
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number(), 
        CurrentToken()->symbol));

    // Check if this is a constructor definition or a function definition.
    if (AdvanceKeyword(LexerSymbol::CONSTRUCT)) {

        // Disallow constructors outside of function.
        if (!in_spec) {
            THROW_EXCEPTION(
                this->lexer_.current_line_number(),
                this->lexer_.current_column_number(),
                STATUS_PARSER_CONSTRUCTOR_OUTSIDE_SPEC);
        }

        // Constructor functions are void type.
        function_node->AddChild(new Node(
            NodeRule::TYPE,
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            &TYPE_VOID.symbol_name()));

        // Name is mangled to be inaccessible from user code.
        function_node->AddChild(
            new Node(NodeRule::NAME,
                this->lexer_.current_line_number(),
                this->lexer_.current_column_number(),
                &kConstructorName));
    }
    else {
        // Check for TYPE for return type.
        if (CurrentToken()->type != LexerTokenType::NAME) {
            THROW_EXCEPTION(
                this->lexer_.current_line_number(),
                this->lexer_.current_column_number(),
                STATUS_PARSER_MALFORMED_FUNCTION_MISSING_TYPE);
        }

        ParseTypeExpression(function_node);

        // Check for NAME symbol type for function name.
        if (CurrentToken()->type != LexerTokenType::NAME) {
            THROW_EXCEPTION(
                this->lexer_.current_line_number(),
                this->lexer_.current_column_number(),
                STATUS_PARSER_MALFORMED_FUNCTION_MISSING_NAME);
        }

        function_node->AddChild(new Node(
            NodeRule::NAME,
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            CurrentToken()->string_const));
    }

    // Check for opening parenthesis for parameters.
    if (!AdvanceSymbol(LexerSymbol::LPAREN)) {
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_PARSER_MALFORMED_FUNCTION_MISSING_LPAREN);
    }

    ParseFunctionParameters(function_node);

    // Check for closing parenthesis for params.
    if (!CurrentSymbol(LexerSymbol::RPAREN)) {
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_PARSER_MALFORMED_FUNCTION_MISSING_RPAREN);
    }

    AdvanceNext();

    ParseBlockStatement(function_node);
}

// Parses function arguments.
// Throws: Lexer or Parser exceptions from the respective headers if a problem
// is encountered with lexemes or syntax.
void Parser::ParseFunctionParameters(Node* function_node) {

    GS_ASSERT_NODE_RULE(function_node, NodeRule::FUNCTION);

    Node* parameters_node = new Node(
        NodeRule::FUNCTION_PARAMETERS,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number());
    function_node->AddChild(parameters_node);

    // Keep on parsing till we reach the close parenthesis.
    if (!AdvanceSymbol(LexerSymbol::RPAREN)) {
        ParseFunctionParameter(parameters_node);

        while (!AdvanceSymbol(LexerSymbol::RPAREN)) {

            // Check for comma.
            if (!CurrentSymbol(LexerSymbol::COMMA)) {
                THROW_EXCEPTION(
                    this->lexer_.current_line_number(),
                    this->lexer_.current_column_number(),
                    STATUS_PARSER_MALFORMED_FUNCTIONPARAMS_MISSING_COMMA);
            }

            AdvanceNext();
            ParseFunctionParameter(parameters_node);
        }
    }
}

// Parses a single function parameter.
// Throws: Lexer or Parser exceptions from the respective headers if a problem
// is encountered with lexemes or syntax.
void Parser::ParseFunctionParameter(Node* parameters_node) {

    GS_ASSERT_NODE_RULE(parameters_node, NodeRule::FUNCTION_PARAMETERS);

    Node* parameter_node = new Node(
        NodeRule::FUNCTION_PARAMETER,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number());
    parameters_node->AddChild(parameter_node);

    // Check for a parameter TYPE.
    if (CurrentToken()->type != LexerTokenType::NAME) {
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_PARSER_MALFORMED_FUNCTIONPARAMS_MISSING_TYPE);
    }

    ParseTypeExpression(parameter_node);

    // Check for a parameter NAME.
    if (CurrentToken()->type != LexerTokenType::NAME) {
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_PARSER_MALFORMED_FUNCTIONPARAMS_MISSING_NAME);
    }

    parameter_node->AddChild(new Node(
        NodeRule::NAME,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number(),
        CurrentToken()->string_const));
}

// Parses a block of code from an open brace to a closed brace.
// Throws: Lexer or Parser exceptions from the respective headers if a problem
// is encountered with lexemes or syntax.
void Parser::ParseBlockStatement(Node* node) {

    GS_ASSERT_FALSE(node == NULL, "NULL node in ParseBlockStatement");

    Node* block_node = new Node(
        NodeRule::BLOCK,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number());
    node->AddChild(block_node);

    // Check opening brace.
    // TODO: this might never be hit. Re-evaluate whether or not they are need.
    if (!CurrentSymbol(LexerSymbol::LBRACE)) {
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_PARSER_MALFORMED_BLOCK_MISSING_LBRACE);
    }

    AdvanceNext();

    while (!CurrentSymbol(LexerSymbol::RBRACE)) {
        ParseStatement(block_node);
    }

    // Check closing brace.
    // TODO: this might never be hit. Re-evaluate whether or not they are need.
    if (!CurrentSymbol(LexerSymbol::RBRACE)) {
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_PARSER_MALFORMED_BLOCK_MISSING_RBRACE);
    }
}

// Parses any stand-alone statement in code (has side effects or control flow,
// not just an expression).
// Throws: Lexer or Parser exceptions from the respective headers if a problem
// is encountered with lexemes or syntax.
void Parser::ParseStatement(Node* node) {

    GS_ASSERT_FALSE(node == NULL, "NULL node in ParseStatement");

    switch (CurrentToken()->type) {
    case LexerTokenType::SYMBOL:
        // TODO: do we NEED to allow parenthesis in member expressions?
        if (CurrentSymbol(LexerSymbol::LBRACE)) {
            ParseBlockStatement(node);
            AdvanceNext();
            break;
        }
        // else: this may be a parenthesis in a member expression. e.g.: (x).y
        // and so we fall through to NAME.
    case LexerTokenType::NAME:
        ParseNameStatement(node);
        break;
    case LexerTokenType::KEYWORD:
        ParseKeywordStatement(node);
        break;
    default:
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_PARSER_EXPECTED_STATEMENT);
    }
}

// Parses a keyword statement (control flow structures and return statement).
// Throws: Lexer or Parser exceptions from the respective headers if a problem
// is encountered with lexemes or syntax.
void Parser::ParseKeywordStatement(Node* node) {

    GS_ASSERT_FALSE(node == NULL, "NULL node in ParseKeywordStatement");

    switch (CurrentToken()->symbol) {
    case LexerSymbol::IF:
        ParseIfStatement(node);
        break;
    case LexerSymbol::WHILE:
        ParseWhileStatement(node);
        break;
    case LexerSymbol::FOR:
        ParseForStatement(node);
        break;
    case LexerSymbol::RETURN:
        ParseReturnStatement(node);
        break;
    default:
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_PARSER_EXPECTED_STATEMENT_INVALID_KEYWORD);
    }
}

// Parses if statement and attaches the IF node to the given node.
void Parser::ParseIfStatement(Node* node) {
    GS_ASSERT_TRUE(CurrentKeyword(LexerSymbol::IF), "Expected IF token in if statement parser");

    AdvanceNext();

    // Create IF statement node.
    Node* if_node = new Node(
        NodeRule::IF,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number());
    node->AddChild(if_node);

    // Check for left parenthesis.
    if (!CurrentSymbol(LexerSymbol::LPAREN)) {
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_PARSER_MALFORMED_IF_MISSING_LPAREN);
    }

    AdvanceNext();

    // Parse if statement condition.
    ParseExpression(if_node);

    // Check for right parenthesis.
    if (!CurrentSymbol(LexerSymbol::RPAREN)) {
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_PARSER_MALFORMED_IF_MISSING_RPAREN);
    }

    AdvanceNext();

    // Parse the IF true body.
    ParseBlockStatement(if_node);

    AdvanceNext();

    // Parse the else or else if if there is one.
    ParseElIfStatement(if_node);
}

// Parses else or else if statement or returns if neither is given.
void Parser::ParseElIfStatement(Node* node) {
    GS_ASSERT_NODE_RULE(node, NodeRule::IF);

    // Check if an else / else if was provided.
    if (!CurrentKeyword(LexerSymbol::ELSE)) {

        // Still need an empty block to maintain the proper structure of the tree.
        Node* else_block_node = new Node(
            NodeRule::BLOCK,
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number());
        node->AddChild(else_block_node);
        return;
    }

    AdvanceNext();

    // Check if there is an else if or an else (false) block.
    if (CurrentKeyword(LexerSymbol::IF)) {
        // Create else body block to contain the if statement.
        Node* else_block_node = new Node(
            NodeRule::BLOCK,
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number());
        node->AddChild(else_block_node);

        // Parse the if statement into the else's block.
        ParseIfStatement(else_block_node);
    }
    else {
        // Parse the else block onto the last slot of the IF subtree.
        ParseBlockStatement(node);
        AdvanceNext();
    }
}

// Parses a while loop.
void Parser::ParseWhileStatement(Node* node) {
    GS_ASSERT_TRUE(CurrentKeyword(LexerSymbol::WHILE), "Expected WHILE token in for statement parser");

    // Create WHILE loop subtree root node.
    // WHILE loop is a syntactic sugar around the FOR loop parse tree.
    Node* while_node = new Node(
        NodeRule::FOR,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number());
    node->AddChild(while_node);

    // Create LOOP_INITIALIZE node.
    Node* init_node = new Node(
        NodeRule::LOOP_INITIALIZE,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number());
    while_node->AddChild(init_node);

    // Create LOOP_CONDITION node.
    Node* cond_node = new Node(
        NodeRule::LOOP_CONDITION,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number());
    while_node->AddChild(cond_node);

    // Create LOOP_UPDATE node.
    Node* update_node = new Node(
        NodeRule::LOOP_UPDATE,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number());
    while_node->AddChild(update_node);

    AdvanceNext();

    // Check for left parenthesis.
    if (!CurrentSymbol(LexerSymbol::LPAREN)) {
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_PARSER_MALFORMED_WHILE_MISSING_LPAREN);
    }

    // Parse loop expression.
    AdvanceNext();
    ParseExpression(cond_node);

    // Check for right parenthesis.
    if (!CurrentSymbol(LexerSymbol::RPAREN)) {
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_PARSER_MALFORMED_WHILE_MISSING_RPAREN);
    }

    AdvanceNext();

    // Parse the for loop body.
    ParseBlockStatement(while_node);

    AdvanceNext();
}

// Parses for loop.
void Parser::ParseForStatement(Node* node) {
    GS_ASSERT_TRUE(CurrentKeyword(LexerSymbol::FOR), "Expected FOR token in for statement parser");

    // Create FOR loop subtree root node.
    Node* for_node = new Node(
        NodeRule::FOR,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number());
    node->AddChild(for_node);

    // Create LOOP_INITIALIZE node.
    Node* init_node = new Node(
        NodeRule::LOOP_INITIALIZE,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number());
    for_node->AddChild(init_node);

    // Create LOOP_CONDITION node.
    Node* cond_node = new Node(
        NodeRule::LOOP_CONDITION,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number());
    for_node->AddChild(cond_node);

    // Create LOOP_UPDATE node.
    Node* update_node = new Node(
        NodeRule::LOOP_UPDATE,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number());
    for_node->AddChild(update_node);

    AdvanceNext();

    // Check for left parenthesis.
    if (!CurrentSymbol(LexerSymbol::LPAREN)) {
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_PARSER_MALFORMED_FOR_MISSING_LPAREN);
    }

    AdvanceNext();

    // Check for loop initialization statement
    if (!CurrentSymbol(LexerSymbol::SEMICOLON)) {
        ParseExpression(init_node);
    }

    ParseSemicolon();
    AdvanceNext();

    // Check for loop condition expression.
    if (!CurrentSymbol(LexerSymbol::SEMICOLON)) {
        ParseExpression(cond_node);
    }

    ParseSemicolon();
    AdvanceNext();

    // Check for loop update assign expression.
    if (!CurrentSymbol(LexerSymbol::RPAREN)) {
        ParseExpression(update_node);
    }

    // Check for right parenthesis.
    if (!CurrentSymbol(LexerSymbol::RPAREN)) {
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_PARSER_MALFORMED_FOR_MISSING_RPAREN);
    }

    AdvanceNext();

    // Parse the for loop body.
    ParseBlockStatement(for_node);

    AdvanceNext();
}

// Parses a return statement.
// Throws: Lexer or Parser exceptions from the respective headers if a problem
// is encountered with lexemes or syntax.
void Parser::ParseReturnStatement(Node* node) {

    GS_ASSERT_FALSE(node == NULL, "NULL node in ParseReturnStatement");
    GS_ASSERT_TRUE(CurrentKeyword(LexerSymbol::RETURN), "Expected RETURN in ParseBlockStatement");

    Node* return_node = new Node(
        NodeRule::RETURN,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number());
    node->AddChild(return_node);

    // Check for return expression.
    if (!AdvanceSymbol(LexerSymbol::SEMICOLON)) {
        ParseExpression(return_node);
    }

    ParseSemicolon();
    AdvanceNext();
}

// Parses a statement beginning with a name (a variable or function name) into either
// a function call or an assignment.
// Throws: Lexer or Parser exceptions from the respective headers if a problem
// is encountered with lexemes or syntax.
void Parser::ParseNameStatement(Node* node) {

    GS_ASSERT_FALSE(node == NULL, "NULL node in ParseNameStatement");
    GS_ASSERT_TRUE(CurrentToken()->type == LexerTokenType::NAME ||
        CurrentSymbol(LexerSymbol::LPAREN),
        "Expected NAME in ParseNameStatement");

    // Call the toplevel assign parser. Parser will fall through into subparsers
    // as necessary until we are left with either a call, an assignment, either by
    // itself or embedded inside of a member (object dereference) expression.
    Node* statement_node = ParseAssignExpressionA();
    node->AddChild(statement_node);

    AdvanceNext();

    // Check that evaulating the expression resulted in either a Call, Assign, or
    // one of the former as the last item in a member (object dereference) expression.
    if (statement_node->rule() == NodeRule::CALL ||
        (statement_node->rule() == NodeRule::MEMBER && 
            statement_node->child(1)->rule() == NodeRule::CALL) ||
        statement_node->rule() == NodeRule::ASSIGN) {
        return;
    }
    
    THROW_EXCEPTION(
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number(),
        STATUS_PARSER_INCOMPLETE_NAME_STATEMENT);
}

// Parses a function call.
// Throws: Lexer or Parser exceptions from the respective headers if a problem
// is encountered with lexemes or syntax.
void Parser::ParseCallStatement(Node* node) {

    GS_ASSERT_FALSE(node == NULL, "NULL node in ParseCallStatement");
    GS_ASSERT_TRUE(CurrentToken()->type == LexerTokenType::NAME,
        "Expected NAME in ParseCallStatement");
    GS_ASSERT_TRUE(NextSymbol(LexerSymbol::LPAREN),
        "Expected LPAREN in ParseCallStatement");

    node->AddChild(ParseCallExpression());
    ParseSemicolon();
    AdvanceNext();
}

// Parses a variable assignment statement.
// Throws: Lexer or Parser exceptions from the respective headers if a problem
// is encountered with lexemes or syntax.
void Parser::ParseAssignStatement(Node* node) {

    GS_ASSERT_FALSE(node == NULL, "NULL node in ParseAssignStatement");
    GS_ASSERT_TRUE(CurrentToken()->type == LexerTokenType::NAME,
        "Expected NAME in ParseAssignStatement");
    GS_ASSERT_TRUE(NextSymbol(LexerSymbol::ASSIGN),
        "Expected ASSIGN in ParseAssignStatement");

    node->AddChild(ParseAssignExpressionA());
    ParseSemicolon();
    AdvanceNext();
}

// Parses an expression.
// Throws: Lexer or Parser exceptions from the respective headers if a problem
// is encountered with lexemes or syntax.
void Parser::ParseExpression(Node* node) {

    GS_ASSERT_FALSE(node == NULL, "NULL node in ParseExpression");

    Node* expression_node = new Node(
        NodeRule::EXPRESSION,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number());
    node->AddChild(expression_node);

    expression_node->AddChild(ParseAssignExpressionA());
}

Node* Parser::ParseAssignExpressionA() {

    if (CurrentToken()->type == LexerTokenType::NAME &&
        NextSymbol(LexerSymbol::ASSIGN)) {
        return ParseAssignExpressionB(ParseVariableExpression());
    }
    else {
        return ParseOrExpression(ParseAndExpressionA());
    }
}

Node* Parser::ParseAssignExpressionB(Node* left_operand_node) {

    GS_ASSERT_FALSE(left_operand_node == NULL, "NULL node in ParseAssignExpressionB");

    if (CurrentToken()->symbol != LexerSymbol::ASSIGN) {
        return left_operand_node;
    }

    Node* operation_node = new Node(
        NodeRule::ASSIGN,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number());
    try {
        AdvanceNext();
        operation_node->AddChild(left_operand_node);
        operation_node->AddChild(ParseAssignExpressionA());
    }
    catch (const Exception&) {
        delete operation_node;
        throw;
    }

    return operation_node;
}

Node* Parser::ParseOrExpression(Node* left_operand_node) {

    GS_ASSERT_FALSE(left_operand_node == NULL,
        "NULL node in ParseOrExpression");

    // Return left operand if this isn't an operation.
    if (CurrentToken()->type != LexerTokenType::SYMBOL) {
        return left_operand_node;
    }

    if (CurrentToken()->symbol != LexerSymbol::LOGOR) {
        return left_operand_node;
    }

    Node* operation_node = new Node(
        NodeRule::LOGOR,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number());

    try {
        AdvanceNext();
        operation_node->AddChild(left_operand_node);
        operation_node->AddChild(ParseAndExpressionA());
    }
    catch (const Exception&) {
        delete operation_node;
        throw;
    }

    Node* parent_node = NULL;
    try {
        parent_node = ParseOrExpression(operation_node);
    }
    catch (const Exception&) {
        delete parent_node;
        throw;
    }

    return parent_node;
}

Node* Parser::ParseAndExpressionA() {
    return ParseAndExpressionB(ParseComparisonExpressionA());
}

Node* Parser::ParseAndExpressionB(Node* left_operand_node) {

    GS_ASSERT_FALSE(left_operand_node == NULL,
        "NULL node in ParseAndExpressionB");

    // Return left operand if this isn't an operation.
    if (CurrentToken()->type != LexerTokenType::SYMBOL) {
        return left_operand_node;
    }

    Node* operation_node = NULL;

    switch (CurrentToken()->symbol) {
    case LexerSymbol::LOGAND:
        operation_node = new Node(
            NodeRule::LOGAND,
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number());
        break;
    default:
        // Return left operand if this isn't an operation.
        return left_operand_node;
    }

    Node* parent_node = NULL;

    try {
        AdvanceNext();
        operation_node->AddChild(left_operand_node);
        operation_node->AddChild(ParseComparisonExpressionA());
        parent_node = ParseAndExpressionB(operation_node);
    }
    catch (const Exception&) {
        delete operation_node;
        throw;
    }

    return parent_node;
}

Node* Parser::ParseComparisonExpressionA() {
    return ParseComparisonExpressionB(ParsePrimaryExpressionA());
}

Node* Parser::ParseComparisonExpressionB(Node* left_operand_node) {

    GS_ASSERT_FALSE(left_operand_node == NULL,
        "NULL node in ParseComparisonExpressionB");

    // Return left operand if this isn't an operation.
    if (CurrentToken()->type != LexerTokenType::SYMBOL) {
        return left_operand_node;
    }

    Node* operation_node = NULL;

    switch (CurrentToken()->symbol) {
    case LexerSymbol::EQUALS:
        operation_node = new Node(
            NodeRule::EQUALS,
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number());
        break;
    case LexerSymbol::NOTEQUALS:
        operation_node = new Node(
            NodeRule::NOT_EQUALS,
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number());
        break;
    case LexerSymbol::LESS:
        operation_node = new Node(
            NodeRule::LESS,
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number());
        break;
    case LexerSymbol::LESSEQUALS:
        operation_node = new Node(
            NodeRule::LESS_EQUALS,
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number());
        break;
    case LexerSymbol::GREATER:
        operation_node = new Node(
            NodeRule::GREATER,
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number());
        break;
    case LexerSymbol::GREATEREQUALS:
        operation_node = new Node(
            NodeRule::GREATER_EQUALS,
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number());
        break;
    default:
        // Return left operand if this isn't an operation.
        return left_operand_node;
    }

    Node* parent_node = NULL;

    try {
        AdvanceNext();
        operation_node->AddChild(left_operand_node);
        operation_node->AddChild(ParsePrimaryExpressionA());
        parent_node = ParseComparisonExpressionB(operation_node);
    }
    catch (const Exception&) {
        delete operation_node;
        throw;
    }

    return parent_node;
}

Node* Parser::ParsePrimaryExpressionA() {
    return ParsePrimaryExpressionB(ParseSecondaryExpressionA());
}

Node* Parser::ParsePrimaryExpressionB(Node* left_operand_node) {

    GS_ASSERT_FALSE(left_operand_node == NULL,
        "NULL node in ParsePrimaryExpressionB");

    // Return left operand if this isn't an operation.
    if (CurrentToken()->type != LexerTokenType::SYMBOL) {
        return left_operand_node;
    }

    Node* operation_node = NULL;

    switch (CurrentToken()->symbol) {
    case LexerSymbol::ADD:
        operation_node = new Node(
            NodeRule::ADD,
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number());
        break;
    case LexerSymbol::SUB:
        operation_node = new Node(
            NodeRule::SUB,
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number());
        break;
    default:
        // Return left operand if this isn't an operation.
        return left_operand_node;
    }

    Node* parent_node = NULL;

    try {
        AdvanceNext();
        operation_node->AddChild(left_operand_node);
        operation_node->AddChild(ParseSecondaryExpressionA());
        parent_node = ParsePrimaryExpressionB(operation_node);
    }
    catch (const Exception&) {
        delete operation_node;
        throw;
    }

    return parent_node;
}

Node* Parser::ParseSecondaryExpressionA() {
    return ParseSecondaryExpressionB(ParseTertiaryExpressionA());
}

Node* Parser::ParseSecondaryExpressionB(Node* left_operand_node) {

    GS_ASSERT_FALSE(left_operand_node == NULL,
        "NULL node in ParseSecondaryExpressionB");

    // Return left operand if this isn't an operation.
    if (CurrentToken()->type != LexerTokenType::SYMBOL) {
        return left_operand_node;
    }

    Node* operation_node = NULL;

    switch (CurrentToken()->symbol) {
    case LexerSymbol::MUL:
        operation_node = new Node(
            NodeRule::MUL,
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number());
        break;
    case LexerSymbol::DIV:
        operation_node = new Node(
            NodeRule::DIV,
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number());
        break;
    case LexerSymbol::MOD:
        operation_node = new Node(
            NodeRule::MOD,
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number());
        break;
    default:
        // Return left operand if this isn't an operation.
        return left_operand_node;
    }

    Node* parent_node = NULL;

    try {
        AdvanceNext();
        operation_node->AddChild(left_operand_node);
        operation_node->AddChild(ParseTertiaryExpressionA());
        parent_node = ParseSecondaryExpressionB(operation_node);
    }
    catch (const Exception&) {
        delete operation_node;
        throw;
    }

    return parent_node;
}

Node* Parser::ParseTertiaryExpressionA() {
    return ParseTertiaryExpressionB(ParseInvertExpression());
}

Node* Parser::ParseTertiaryExpressionB(Node* left_operand_node) {

    GS_ASSERT_FALSE(left_operand_node == NULL,
        "NULL node in ParseTertiaryExpressionB");

    // Return left operand if this isn't an operation.
    if (CurrentToken()->type != LexerTokenType::SYMBOL) {
        return left_operand_node;
    }

    Node* operation_node = NULL;

    switch (CurrentToken()->symbol) {
    case LexerSymbol::DOT:
        operation_node = new Node(
            NodeRule::MEMBER,
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number());
        break;
    case LexerSymbol::ASSIGN:
        operation_node = new Node(
            NodeRule::ASSIGN,
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number());
        break;
    default:
        // Return left operand if this isn't an operation.
        return left_operand_node;
    }

    Node* parent_node = NULL;

    try {
        AdvanceNext();
        operation_node->AddChild(left_operand_node);
        operation_node->AddChild(ParseInvertExpression());
        parent_node = ParseTertiaryExpressionB(operation_node);
    }
    catch (const Exception&) {
        delete operation_node;
        throw;
    }

    // Check to make sure that we aren't assigning to a function call.
    if (operation_node->rule() == NodeRule::ASSIGN &&
        (operation_node->child(0)->rule() == NodeRule::CALL ||
        (operation_node->child(0)->rule() == NodeRule::MEMBER &&
            operation_node->child(0)->child(1)->rule() != NodeRule::SYMBOL))) {
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_PARSER_INCOMPLETE_NAME_STATEMENT);;
    }

    return parent_node;
}

Node* Parser::ParseInvertExpression() {

    if (CurrentToken()->type != LexerTokenType::SYMBOL) {
        return ParseAtomicExpression();
    }

    Node* invert_node = NULL;

    switch (CurrentToken()->symbol) {
    case LexerSymbol::SUB:
        // Add NEGATE node if there is a '-'.
        invert_node = new Node(
            NodeRule::SUB,
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number());
        invert_node->AddChild(new Node(
            NodeRule::ANY_TYPE,
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            0l));
        break;

    case LexerSymbol::LOGNOT:
        // Add NOT node if there is a '!'.
        invert_node = new Node(
            NodeRule::LOGNOT,
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number());
        break;
    default:
        return ParseAtomicExpression();
    }

    // If we got this far without returning it means that we found either a '-' number
    // or '!' boolean, so Advance to the next symbol and recurse into this function again
    // instead of ParseAtomicExpression() in case there is another '-' or '!'.
    try {
        AdvanceNext();
        invert_node->AddChild(ParseInvertExpression());
    }
    catch (const Exception&) {
        delete invert_node;
        throw;
    }

    return invert_node;
}

Node* Parser::ParseAtomicExpression() {

    // If no LPAREN, evaluate value of expression.
    if (!CurrentSymbol(LexerSymbol::LPAREN)) {
        return ParseValueExpression();
    }

    // LPAREN, probably a "(" Expr ")".
    AdvanceNext();
    Node* node = ParseAssignExpressionA();

    // Check for closing parenthesis.
    if (!CurrentSymbol(LexerSymbol::RPAREN)) {
        delete node;
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_PARSER_MALFORMED_EXPRESSION_MISSING_RPAREN);
    }

    AdvanceNext();

    return node;
}

Node* Parser::ParseValueExpression() {
    switch (CurrentToken()->type) {
    case LexerTokenType::NAME:
        return ParseNamedValueExpression();
    case LexerTokenType::KEYWORD:
        switch (CurrentToken()->symbol)
        {
        case LexerSymbol::NEW:
            return ParseNewExpression();
        case LexerSymbol::DEFAULT:
            return ParseDefaultExpression();
        default:
            return ParseBoolConstant();
        }
    case LexerTokenType::INT:
        return ParseIntConstant();
    case LexerTokenType::FLOAT:
        return ParseFloatConstant();
    case LexerTokenType::CHAR:
        return ParseCharConstant();
    case LexerTokenType::STRING:
        return ParseStringConstant();
    default:
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_PARSER_MALFORMED_EXPRESSION_INVALID_TOKEN);
    }
}

Node* Parser::ParseNamedValueExpression() {

    GS_ASSERT_TRUE(CurrentToken()->type == LexerTokenType::NAME,
        "Expected NAME in ParseNamedValueExpression");

    if (NextToken()->type != LexerTokenType::SYMBOL) {
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_PARSER_MALFORMED_EXPRESSION_INVALID_TOKEN);
    }

    switch (NextToken()->symbol) {
    case LexerSymbol::LPAREN:
        return ParseCallExpression();
    default:
        return ParseVariableExpression();
    }
}

// Parses a type expression of the form GenericType<Param1, Param2, ...>
void Parser::ParseTypeExpression(Node* parent_node) {

    // This check is an assert because it is usually done by the caller
    // for a more specific error message.
    GS_ASSERT_TRUE(CurrentToken()->type == LexerTokenType::NAME,
        "Parser expected NAME in ParseTypeExpression");

    Node* type_node = new Node(
        NodeRule::TYPE,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number(),
        CurrentToken()->string_const);
    parent_node->AddChild(type_node);

    // Check for open angle brace <
    // Type parameters are optional and only required for generic types.
    // The correctness of these is checked in the SemanticAstWalker.
    if (NextSymbol(LexerSymbol::LESS)) {
        AdvanceNext();
        AdvanceNext();

        int line = this->lexer_.current_line_number();
        int column = this->lexer_.current_column_number();

        // Parse a single type argument.
        std::function<void()> parse_argument_lambda = [this, type_node, line, column]() {

            // Check first param is of type name.
            if (CurrentToken()->type != LexerTokenType::NAME) {
                THROW_EXCEPTION(line, column, STATUS_PARSER_MALFORMED_TYPE_PARAM_MISSING_NAME);
            }

            // Recursively build tree for type's first param type. Recursion is necessary
            // for nested generics.
            // e.g.: List<List<int32>>
            ParseTypeExpression(type_node);
        };

        parse_argument_lambda();

        // Parse the 2nd, 3rd, ... type params.
        while (!CurrentSymbol(LexerSymbol::GREATER)) {

            // Check for comma delimiter.
            if (!CurrentSymbol(LexerSymbol::COMMA)) {
                THROW_EXCEPTION(
                    this->lexer_.current_line_number(),
                    this->lexer_.current_column_number(),
                    STATUS_PARSER_MALFORMED_TYPE_PARAM_MISSING_COMMA);
            }

            AdvanceNext();

            parse_argument_lambda();
        }

        // Check for close angle brace >
        if (!CurrentSymbol(LexerSymbol::GREATER)) {
            THROW_EXCEPTION(
                this->lexer_.current_line_number(),
                this->lexer_.current_column_number(),
                STATUS_PARSER_MALFORMED_TYPE_PARAM_MISSING_GREATER);
        }
    }

    AdvanceNext();
}

Node* Parser::ParseCallExpression() {

    GS_ASSERT_TRUE(CurrentToken()->type == LexerTokenType::NAME,
        "Expected NAME in ParseCallExpression");
    GS_ASSERT_TRUE(NextSymbol(LexerSymbol::LPAREN),
        "Expected NAME in ParseCallExpression");

    Node* function_node = new Node(
        NodeRule::CALL,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number());
    function_node->AddChild(new Node(
        NodeRule::NAME,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number(),
        CurrentToken()->string_const));

    try {
        AdvanceNext();
        AdvanceNext();

        ParseCallParameters(function_node);

        // Check closing parenthesis.
        if (!CurrentSymbol(LexerSymbol::RPAREN)) {
            THROW_EXCEPTION(
                this->lexer_.current_line_number(),
                this->lexer_.current_column_number(),
                STATUS_PARSER_MALFORMED_FUNCTIONCALL_MISSING_RPAREN);
        }
        AdvanceNext();
    }
    catch (const Exception ex) {
        delete function_node;
        throw;
    }

    return function_node;
}

// Parses a new SpecName() expression.
Node* Parser::ParseNewExpression() {
    GS_ASSERT_TRUE(CurrentKeyword(LexerSymbol::NEW), "Expected NEW in ParseNewExpression");

    Node* new_node = new Node(
        NodeRule::NEW,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number());

    // Check for spec name.
    if (AdvanceNext()->type != LexerTokenType::NAME) {
        delete new_node;
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_PARSER_MALFORMED_NEW_EXPRESSION_MISSING_NAME);
    }

    ParseTypeExpression(new_node);

    // Check for open parenthesis.
    if (!CurrentSymbol(LexerSymbol::LPAREN)) {
        delete new_node;
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_PARSER_MALFORMED_NEW_EXPRESSION_MISSING_LPAREN);
    }

    AdvanceNext();

    // Parse params to the new expression.
    ParseCallParameters(new_node);

    // Check for closing parenthesis.
    if (!CurrentSymbol(LexerSymbol::RPAREN)) {
        delete new_node;
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_PARSER_MALFORMED_NEW_EXPRESSION_MISSING_RPAREN);
    }

    AdvanceNext();

    return new_node;
}

// Parses a default(symbol) expression.
Node* Parser::ParseDefaultExpression() {
    GS_ASSERT_TRUE(CurrentKeyword(LexerSymbol::DEFAULT), "Expected DEFAULT in ParseDefaultExpression");

    Node* default_node = new Node(
        NodeRule::DEFAULT,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number());

    if (!AdvanceSymbol(LexerSymbol::LPAREN)) {
        delete default_node;
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_PARSER_MALFORMED_DEFAULT_EXPRESSION_MISSING_LPAREN);
    }

    AdvanceNext();

    // Parse params to the default expression.
    ParseTypeExpression(default_node);

    // Check for closing parenthesis.
   if (!CurrentSymbol(LexerSymbol::RPAREN)) {
       delete default_node;
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_PARSER_MALFORMED_DEFAULT_EXPRESSION_MISSING_LPAREN);
    }

    AdvanceNext();

    return default_node;
}


void Parser::ParseCallParameters(Node* node) {

    // Allow NEW objects as well as CALL object.
    GS_ASSERT_TRUE(node != NULL, "NULL node in ParseCallParameters");

    Node* parameters_node = new Node(
        NodeRule::CALL_PARAMETERS,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number());
    node->AddChild(parameters_node);

    // Keep on parsing till we reach the close parenthesis.
    if (!CurrentSymbol(LexerSymbol::RPAREN)) {
        ParseExpression(parameters_node);

        while (!CurrentSymbol(LexerSymbol::RPAREN)) {

            // Check for comma.
            if (!CurrentSymbol(LexerSymbol::COMMA)) {
                THROW_EXCEPTION(
                    this->lexer_.current_line_number(),
                    this->lexer_.current_column_number(),
                    STATUS_PARSER_MALFORMED_FUNCTIONCALL_MISSING_COMMA);
            }

            AdvanceNext();
            ParseExpression(parameters_node);
        }
    }
}

Node* Parser::ParseVariableExpression() {

    // Check for variable name type.
    GS_ASSERT_TRUE(CurrentToken()->type == LexerTokenType::NAME,
        "Expected NAME in ParseVariableExpression");

    Node* variable_node = new Node(
        NodeRule::SYMBOL,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number());
    variable_node->AddChild(new Node(
        NodeRule::NAME,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number(),
        CurrentToken()->string_const));
    try {
        AdvanceNext();
    }
    catch (const Exception&) {
        delete variable_node;
        throw;
    }

    return variable_node;
}

Node* Parser::ParseBoolConstant() {

    GS_ASSERT_TRUE(CurrentToken()->type == LexerTokenType::KEYWORD,
        "KEYWORD Expected in ParseBoolConstant");

    LexerSymbol true_false_value = CurrentToken()->symbol;
    AdvanceNext();

    switch (true_false_value) {
    case LexerSymbol::KTRUE:
        return new Node(
            NodeRule::BOOL,
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(), 
            true);
    case LexerSymbol::KFALSE:
        return new Node(
            NodeRule::BOOL,
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(), 
            false);
    default:
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_PARSER_MALFORMED_EXPRESSION_INVALID_TOKEN);
    }
}

Node* Parser::ParseIntConstant() {

    GS_ASSERT_TRUE(CurrentToken()->type == LexerTokenType::INT,
        "INT Expected in ParseIntConstant");

    long int_const = CurrentToken()->int_const;

    AdvanceNext();
    return new Node(
        NodeRule::INT,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number(), 
        int_const);
}

Node* Parser::ParseFloatConstant() {

    GS_ASSERT_TRUE(CurrentToken()->type == LexerTokenType::FLOAT,
        "FLOAT Expected in ParseFloatConstant");

    double float_const = CurrentToken()->float_const;

    AdvanceNext();

    return new Node(
        NodeRule::FLOAT, 
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number(),
        float_const);
}

Node* Parser::ParseCharConstant() {

    GS_ASSERT_TRUE(CurrentToken()->type == LexerTokenType::CHAR,
        "CHAR Expected in ParseCharConstant");

    char char_const = CurrentToken()->char_const;

    AdvanceNext();

    return new Node(
        NodeRule::CHAR, 
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number(),
        (long)char_const);
}

Node* Parser::ParseStringConstant() {

    GS_ASSERT_TRUE(CurrentToken()->type == LexerTokenType::STRING,
        "STRING Expected in ParseStringConstant");

    Node* string_node = new Node(
        NodeRule::STRING,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number(),
        CurrentToken()->string_const);

    try {
        AdvanceNext();
    }
    catch (const Exception&) {
        delete string_node;
        throw;
    }

    return string_node;
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

bool Parser::NextSymbol(LexerSymbol symbol) {
    return NextToken()->type == LexerTokenType::SYMBOL &&
        NextToken()->symbol == symbol;
}

bool Parser::AdvanceType(LexerSymbol type) {
    return AdvanceNext()->type == LexerTokenType::NAME &&
        CurrentToken()->symbol == type;
}

bool Parser::CurrentType(LexerSymbol type) {
    return CurrentToken()->type == LexerTokenType::NAME &&
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

bool Parser::NextKeyword(LexerSymbol symbol) {
    return NextToken()->type == LexerTokenType::KEYWORD &&
        NextToken()->symbol == symbol;
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
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_PARSER_EOF);
    }
}

} // namespace compiler
} // namespace gunderscript
