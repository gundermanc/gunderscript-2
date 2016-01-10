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
const ExceptionStatus STATUS_SUCCESS = ExceptionStatus(0, "Success");
const ExceptionStatus STATUS_ILLEGAL_STATE = ExceptionStatus(-1, "Feature not implemented or bug detected");

const ExceptionStatus STATUS_LEXER_UNTERMINATED_COMMENT = ExceptionStatus(100, "Unterminated comment");
const ExceptionStatus STATUS_LEXER_INVALID_ESCAPE = ExceptionStatus(101, "Invalid escape sequence");
const ExceptionStatus STATUS_LEXER_UNTERMINATED_STRING = ExceptionStatus(102, "Unterminated string");
const ExceptionStatus STATUS_LEXER_MALFORMED_NUMBER = ExceptionStatus(103, "Malformed numeric constant");
const ExceptionStatus STATUS_LEXER_MALFORMED_CHAR = ExceptionStatus(104, "Malformed character constant");
const ExceptionStatus STATUS_LEXER_NO_MATCH = ExceptionStatus(105, "No matching lexer rule");
const ExceptionStatus STATUS_LEXER_NEWLINE_IN_STRING = ExceptionStatus(106, "New line character in string constant");

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
