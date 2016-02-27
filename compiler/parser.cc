// Gunderscript-2 Parser
// (C) 2014-2016 Christian Gunderman

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

    Node* depends_node = new Node(
        NodeRule::DEPENDS,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number());
    Node* specs_node = new Node(
        NodeRule::SPECS,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number());
    Node* functions_node = new Node(
        NodeRule::FUNCTIONS,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number());
    module_node_->AddChild(depends_node);
    module_node_->AddChild(specs_node);
    module_node_->AddChild(functions_node);

    // Allow end of file after package.
    if (has_next()) {
        AdvanceNext();
        ParseDependsStatements(depends_node);
    }

    // Allow end of file after depends.
    if (has_next()) {
        ParseModuleBody(specs_node, functions_node);
    }

    return module_node_;
}

// Parses a package declaration from a file. Package declaration syntax is:
// package "MyPackage";
// Throws: Lexer or Parser exceptions from the respective headers if a problem
// is encountered with lexemes or syntax.
void Parser::ParsePackageDeclaration(Node* node) {

    // Check "package" keyword.
    if (!AdvanceKeyword(LexerSymbol::PACKAGE)) {
        THROW_EXCEPTION(
            1,
            1,
            STATUS_PARSER_MISSING_PACKAGE);
    }

    // Check package name string.
    if (AdvanceNext()->type != LexerTokenType::STRING) {
        THROW_EXCEPTION(
            1,
            1,
            STATUS_PARSER_INVALID_PACKAGE);
    }

    // Save package name string in first module child.
    node->AddChild(new Node(
        NodeRule::NAME,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number(),
        CurrentToken()->string_const));

    AdvanceNext();
    ParseSemicolon(node);
}

// Parses all depends statements from a file. Depends statements indicate that this
// file requires code in another file to run. Depends comes right after package declarations
// and may be from 0 to many.
// Throws: Lexer or Parser exceptions from the respective headers if a problem
// is encountered with lexemes or syntax.
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

// Parses single depends statements from a file. Depends statements indicate that this
// file requires code in another file to run. Looks like:
// depends "OtherScript";
// Throws: Lexer or Parser exceptions from the respective headers if a problem
// is encountered with lexemes or syntax.
void Parser::ParseDependsStatement(Node* node) {

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
    ParseSemicolon(node);
}

// Checks that current lexeme is a semicolon, such as at the end of a line.
// Throws: Parser exceptions if current lexeme isn't a semicolon.
void Parser::ParseSemicolon(Node* node) {

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
void Parser::ParseModuleBody(Node* specs_node, Node* functions_node) {
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
            ParseFunction(functions_node);
        }

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
void Parser::ParseSpecDefinition(Node* node) {
    Node* spec_node = new Node(
        NodeRule::SPEC,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number());
    node->AddChild(spec_node);

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

    spec_node->AddChild(new Node(
        NodeRule::NAME,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number(),
        CurrentToken()->string_const));

    // Check for opening curly brace.
    if (!AdvanceSymbol(LexerSymbol::LBRACE)) {
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
void Parser::ParseSpecBody(Node* node) {
    Node* properties_node = new Node(
        NodeRule::PROPERTIES,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number());
    Node* functions_node = new Node(
        NodeRule::FUNCTIONS,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number());

    node->AddChild(functions_node);
    node->AddChild(properties_node);

    // Parse body elements until we reach the closing curly brace.
    while (!AdvanceSymbol(LexerSymbol::RBRACE)) {
        switch (CurrentToken()->type) {
        case LexerTokenType::ACCESS_MODIFIER:
            ParseFunction(functions_node);
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
void Parser::ParseProperty(Node* node) {
    Node* property_node = new Node(
        NodeRule::PROPERTY,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number());
    node->AddChild(property_node);

    // Make sure first token is a TYPE.
    if (CurrentToken()->type != LexerTokenType::NAME) {
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_PARSER_MALFORMED_PROPERTY_TYPE_MISSING);
    }

    property_node->AddChild(new Node(
        NodeRule::TYPE,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number(),
        CurrentToken()->string_const));

    // Make sure second token is a NAME.
    if (AdvanceNext()->type != LexerTokenType::NAME) {
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
void Parser::ParsePropertyBody(Node* node) {
    Node* getter_node = new Node(
        NodeRule::PROPERTY_FUNCTION,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number());
    Node* setter_node = new Node(
        NodeRule::PROPERTY_FUNCTION,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number());

    node->AddChild(getter_node);
    node->AddChild(setter_node);

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
void Parser::ParseFunction(Node* node) {
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

    // Check for NATIVE function token.
    bool native_function = false;
    if (AdvanceKeyword(LexerSymbol::NATIVE)) {
        native_function = true;
        AdvanceNext();
    }

    function_node->AddChild(new Node(
        NodeRule::NATIVE,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number(),
        native_function));

    if (CurrentKeyword(LexerSymbol::CONSTRUCT)) {
        // This branch reads contructors and assigns NULL as their
        // name and type name nodes.

        function_node->AddChild(new Node(
            NodeRule::TYPE,
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            (const std::string*)NULL));

        function_node->AddChild(new Node(
            NodeRule::NAME,
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            (const std::string*)NULL));
    }
    else {
        // This branch checks for function return type and name.

        // Check for TYPE for return type.
        if (CurrentToken()->type != LexerTokenType::NAME) {
            THROW_EXCEPTION(
                this->lexer_.current_line_number(),
                this->lexer_.current_column_number(),
                STATUS_PARSER_MALFORMED_FUNCTION_MISSING_TYPE);
        }

        function_node->AddChild(new Node(
            NodeRule::TYPE,
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            CurrentToken()->string_const));

        // Check for NAME symbol type for function name.
        if (AdvanceNext()->type != LexerTokenType::NAME) {
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

    // If we found a semicolon, function has no body, we're done.
    if (AdvanceSymbol(LexerSymbol::SEMICOLON)) {
        return;
    }

    ParseBlockStatement(function_node);
}

// Parses function arguments.
// Throws: Lexer or Parser exceptions from the respective headers if a problem
// is encountered with lexemes or syntax.
void Parser::ParseFunctionParameters(Node* node) {
    Node* parameters_node = new Node(
        NodeRule::FUNCTION_PARAMETERS,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number());
    node->AddChild(parameters_node);

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
void Parser::ParseFunctionParameter(Node* node) {
    Node* parameter_node = new Node(
        NodeRule::FUNCTION_PARAMETER,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number());
    node->AddChild(parameter_node);

    // Check for a parameter TYPE.
    if (CurrentToken()->type != LexerTokenType::NAME) {
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_PARSER_MALFORMED_FUNCTIONPARAMS_MISSING_TYPE);
    }

    parameter_node->AddChild(new Node(
        NodeRule::TYPE,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number(),
        CurrentToken()->string_const));

    // Check for a parameter NAME.
    if (AdvanceNext()->type != LexerTokenType::NAME) {
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
    switch (CurrentToken()->type) {
    case LexerTokenType::SYMBOL:
        ParseBlockStatement(node);
        AdvanceNext();
        break;
    case LexerTokenType::KEYWORD:
        ParseKeywordStatement(node);
        break;
    case LexerTokenType::NAME:
        ParseNameStatement(node);
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
    switch (CurrentToken()->symbol) {
    case LexerSymbol::IF:
        ParseIfStatement(node);
        break;
    case LexerSymbol::WHILE:
        ParseWhileStatement(node);
        break;
    case LexerSymbol::DO:
        ParseDoWhileStatement(node);
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

void Parser::ParseIfStatement(Node* node) {
    THROW_EXCEPTION(
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number(),
        STATUS_ILLEGAL_STATE);
}

void Parser::ParseWhileStatement(Node* node) {
    THROW_EXCEPTION(
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number(),
        STATUS_ILLEGAL_STATE);
}

void Parser::ParseDoWhileStatement(Node* node) {
    THROW_EXCEPTION(
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number(),
        STATUS_ILLEGAL_STATE);
}

void Parser::ParseForStatement(Node* node) {
    THROW_EXCEPTION(
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number(),
        STATUS_ILLEGAL_STATE);
}

// Parses a return statement.
// Throws: Lexer or Parser exceptions from the respective headers if a problem
// is encountered with lexemes or syntax.
void Parser::ParseReturnStatement(Node* node) {

    // Check for return keyword.
    if (!CurrentKeyword(LexerSymbol::RETURN)) {
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_ILLEGAL_STATE);
    }

    Node* return_node = new Node(
        NodeRule::RETURN,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number());
    node->AddChild(return_node);

    // Check for return expression.
    if (!AdvanceSymbol(LexerSymbol::SEMICOLON)) {
        ParseExpression(return_node);
    }

    ParseSemicolon(node);
    AdvanceNext();
}

// Parses a statement beginning with a name (a variable or function name) into either
// a function call or an assignment.
// Throws: Lexer or Parser exceptions from the respective headers if a problem
// is encountered with lexemes or syntax.
void Parser::ParseNameStatement(Node* node) {

    // Check for name token type.
    if (CurrentToken()->type != LexerTokenType::NAME ||
        NextToken()->type != LexerTokenType::SYMBOL) {
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_ILLEGAL_STATE);
    }

    switch (NextToken()->symbol) {
    case LexerSymbol::LPAREN:
        ParseCallStatement(node);
        break;
    case LexerSymbol::ASSIGN:
        ParseAssignStatement(node);
        break;
    default:
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_ILLEGAL_STATE);
    }
}

// Parses a function call.
// Throws: Lexer or Parser exceptions from the respective headers if a problem
// is encountered with lexemes or syntax.
void Parser::ParseCallStatement(Node* node) {

    // Check for receiving variable and assign operator.
    if (CurrentToken()->type != LexerTokenType::NAME ||
        !NextSymbol(LexerSymbol::LPAREN)) {
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_ILLEGAL_STATE);
    }

    node->AddChild(ParseCallExpression());
    ParseSemicolon(node);
    AdvanceNext();
}

// Parses a variable assignment statement.
// Throws: Lexer or Parser exceptions from the respective headers if a problem
// is encountered with lexemes or syntax.
void Parser::ParseAssignStatement(Node* node) {

    // Check for receiving variable and assign operator.
    if (CurrentToken()->type != LexerTokenType::NAME ||
        !NextSymbol(LexerSymbol::ASSIGN)) {
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_ILLEGAL_STATE);
    }

    node->AddChild(ParseAssignExpressionA());
    ParseSemicolon(node);
    AdvanceNext();
}

// Parses an expression.
// Throws: Lexer or Parser exceptions from the respective headers if a problem
// is encountered with lexemes or syntax.
void Parser::ParseExpression(Node* node) {
    // TODO: Implement:
    // ParseSpecExpression for creating and dereferencing objects.
    Node* expression_node = new Node(
        NodeRule::EXPRESSION,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number());
    node->AddChild(expression_node);

    expression_node->AddChild(ParseAssignExpressionA());
}

Node* Parser::ParseAssignExpressionA() {
    Node* left_operand_node = ParseOrExpressionA();

    return ParseAssignExpressionB(left_operand_node);
}

Node* Parser::ParseAssignExpressionB(Node* left_operand_node) {

    // Return left operand if this isn't an operation.
    if (CurrentToken()->type != LexerTokenType::SYMBOL) {
        return left_operand_node;
    }

    if (CurrentToken()->symbol != LexerSymbol::ASSIGN) {
        return left_operand_node;
    }

    Node* operation_node = new Node(
        NodeRule::ASSIGN,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number());
    Node* parent_node = NULL;
    try {
        AdvanceNext();
        operation_node->AddChild(left_operand_node);
        operation_node->AddChild(ParseOrExpressionA());
        parent_node = ParseAssignExpressionB(operation_node);
    }
    catch (const Exception&) {
        delete operation_node;
        throw;
    }

    return parent_node;
}

Node* Parser::ParseOrExpressionA() {
    Node* left_operand_node = ParseAndExpressionA();

    return ParseOrExpressionB(left_operand_node);
}

Node* Parser::ParseOrExpressionB(Node* left_operand_node) {

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
        parent_node = ParseOrExpressionB(operation_node);
    }
    catch (const Exception&) {
        delete parent_node;
        throw;
    }

    return parent_node;
}

Node* Parser::ParseAndExpressionA() {
    Node* left_operand_node = ParseComparisonExpressionA();

    return ParseAndExpressionB(left_operand_node);
}

Node* Parser::ParseAndExpressionB(Node* left_operand_node) {

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
    Node* left_operand_node = ParsePrimaryExpressionA();

    return ParseComparisonExpressionB(left_operand_node);
}

Node* Parser::ParseComparisonExpressionB(Node* left_operand_node) {

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
    Node* left_operand_node = ParseSecondaryExpressionA();

    return ParsePrimaryExpressionB(left_operand_node);
}

Node* Parser::ParsePrimaryExpressionB(Node* left_operand_node) {

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
    Node* left_operand_node = ParseTertiaryExpressionA();

    return ParseSecondaryExpressionB(left_operand_node);
}

Node* Parser::ParseSecondaryExpressionB(Node* left_operand_node) {

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
    Node* left_operand_node = ParseInvertExpression();

    return ParseTertiaryExpressionB(left_operand_node);
}

Node* Parser::ParseTertiaryExpressionB(Node* left_operand_node) {

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
        return ParseBoolConstant();
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

    // Check for NAME.
    if (CurrentToken()->type != LexerTokenType::NAME) {
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_ILLEGAL_STATE);
    }

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

Node* Parser::ParseMemberNameExpression() {

    // Check for NAME.
    if (CurrentToken()->type != LexerTokenType::NAME ||
        !NextSymbol(LexerSymbol::DOT)) {
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_ILLEGAL_STATE);
    }

    Node* name_node = new Node(
        NodeRule::NAME,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number(),
        CurrentToken()->string_const);

    try {
        AdvanceNext();
    }
    catch (const Exception&) {
        delete name_node;
        throw;
    }

    return name_node;
}

Node* Parser::ParseCallExpression() {

    // Check for function NAME and LPAREN.
    if (CurrentToken()->type != LexerTokenType::NAME ||
        !NextSymbol(LexerSymbol::LPAREN)) {
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_ILLEGAL_STATE);
    }

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

void Parser::ParseCallParameters(Node* node) {
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
    if (CurrentToken()->type != LexerTokenType::NAME) {
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_ILLEGAL_STATE);
    }

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

    if (CurrentToken()->type != LexerTokenType::KEYWORD) {
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_ILLEGAL_STATE);
    }

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
    long int_const = CurrentToken()->int_const;

    // Check for INT type.
    if (CurrentToken()->type != LexerTokenType::INT) {
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_ILLEGAL_STATE);
    }

    AdvanceNext();
    return new Node(
        NodeRule::INT,
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number(), 
        int_const);
}

Node* Parser::ParseFloatConstant() {
    double float_const = CurrentToken()->float_const;

    // Check for FLOAT type.
    if (CurrentToken()->type != LexerTokenType::FLOAT) {
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_ILLEGAL_STATE);
    }

    AdvanceNext();

    return new Node(
        NodeRule::FLOAT, 
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number(),
        float_const);
}

Node* Parser::ParseCharConstant() {
    char char_const = CurrentToken()->char_const;

    // Check for CHAR type.
    if (CurrentToken()->type != LexerTokenType::CHAR) {
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_ILLEGAL_STATE);
    }

    AdvanceNext();

    return new Node(
        NodeRule::CHAR, 
        this->lexer_.current_line_number(),
        this->lexer_.current_column_number(),
        (long)char_const);
}

Node* Parser::ParseStringConstant() {

    // Check for STRING type.
    if (CurrentToken()->type != LexerTokenType::STRING) {
        THROW_EXCEPTION(
            this->lexer_.current_line_number(),
            this->lexer_.current_column_number(),
            STATUS_ILLEGAL_STATE);
    }

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
