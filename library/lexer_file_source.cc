// Gunderscript-2 Lexer File Source
// (C) 2015 Christian Gunderman

#include "lexer_file_source.h"

using std::fstream;
using std::ios_base;

namespace gunderscript {
namespace library {

// Constructor for the file source object.
LexerFileSource::LexerFileSource(const std::string& file_name) : file_name_(file_name) {
    this->file_stream_ = new fstream(file_name, ios_base::in);

    // If unable to read file, throw an exception.
    if (!this->file_stream_->is_open() || !this->file_stream_->good()) {
        throw LexerFileReadException(file_name);
    }

    // Preload the first byte.
    this->NextChar();
}

// Checks to see if the file has any more bytes remaining.
bool LexerFileSource::has_next() {
    return this->next_ != EOF;
}

// Gets the next byte from the file.
// Returns: next byte or -1 if none remain.
// Throws LexerFileReadException if read error occurs.
int LexerFileSource::NextChar() {
    if (!this->has_next()) {
        return -1;
    }

    int current = this->next_;

    // Update stored next char.
    this->next_ = this->file_stream_->get();

    // Throw exception if unable to read.
    if (this->file_stream_->fail() && this->has_next()) {
        throw LexerFileReadException(this->file_name_);
    }

    // Return the old next, a.k.a., current char.
    return current;
}

// Get the char without advancing the current char cursor in the source.
int LexerFileSource::PeekNextChar() {
    if (this->next_ == EOF) {
        return -1;
    }

    return this->next_;
}

// Delete the file source object and closes the file.
LexerFileSource::~LexerFileSource() {
    delete this->file_stream_;
}

} // namespace library
} // namespace gunderscript