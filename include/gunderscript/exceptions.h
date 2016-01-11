// Gunderscript-2 Exceptions Definitions
// (C) 2014-2016 Christian Gunderman

#ifndef GUNDERSCRIPT_EXCEPTIONS__H__
#define GUNDERSCRIPT_EXCEPTIONS__H__

#include <exception>
#include <string>

namespace gunderscript {

class ExceptionStatus {
public:
    ExceptionStatus(int code, const std::string& what) : code_(code), what_(what) { }

    int code() const { return code_; }
    const std::string& what() const { return what_; }

private:
    const int code_;
    const std::string what_;
};

class Exception2 : public std::exception {
public:
    Exception2(
        int impl_line,
        const char* impl_file,
        int line,
        int column,
        const ExceptionStatus& status) throw()
        : impl_line_(impl_line), impl_file_(impl_file),
        line_(line), column_(column), status_(status) { }

    int impl_line() const { return impl_line_; }
    const char * impl_file() const { return impl_file_; }
    int line() const { return line_; }
    int column() const { return column_; }
    ExceptionStatus status() const { return status_; }
    virtual const char* what() const throw() { return status_.what().c_str(); }

private:
    const int impl_line_;
    const char * impl_file_;
    const int line_;
    const int column_;
    const ExceptionStatus status_;
};

#define THROW_EXCEPTION(line, column, status)     throw Exception2(__LINE__, __FILE__, line, column, status)

// Status definitions indicating the compile errors.
// Officially status codes are identified by GS[code] but we leave this out so we can use INT comparisons.
const ExceptionStatus STATUS_SUCCESS = ExceptionStatus(0, "Success");
const ExceptionStatus STATUS_ILLEGAL_STATE = ExceptionStatus(-1, "Feature not implemented or bug detected in Gunderscript");

const ExceptionStatus STATUS_LEXER_UNTERMINATED_COMMENT = ExceptionStatus(100, "Unterminated comment");
const ExceptionStatus STATUS_LEXER_INVALID_ESCAPE = ExceptionStatus(101, "Invalid escape sequence");
const ExceptionStatus STATUS_LEXER_UNTERMINATED_STRING = ExceptionStatus(102, "Unterminated string");
const ExceptionStatus STATUS_LEXER_MALFORMED_NUMBER = ExceptionStatus(103, "Malformed numeric constant");
const ExceptionStatus STATUS_LEXER_MALFORMED_CHAR = ExceptionStatus(104, "Malformed character constant");
const ExceptionStatus STATUS_LEXER_NO_MATCH = ExceptionStatus(105, "No matching lexer rule");
const ExceptionStatus STATUS_LEXER_NEWLINE_IN_STRING = ExceptionStatus(106, "New line character in string constant");

const ExceptionStatus STATUS_PARSER_MISSING_PACKAGE= ExceptionStatus(200, "Expected a package declaration at top of file");
const ExceptionStatus STATUS_PARSER_INVALID_PACKAGE = ExceptionStatus(201, "Invalid package name at top of file");
const ExceptionStatus STATUS_PARSER_MALFORMED_DEPENDS = ExceptionStatus(202, "Malformed depends statement");
const ExceptionStatus STATUS_PARSER_EXPECTED_SEMICOLON = ExceptionStatus(203, "Expected but did not find a semicolon");
const ExceptionStatus STATUS_PARSER_MALFORMED_SPEC_ACCESS_MODIFIER_MISSING 
    = ExceptionStatus(204, "Trying to parse Spec definition, expected but did not find an access modifier");
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
    = ExceptionStatus(225, "Malformed block or function or property body, expected a left brace");
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

// Gunderscript Exceptions Parent Class
// Each module's exceptions descend from this class.
class Exception : public std::exception {
public:
    Exception() throw() { };
    Exception(const std::string& message) throw() { message_ = message; };
    virtual const char* what() const throw() { return message_.c_str(); }

private:
    std::string message_;
    std::exception exception_;
};

// Not implemented exception.
class NotImplementedException : public Exception {
public:
    NotImplementedException() : Exception("Feature not implemented.") { }
    NotImplementedException(const std::string& message) : Exception(message) { }
};

// Illegal State exception: thrown if bug detected in Gunderscript.
class IllegalStateException : public Exception {
public:
    IllegalStateException() : Exception("Illegal State: bug detected in Gunderscript.") { }
    IllegalStateException(const std::string& message) : Exception(message) { }
};

} // namespace gunderscript

#endif // GUNDERSCRIPT_EXCEPTIONS__H__
