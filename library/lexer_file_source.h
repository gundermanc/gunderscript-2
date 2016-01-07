// Gunderscript-2 Lexer File Source
// (C) 2015 Christian Gunderman

#ifndef GUNDERSCRIPT_LEXER_FILE_SOURCE__H__
#define GUNDERSCRIPT_LEXER_FILE_SOURCE__H__

#include <fstream>
#include <iostream>

#include "lexer.h"

namespace gunderscript {
namespace library {

// Implements LexerSourceInterface and provides front
// end for strings.
class LexerFileSource : public LexerSourceInterface {
public:
    LexerFileSource(const std::string& file_name);
    ~LexerFileSource();
    bool has_next();
    int NextChar();
    int PeekNextChar();
private:
    const std::string& file_name_;
    std::fstream* file_stream_ = NULL;
    int next_ = 0;
};

// Exception for file read errors.
class LexerFileReadException : public Exception {
public:
    LexerFileReadException(const std::string& file_name) :
        Exception("Error opening or reading from " + file_name + ".") { }
};

} // namespace library
} // namespace gunderscript

#endif // GUNDERSCRIPT_LEXER_FILE_SOURCE__H__
