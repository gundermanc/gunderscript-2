// Gunderscript-2 Compiler Source Interface API
// (C) 2016 Christian Gunderman

#ifndef GUNDERSCRIPT_COMPILER_SOURCE__H__
#define GUNDERSCRIPT_COMPILER_SOURCE__H__

#include <string>

namespace gunderscript {

// Provides source of input to Compiler.
// has_next: returns false if no more input.
// NextChar: returns -1 if no more input.
// PeekNextChar: returns -1 if no more inpu.
class CompilerSourceInterface {
public:
    virtual ~CompilerSourceInterface() { };
    virtual bool has_next() = 0;
    virtual int NextChar() = 0;
    virtual int PeekNextChar() = 0;
};

// Implements CompilerSourceInterface and provides front
// end for strings.
class CompilerStringSource : public CompilerSourceInterface {
public:
    CompilerStringSource(std::string& input) { this->input_ = input; }
    bool has_next() { return this->index < this->input_.length(); }
    int NextChar();
    int PeekNextChar();
private:
    std::string input_;
    size_t index = 0;
};

// Implements CompilerSourceInterface and provides front
// end for files.
class CompilerFileSource : public CompilerSourceInterface {
public:
    CompilerFileSource(const std::string& file_name);
    ~CompilerFileSource();
    bool has_next();
    int NextChar();
    int PeekNextChar();
private:
    const std::string& file_name_;
    std::fstream* file_stream_ = NULL;
    int next_ = 0;
};

} // namespace gunderscript

#endif // GUNDERSCRIPT_COMPILER_SOURCE__H__