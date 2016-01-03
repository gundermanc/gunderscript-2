// Gunderscript-2 Compiler API Implementation
// (C) 2014-2015 Christian Gunderman

#include "gunderscript/compiler.h"

#include "lexer.h"
#include "parser.h"
#include "semantic_ast_walker.h"

using namespace gunderscript::library;

namespace gunderscript {

// Class implementation goes here.
class CompilerImpl {
public:
    CompilerImpl(CompilerSourceInterface& source) : source_(source) { }

private:
    CompilerSourceInterface& source_;
};

Compiler::Compiler(CompilerSourceInterface& source) 
    : pimpl_(new CompilerImpl(source)) {

}

Compiler::~Compiler() {
    delete pimpl_;
}

} // namespace gunderscript
