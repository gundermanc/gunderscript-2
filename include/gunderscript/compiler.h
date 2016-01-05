// Gunderscript-2 Compiler API
// (C) 2016 Christian Gunderman

#ifndef GUNDERSCRIPT_COMPILER__H__
#define GUNDERSCRIPT_COMPILER__H__

#include "compiler_source.h"

namespace gunderscript {

class CompilerImpl;

class Compiler {
public:
    Compiler(CompilerSourceInterface& source);
    ~Compiler();

private:
    CompilerImpl* pimpl_;
};

} // namespace gunderscript

#endif // GUNDERSCRIPT_COMPILER__H__
