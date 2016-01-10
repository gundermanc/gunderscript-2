// Gunderscript-2 Compiler API Implementation
// (C) 2016 Christian Gunderman

#include "gunderscript/compiler.h"

#include "lexer.h"
#include "parser.h"
#include "semantic_ast_walker.h"

using namespace gunderscript::library;

namespace gunderscript {

// Compiler Class Private Implementation class. This is part of the PIMPL
// pattern and is used to hide implementation details from callers.
class CompilerImpl {
public:
    CompilerImpl() { }
    void DebugCompilation(
        CompilerSourceInterface& source,
        CompilerStage stop_at,
        LexerTokenFunc lexer_iteration_func,
        ParserNodeFunc parser_walk_func,
        ParserNodeFunc typecheck_walk_func);
};

// Implementation of compiler DebugCompilation function.
void CompilerImpl::DebugCompilation(
    CompilerSourceInterface& source,
    CompilerStage stop_at,
    LexerTokenFunc lexer_iteration_func,
    ParserNodeFunc parser_walk_func,
    ParserNodeFunc typecheck_walk_func) {

    Lexer lexer(source);

    // Run the Lexer token iterator function if provided.
    if (lexer_iteration_func != NULL) {
        for (const LexerToken* token = lexer.AdvanceNext(); lexer.has_next(); token = lexer.AdvanceNext()) {
            lexer_iteration_func(*token);
        }
    }

    if (stop_at == CompilerStage::LEXER) {
        return;
    }

    // Perform parsing step.
    Parser parser(lexer);
    Node* root = parser.Parse();

    // Run the Parser walk function if provided.
    if (parser_walk_func != NULL) {
        parser_walk_func(root);
    }

    if (stop_at == CompilerStage::PARSER) {
        delete root;
        return;
    }

    // Run Post-Typecheck AST walker function if given.
    if (typecheck_walk_func != NULL) {
        typecheck_walk_func(root);
    }

    // Delete AST and return. Be sure to add a stop_at check for TYPECHECK
    // stage if you add more compiler stages.
    delete root;
    return;
}

// Public constructor.
Compiler::Compiler() 
    : pimpl_(new CompilerImpl()) {
}

// The debug compilation method for snooping on the build process steps.
// This method is a wrapper that uses the PIMPL pattern to obscure implementation
// details.
void Compiler::DebugCompilation(
    CompilerSourceInterface& source,
    CompilerStage stop_at,
    void(*LexerIterationFunc)(const LexerToken& token),
    void(*ParserWalkFunc)(const Node* root),
    void(*TypeCheckedWalkFunc)(const Node* root)) {
    pimpl_->DebugCompilation(
        source,
        stop_at,
        LexerIterationFunc,
        ParserWalkFunc,
        ParserWalkFunc);
}

// Destroy private implementation object when done.
Compiler::~Compiler() {
    delete pimpl_;
}

} // namespace gunderscript
