// Gunderscript-2 Compiler API
// (C) 2016 Christian Gunderman

#ifndef GUNDERSCRIPT_COMPILER__H__
#define GUNDERSCRIPT_COMPILER__H__

#include "compiler_source.h"
#include "lexer_resources.h"
#include "node.h"

namespace gunderscript {

// Identifies stages of compilation.
enum class CompilerStage {
    LEXER = 100,
    PARSER = 200,
    TYPE_CHECKER = 300
};

typedef void(*LexerTokenFunc)(const LexerToken& token);
typedef void(*ParserNodeFunc)(const Node* root);

class CompilerImpl;

class Compiler {
public:
    Compiler();
    ~Compiler();
    void DebugCompilation(
        CompilerSourceInterface& source,
        CompilerStage stop_at,
        LexerTokenFunc lexer_iteration_func,
        ParserNodeFunc parser_walk_func,
        ParserNodeFunc typecheck_walk_func);

private:
    CompilerImpl* pimpl_;
};

} // namespace gunderscript

#endif // GUNDERSCRIPT_COMPILER__H__
