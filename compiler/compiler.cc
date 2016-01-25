// Gunderscript-2 Compiler API Implementation
// (C) 2016 Christian Gunderman

#include "gunderscript/compiler.h"

#include "lexer.h"
#include "lirgen_ast_walker.h"
#include "moduleimpl.h"
#include "parser.h"
#include "semantic_ast_walker.h"

using namespace gunderscript::compiler;

namespace gunderscript {

// Gunderscript Build Configuration.
const char* GunderscriptBuildConfigurationString() {
#ifdef _DEBUG
    return "DEBUG";
#else
    return "RELEASE";
#endif
}

// Gunderscript Build Timestamp.
const char* GunderscriptBuildTimestampString() {
    return __TIMESTAMP__;
}

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
    Module Compile(CompilerSourceInterface& source);
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
    Node* root = NULL;
    
    try {
        root = parser.Parse();

        // Run the Parser walk function if provided.
        if (parser_walk_func != NULL) {
            parser_walk_func(root);
        }

        if (stop_at == CompilerStage::PARSER) {
            delete root;
            return;
        }

        // Perform typechecking step.
        SemanticAstWalker semantic_walker(*root);
        semantic_walker.Walk();

        // Run Post-Typecheck AST walker function if given.
        if (typecheck_walk_func != NULL) {
            typecheck_walk_func(root);
        }
    }
    catch (const Exception&) {

        if (root != NULL) {
            delete root;
        }

        throw;
    }

    // Delete AST and return. Be sure to add a stop_at check for TYPECHECK
    // stage if you add more compiler stages.
    delete root;
    return;
}

// Compiles code from a source into a module.
Module CompilerImpl::Compile(CompilerSourceInterface& source) {
    Lexer lexer(source);
    Parser parser(lexer);
    
    Node* root = NULL;

    try {
        // Perform parse step:
        root = parser.Parse();
        SemanticAstWalker semantic_walker(*root);

        // Perform type checking step.
        semantic_walker.Walk();

        // Generate NanoJIT IR Code.
        LIRGenAstWalker lir_generator(*root);
        return lir_generator.Generate();
    }
    catch (const Exception&) {

        // Free on exception.
        if (root != NULL) {
            delete root;
        }

        throw;
    }
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
        TypeCheckedWalkFunc);
}

// The official compilation method for compiling a file to a Module.
// This method is a wrapper that uses the PIMPL pattern to obscure implementation
// details.
Module Compiler::Compile(CompilerSourceInterface& source) {
    return this->pimpl_->Compile(source);
}

// Destroy private implementation object when done.
Compiler::~Compiler() {
    delete pimpl_;
}

} // namespace gunderscript
