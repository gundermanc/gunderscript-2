// Gunderscript-2 Lexer File Source
// (C) 2015-2016 Christian Gunderman

#include <fstream>
#include <iostream>

#include "gunderscript/compiler_source.h"

using std::fstream;
using std::ios_base;

namespace gunderscript {

// Gets the current character from an instance of LexerStringSource
// and advances the iterator to the next char.
// Returns: The next char, or -1 if no characters remain.
int CompilerStringSource::NextChar() {
    if (!this->has_next()) {
        return -1;
    }

    return this->input_[this->index++];
}

// Looks at the current character in the series without moving the
// iterator.
// Returns: The current character, or -1 if no characters remain.
int CompilerStringSource::PeekNextChar() {
    if (!this->has_next()) {
        return -1;
    }

    return this->input_[this->index];
}

// Constructor for the file source object.
CompilerFileSource::CompilerFileSource(const std::string& file_name) : file_name_(file_name) {
    this->file_stream_ = new fstream(file_name, ios_base::in);

    // If unable to read file, throw an exception.
    if (!this->file_stream_->is_open() || !this->file_stream_->good()) {
        throw CompilerFileReadException(file_name);
    }

    // Preload the first byte.
    this->NextChar();
}

// Checks to see if the file has any more bytes remaining.
bool CompilerFileSource::has_next() {
    return this->next_ != EOF;
}

// Gets the next byte from the file.
// Returns: next byte or -1 if none remain.
// Throws CompilerFileReadException if read error occurs.
int CompilerFileSource::NextChar() {
    if (!this->has_next()) {
        return -1;
    }

    int current = this->next_;

    // Update stored next char.
    this->next_ = this->file_stream_->get();

    // Throw exception if unable to read.
    if (this->file_stream_->fail() && this->has_next()) {
        throw CompilerFileReadException(this->file_name_);
    }

    // Return the old next, a.k.a., current char.
    return current;
}

// Get the char without advancing the current char cursor in the source.
int CompilerFileSource::PeekNextChar() {
    if (this->next_ == EOF) {
        return -1;
    }

    return this->next_;
}

// Delete the file source object and closes the file.
CompilerFileSource::~CompilerFileSource() {
    delete this->file_stream_;
}

} // namespace gunderscript