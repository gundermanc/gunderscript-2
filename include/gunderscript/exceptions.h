// Gunderscript-2 Exceptions Definitions
// (C) 2014-2016 Christian Gunderman

#ifndef GUNDERSCRIPT_EXCEPTIONS__H__
#define GUNDERSCRIPT_EXCEPTIONS__H__

#include <exception>
#include <string>

namespace gunderscript {

// Describes a Gunderscript status that indicates a compilation or execution error.
class ExceptionStatus {
public:
    ExceptionStatus(int code, const std::string& what) : code_(code), what_(what) { }

    int code() const { return code_; }
    const std::string& what() const { return what_; }
    bool operator==(const ExceptionStatus& other) { return other.code() == this->code(); }
    bool operator!=(const ExceptionStatus& other) { return other.code() != this->code(); }

private:
    const int code_;
    const std::string what_;
};

// The class for all Gunderscript engine exceptions. Use THROW_EXCEPTION instead
// of concrete constructor calls. In DEBUG configuration exception defines a file
// and line number field that show the implementation file where an exception
// originated.
class Exception : public std::exception {
public:
    Exception(
#ifdef _DEBUG
        int impl_line,
        const char* impl_file,
#endif
        int line,
        int column,
        const ExceptionStatus& status) throw() :
#ifdef _DEBUG
        impl_line_(impl_line), impl_file_(impl_file),
#endif
        line_(line), column_(column), status_(status) { }

#ifdef _DEBUG
    int impl_line() const { return impl_line_; }
    const char * impl_file() const { return impl_file_; }
#endif
    int line() const { return line_; }
    int column() const { return column_; }
    ExceptionStatus status() const { return status_; }
    virtual const char* what() const throw() { return status_.what().c_str(); }

private:
#ifdef _DEBUG
    const int impl_line_;
    const char * impl_file_;
#endif
    const int line_;
    const int column_;
    const ExceptionStatus status_;
};

// Used to throw all exceptions within Gunderscript engine so that implementation
// file and line are saved when in DEBUG configuration.
#ifdef _DEBUG
#define THROW_EXCEPTION(line, column, status)     throw Exception(__LINE__, __FILE__, line, column, status)
#else
#define THROW_EXCEPTION(line, column, status)     throw Exception(line, column, status)
#endif

#define THROW_NOT_IMPLEMENTED()                   THROW_EXCEPTION(1, 1, STATUS_ILLEGAL_STATE) 

// Status definitions indicating the compile errors.
// Officially status codes are identified by GS[code] but we leave this out so we can use INT comparisons.
// None of the first group should ever be thrown in normal operation but they're here for completeness.
const ExceptionStatus STATUS_SUCCESS = ExceptionStatus(0, "Success");
const ExceptionStatus STATUS_ILLEGAL_STATE = ExceptionStatus(-1, "Feature not implemented or bug detected in Gunderscript");
const ExceptionStatus STATUS_SYMBOLTABLE_DUPLICATE_SYMBOL = ExceptionStatus(-2, "Symbol table symbol is already defined");
const ExceptionStatus STATUS_SYMBOLTABLE_UNDEFINED_SYMBOL = ExceptionStatus(-3, "No symbol in symbol table with given key");
const ExceptionStatus STATUS_SYMBOLTABLE_BOTTOM_OF_STACK = ExceptionStatus(-4, "Reached bottom of stack while popping Symbol table scope");
const ExceptionStatus STATUS_FILESOURCE_FILE_READ_ERROR = ExceptionStatus(-5, "Unable to read file");
const ExceptionStatus STATUS_INVALID_CALL = ExceptionStatus(-6, "Caller performed invalid call on Gunderscript library");
const ExceptionStatus STATUS_ASSEMBLER_DIED = ExceptionStatus(-7, "Assembler was unable to assemble code");

// Lexer Exceptions 100-199:
const ExceptionStatus STATUS_LEXER_UNTERMINATED_COMMENT = ExceptionStatus(100, "Unterminated comment");
const ExceptionStatus STATUS_LEXER_INVALID_ESCAPE = ExceptionStatus(101, "Invalid escape sequence");
const ExceptionStatus STATUS_LEXER_UNTERMINATED_STRING = ExceptionStatus(102, "Unterminated string");
const ExceptionStatus STATUS_LEXER_MALFORMED_NUMBER = ExceptionStatus(103, "Malformed numeric constant");
const ExceptionStatus STATUS_LEXER_MALFORMED_CHAR = ExceptionStatus(104, "Malformed character constant");
const ExceptionStatus STATUS_LEXER_NO_MATCH = ExceptionStatus(105, "No matching lexer rule");
const ExceptionStatus STATUS_LEXER_NEWLINE_IN_STRING = ExceptionStatus(106, "New line character in string constant");

// Parser Exceptions 200-299:
const ExceptionStatus STATUS_PARSER_MISSING_PACKAGE= ExceptionStatus(200, "Expected a package declaration at top of file");
const ExceptionStatus STATUS_PARSER_INVALID_PACKAGE = ExceptionStatus(201, "Invalid package name at top of file");
const ExceptionStatus STATUS_PARSER_MALFORMED_DEPENDS = ExceptionStatus(202, "Malformed depends statement");
const ExceptionStatus STATUS_PARSER_EXPECTED_SEMICOLON = ExceptionStatus(203, "Expected but did not find a semicolon");
const ExceptionStatus STATUS_PARSER_MALFORMED_SPEC_OR_FUNC_ACCESS_MODIFIER_MISSING 
    = ExceptionStatus(204, "Trying to parse Spec or static Function definition, expected but did not find an access modifier");
const ExceptionStatus STATUS_PARSER_MALFORMED_SPEC_SPEC_KEYWORD_MISSING
    = ExceptionStatus(205, "Trying to parse Spec definition, expected but did not find spec keyword");
const ExceptionStatus STATUS_PARSER_MALFORMED_SPEC_NAME_MISSING
    = ExceptionStatus(206, "Trying to parse Spec definition, expected but did not find spec name");
const ExceptionStatus STATUS_PARSER_MALFORMED_SPEC_LBRACE_MISSING
    = ExceptionStatus(207, "Trying to parse Spec definition, expected but did not find left brace");
const ExceptionStatus STATUS_PARSER_MALFORMED_SPEC_RBRACE_MISSING
    = ExceptionStatus(208, "Trying to parse Spec definition, expected but did not find right brace");
const ExceptionStatus STATUS_PARSER_MALFORMED_PROPERTY_TYPE_MISSING
    = ExceptionStatus(209, "Trying to parse Property definition, expected but did not find type");
const ExceptionStatus STATUS_PARSER_MALFORMED_PROPERTY_NAME_MISSING
    = ExceptionStatus(210, "Trying to parse Property definition, expected but did not find name");
const ExceptionStatus STATUS_PARSER_MALFORMED_PROPERTY_LBRACE_MISSING
    = ExceptionStatus(211, "Trying to parse Property definition, expected but did not find left brace");
const ExceptionStatus STATUS_PARSER_MALFORMED_PROPERTY_RBRACE_MISSING
    = ExceptionStatus(212, "Trying to parse Property definition, expected but did not find left brace");
const ExceptionStatus STATUS_PARSER_MALFORMED_PROPERTY_MISSING_PROPERTY_FUNCTION
    = ExceptionStatus(213, "Property definition must define exactly one get and one set accessor/mutator");
const ExceptionStatus STATUS_PARSER_MALFORMED_PROPERTYFUNCTION_MISSING_ACCESS_MODIFIER
    = ExceptionStatus(214, "Property accessor/mutator must start with access modifier");
const ExceptionStatus STATUS_PARSER_MALFORMED_PROPERTYFUNCTION_INVALID_ACCESSORMUTATOR
    = ExceptionStatus(215, "Malformed property, body can only contain get or set accessor or mutator");
const ExceptionStatus STATUS_PARSER_MALFORMED_PROPERTYFUNCTION_DUPLICATE
    = ExceptionStatus(216, "Duplicate get or set accessor or mutator");
const ExceptionStatus STATUS_PARSER_MALFORMED_FUNCTION_MISSING_ACCESS_MODIFIER
    = ExceptionStatus(217, "Trying to parse Function definition, expected but did not find access modifier");
const ExceptionStatus STATUS_PARSER_MALFORMED_FUNCTION_MISSING_TYPE
    = ExceptionStatus(218, "Trying to parse Function definition, expected but did not find type");
const ExceptionStatus STATUS_PARSER_MALFORMED_FUNCTION_MISSING_NAME
    = ExceptionStatus(219, "Trying to parse Function definition, expected but did not find name");
const ExceptionStatus STATUS_PARSER_MALFORMED_FUNCTION_MISSING_LPAREN
    = ExceptionStatus(220, "Trying to parse Function definition, expected but did not find left parenthesis for parameters");
const ExceptionStatus STATUS_PARSER_MALFORMED_FUNCTION_MISSING_RPAREN
    = ExceptionStatus(221, "Trying to parse Function definition, expected but did not find right parenthesis for parameters");
const ExceptionStatus STATUS_PARSER_MALFORMED_FUNCTIONPARAMS_MISSING_COMMA
    = ExceptionStatus(222, "Trying to parse Function parameter definitions, expected but did not find comma delimiter or right parenthesis");
const ExceptionStatus STATUS_PARSER_MALFORMED_FUNCTIONPARAMS_MISSING_TYPE
    = ExceptionStatus(223, "Trying to parse Function parameter definitions, expected but did not find type");
const ExceptionStatus STATUS_PARSER_MALFORMED_FUNCTIONPARAMS_MISSING_NAME
    = ExceptionStatus(224, "Trying to parse Function parameter definitions, expected but did not find name");
const ExceptionStatus STATUS_PARSER_MALFORMED_BLOCK_MISSING_LBRACE
    = ExceptionStatus(225, "Malformed block or body, expected a left brace");
const ExceptionStatus STATUS_PARSER_MALFORMED_BLOCK_MISSING_RBRACE
    = ExceptionStatus(226, "Malformed block or function or property body, expected a right brace");
const ExceptionStatus STATUS_PARSER_EXPECTED_STATEMENT
    = ExceptionStatus(227, "Expected a statement");
const ExceptionStatus STATUS_PARSER_EXPECTED_STATEMENT_INVALID_KEYWORD
    = ExceptionStatus(228, "Invalid keyword, expected a statement");
const ExceptionStatus STATUS_PARSER_MALFORMED_SPEC_UNKNOWN_MEMBER
    = ExceptionStatus(229, "Invalid or malformed Spec member, functions start with access modifiers and properties with a type");
const ExceptionStatus STATUS_PARSER_MALFORMED_EXPRESSION_MISSING_RPAREN
    = ExceptionStatus(230, "Invalid expression, missing right parenthesis");
const ExceptionStatus STATUS_PARSER_MALFORMED_EXPRESSION_INVALID_TOKEN
    = ExceptionStatus(231, "Invalid expression, unexpected token");
const ExceptionStatus STATUS_PARSER_MALFORMED_FUNCTIONCALL_MISSING_RPAREN
    = ExceptionStatus(232, "Trying to parse function call parameters, expected but did not find right parenthesis");
const ExceptionStatus STATUS_PARSER_MALFORMED_FUNCTIONCALL_MISSING_COMMA
    = ExceptionStatus(233, "Missing comma delimiter in function call parameters");
const ExceptionStatus STATUS_PARSER_EOF
    = ExceptionStatus(234, "Reached end of file while parsing");
const ExceptionStatus STATUS_PARSER_INCOMPLETE_NAME_STATEMENT
    = ExceptionStatus(235, "Incomplete function call or assignment statement");
const ExceptionStatus STATUS_PARSER_MALFORMED_IF_MISSING_LPAREN
    = ExceptionStatus(236, "Trying to parse if statement, expected but did not find left parenthesis for condition");
const ExceptionStatus STATUS_PARSER_MALFORMED_IF_MISSING_RPAREN
    = ExceptionStatus(237, "Trying to parse if statement, expected but did not find right parenthesis after condition");
const ExceptionStatus STATUS_PARSER_MALFORMED_FOR_MISSING_LPAREN
    = ExceptionStatus(238, "Trying to parse for loop, expected but did not find left parenthesis");
const ExceptionStatus STATUS_PARSER_MALFORMED_FOR_MISSING_RPAREN
    = ExceptionStatus(239, "Trying to parse for loop, expected but did not find right parenthesis");
const ExceptionStatus STATUS_PARSER_MALFORMED_WHILE_MISSING_LPAREN
    = ExceptionStatus(240, "Trying to parse while loop, expected but did not find left parenthesis");
const ExceptionStatus STATUS_PARSER_MALFORMED_WHILE_MISSING_RPAREN
    = ExceptionStatus(241, "Trying to parse while loop, expected but did not find right parenthesis");
const ExceptionStatus STATUS_PARSER_MALFORMED_TYPE_PARAM_MISSING_NAME
    = ExceptionStatus(242, "Trying to parse type expression parameter, expected  but did not find type");
const ExceptionStatus STATUS_PARSER_MALFORMED_TYPE_PARAM_MISSING_COMMA
    = ExceptionStatus(243, "Trying to parse type expression, expected but did not find comma delimiter or right angle brace");
const ExceptionStatus STATUS_PARSER_MALFORMED_TYPE_PARAM_MISSING_GREATER
    = ExceptionStatus(244, "Trying to parse type expression, expected but did not find closing angle brace");
const ExceptionStatus STATUS_PARSER_MALFORMED_NEW_EXPRESSION_MISSING_NAME
    = ExceptionStatus(245, "Trying to parse new spec expression, expected but did not find spec name");
const ExceptionStatus STATUS_PARSER_MALFORMED_NEW_EXPRESSION_MISSING_LPAREN
    = ExceptionStatus(246, "Trying to parse new spec expression, expected but did not find left parenthesis");
const ExceptionStatus STATUS_PARSER_MALFORMED_NEW_EXPRESSION_MISSING_RPAREN
    = ExceptionStatus(247, "Trying to parse new spec expression, expected but did not find right parenthesis");
const ExceptionStatus STATUS_PARSER_MALFORMED_DEFAULT_EXPRESSION_MISSING_LPAREN
    = ExceptionStatus(248, "Trying to parse default expression, expected but did not find left parenthesis");
const ExceptionStatus STATUS_PARSER_MALFORMED_DEFAULT_EXPRESSION_MISSING_TYPE
    = ExceptionStatus(249, "Trying to parse default expression, expected but did not find type");
const ExceptionStatus STATUS_PARSER_MALFORMED_DEFAULT_EXPRESSION_MISSING_RPAREN
    = ExceptionStatus(250, "Trying to parse default expression, expected but did not find right parenthesis");
const ExceptionStatus STATUS_PARSER_CONSTRUCTOR_OUTSIDE_SPEC
    = ExceptionStatus(251, "Constructor function is not allowed outside of spec body");
const ExceptionStatus STATUS_PARSER_MALFORMED_NAME
    = ExceptionStatus(252, "Parsing name expression, expected but did not find name");

// Semantic Walker Exceptions 300-399:
const ExceptionStatus STATUS_SEMANTIC_TYPE_MISMATCH_IN_ASSIGN
    = ExceptionStatus(300, "Invalid type in assignment, expression evaluates to different type than variable");
const ExceptionStatus STATUS_SEMANTIC_RETURN_FROM_PROPERTY_SET
    = ExceptionStatus(301, "Set Property mutator does not return a value but has a return statement");
const ExceptionStatus STATUS_SEMANTIC_RETURN_TYPE_MISMATCH
    = ExceptionStatus(302, "Return statement expression type does not match function or property type");
const ExceptionStatus STATUS_SEMANTIC_INVALID_PACKAGE
    = ExceptionStatus(303, "Invalid package name");
const ExceptionStatus STATUS_SEMANTIC_NONBOOL_IN_LOGNOT
    = ExceptionStatus(304, "Non-boolean type in '!' expression");
const ExceptionStatus STATUS_SEMANTIC_NOT_ACCESSIBLE
    = ExceptionStatus(305, "The referenced spec member has an access modifier that makes it not visible to this code");
const ExceptionStatus STATUS_SEMANTIC_UNMATCHING_TYPE_IN_ADD
    = ExceptionStatus(306, "Non-matching types in '+' expression");
const ExceptionStatus STATUS_SEMANTIC_UNMATCHING_TYPE_IN_SUB
    = ExceptionStatus(307, "Non-matching types in '-' expression");
const ExceptionStatus STATUS_SEMANTIC_UNMATCHING_TYPE_IN_MUL
    = ExceptionStatus(308, "Non-matching types in '*' expression");
const ExceptionStatus STATUS_SEMANTIC_UNMATCHING_TYPE_IN_DIV
    = ExceptionStatus(309 , "Non-matching types in '/' expression");
const ExceptionStatus STATUS_SEMANTIC_UNMATCHING_TYPE_IN_MOD
    = ExceptionStatus(310, "Non-matching types in '%' expression");
const ExceptionStatus STATUS_SEMANTIC_UNMATCHING_TYPE_IN_LOGOR
    = ExceptionStatus(311, "Non-matching types in '||' expression");
const ExceptionStatus STATUS_SEMANTIC_UNMATCHING_TYPE_IN_LOGAND
    = ExceptionStatus(312, "Non-matching types in '&&' expression");
const ExceptionStatus STATUS_SEMANTIC_UNMATCHING_TYPE_IN_GREATER
    = ExceptionStatus(313, "Non-matching types in '>' expression");
const ExceptionStatus STATUS_SEMANTIC_UNMATCHING_TYPE_IN_LESS
    = ExceptionStatus(314, "Non-matching types in '<' expression");
const ExceptionStatus STATUS_SEMANTIC_UNMATCHING_TYPE_IN_GREATER_EQUALS
    = ExceptionStatus(315, "Non-matching types in '>=' expression");
const ExceptionStatus STATUS_SEMANTIC_UNMATCHING_TYPE_IN_LESS_EQUALS
    = ExceptionStatus(316, "Non-matching types in '<=' expression");
const ExceptionStatus STATUS_SEMANTIC_NONNUMERIC_OPERANDS
    = ExceptionStatus(317, "Non-numeric operands used with numeric operator");
const ExceptionStatus STATUS_SEMANTIC_NONBOOL_OPERANDS
    = ExceptionStatus(318, "Non-boolean operands used with boolean operator");
const ExceptionStatus STATUS_SEMANTIC_UNMATCHING_TYPE_IN_EQUALS
    = ExceptionStatus(319, "Non-matching types in '=' expression");
const ExceptionStatus STATUS_SEMANTIC_UNMATCHING_TYPE_IN_NOT_EQUALS
    = ExceptionStatus(320, "Non-matching types in '!=' expression");
const ExceptionStatus STATUS_SEMANTIC_DUPLICATE_FUNCTION
    = ExceptionStatus(321, "A function with the same name and parameters has already been declared in this Spec");
const ExceptionStatus STATUS_SEMANTIC_DUPLICATE_PROPERTY
    = ExceptionStatus(322, "A property with the same name has already been declared in this Spec");
const ExceptionStatus STATUS_SEMANTIC_DUPLICATE_FUNCTION_PARAM
    = ExceptionStatus(323, "A function param with the same name has already been declared in this function");
const ExceptionStatus STATUS_SEMANTIC_FUNCTION_OVERLOAD_NOT_FOUND
    = ExceptionStatus(324, "Cannot find a function overload or typecast with that name that accepts the specified param types");
const ExceptionStatus STATUS_SEMANTIC_INVALID_TYPE_IN_ADD
    = ExceptionStatus(325, "'+' operator supports only string and numeric types");
const ExceptionStatus STATUS_SEMANTIC_DUPLICATE_SPEC
    = ExceptionStatus(326, "A Spec with that name already exists");
const ExceptionStatus STATUS_SEMANTIC_UNDEFINED_TYPE
    = ExceptionStatus(327, "Undefined type or invalid generic parameters");
const ExceptionStatus STATUS_SEMANTIC_UNSUPPORTED_TYPECAST
    = ExceptionStatus(328, "This typecast is not supported");
const ExceptionStatus STATUS_SEMANTIC_INVALID_IF_CONDITION_TYPE
    = ExceptionStatus(329, "Invalid if statement condition type");
const ExceptionStatus STATUS_SEMANTIC_INVALID_LOOP_CONDITION_TYPE
    = ExceptionStatus(330, "Invalid loop condition type");
const ExceptionStatus STATUS_SEMANTIC_UNDEFINED_VARIABLE
    = ExceptionStatus(331, "Variable is not defined in current scope");
const ExceptionStatus STATUS_SEMANTIC_GENERIC_DUPLICATE_PARAM
    = ExceptionStatus(332, "A symbol with the same name as a generic param already exists");
const ExceptionStatus STATUS_SEMANTIC_RETURN_IN_VOID
    = ExceptionStatus(333, "Return value in void function or constructor");
const ExceptionStatus STATUS_SEMANTIC_VOID_USED_IN_EXPR
    = ExceptionStatus(334, "Void function call has no value and cannot be used in expression");
const ExceptionStatus STATUS_SEMANTIC_CONSTRUCTOR_OVERLOAD_NOT_FOUND
    = ExceptionStatus(335, "Cannot find a constructor overload for this spec that accepts the specified param types");
const ExceptionStatus STATUS_SEMANTIC_VOID_USED_IN_PARAM
    = ExceptionStatus(336, "Void type cannot be used as a function param");
const ExceptionStatus STATUS_SEMANTIC_THIS_ASSIGNED
    = ExceptionStatus(337, "'this' keyword cannot be manually assigned to");

} // namespace gunderscript

#endif // GUNDERSCRIPT_EXCEPTIONS__H__
