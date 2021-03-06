// Gunderscript-2 CLI Methods
// (C) 2014-2016 Christian Gunderman

#include <cstring>
#include <iostream>

#include "cli.h"
#include "debug.h"

#include "gunderscript/compiler.h"
#include "gunderscript/virtual_machine.h"

using namespace gunderscript;

namespace gunderscript {
namespace cli {

// Prints an exception message to the screen and returns the CliResult of the exception.
static CliResult PrintException(const Exception& ex) {
    std::cout << std::endl
        << "GS" << ex.status().code() << ": Near Line " << ex.line()
        << " Column " << ex.column() << " " << ex.what() << std::endl;

#ifdef _DEBUG
    // In DEBUG build config print the origin point of the exception.
    std::cout
        << "  >> Error originated from line " << ex.impl_line() << " in "
        << std::endl << "  >> " << ex.impl_file() << std::endl;
#endif
    return (CliResult)(CLIRESULT_INT(CliResult::EXCEPTION_BASE) + ex.status().code());
}

// Performs a debuggable compilation with access to the AST.
static CliResult DebugCompile(
#ifdef NJ_VERBOSE
    bool verbose_asm,
#endif
    int file_count,
    const char** file_names,
    CompilerStage stop_at,
    LexerTokenFunc lexer_iteration_func,
    ParserNodeFunc parser_walk_func,
    ParserNodeFunc typecheck_walk_func) {

    // Check for input files before we go any farther.
    if (file_count == 0) {
        return CliResult::REQUIRES_FILES;
    }

    // Iterate all files in the given memory.
    for (int i = 0; i < file_count; i++) {
        std::cout << "File: " << file_names[i] << "--------------------" << std::endl;

        try {
            CommonResources common_resources;
            CompilerFileSource file_source(file_names[i]);
            Compiler compiler(common_resources);

#ifdef NJ_VERBOSE
            common_resources.set_verbose_asm(verbose_asm);
#endif // NJ_VERBOSE

            // Run a debug compilation.
            compiler.DebugCompilation(
                file_source,
                stop_at,
                lexer_iteration_func,
                parser_walk_func,
                typecheck_walk_func);
        }
        catch (const Exception& ex) {
            return PrintException(ex);
        }

        std::cout << "--------------------------" << std::endl;
    }

    return CliResult::OK;
}

// Lexes the files from the array and prints the symbols to the console in plain text
// debug format.
static CliResult LexFiles(int file_count, const char** file_names) {
    return DebugCompile(
#ifdef NJ_VERBOSE
        false,
#endif // NJ_VERBOSE
        file_count,
        file_names,
        CompilerStage::LEXER,
        [](const LexerToken& token) { DebugPrintLexerToken(token); },
        NULL,
        NULL);
}

// Parses the files from the array and prints the serialized abstract syntax tree
// nodes to the command line in the debug format.
static CliResult ParseFiles(int file_count, const char** file_names) {
    return DebugCompile(
#ifdef NJ_VERBOSE
        false,
#endif // NJ_VERBOSE
        file_count,
        file_names,
        CompilerStage::PARSER,
        NULL,
        [](const Node* root) { DebugPrintNode(root); },
        NULL);
}

// Lexes, parses, and type checks a file and prints the serialized AST
// nodes to the command line in the debug format.
static CliResult TypeCheckFiles(int file_count, const char** file_names) {
    return DebugCompile(
#ifdef NJ_VERBOSE
        false,
#endif // NJ_VERBOSE
        file_count,
        file_names,
        CompilerStage::TYPE_CHECKER,
        NULL,
        NULL,
        [](const Node* root) { DebugPrintNode(root); });
}

// Lexes, parsers, typechecks, generates code, assembles, and prints IR and assembly code.
static CliResult AssembleFiles(int file_count, const char** file_names) {
    return DebugCompile(
#ifdef NJ_VERBOSE
        true,
#endif // NJ_VERBOSE
        file_count,
        file_names,
        CompilerStage::ASSEMBLY,
        NULL,
        NULL,
        NULL);
}

// Lexes, parses, and type checks a file and then generates and runs the code.
static CliResult RunFiles(int file_count, const char** file_names) {
    // Check for input files before we go any farther.
    if (file_count == 0) {
        return CliResult::REQUIRES_FILES;
    }

    // Iterate all files in the given memory.
    for (int i = 0; i < file_count; i++) {
        std::cout << "File: " << file_names[i] << "--------------------" << std::endl;

        try {
            CommonResources common_resources;
            CompilerFileSource file_source(file_names[i]);
            Compiler compiler(common_resources);
            Module module;

            // Run a debug compilation.
            compiler.Compile(file_source, module);

            // Run the function.
            VirtualMachine vm(common_resources);
            std::cout << "Script result: " << vm.HackyRunScriptMainInt(module);
        }
        catch (const Exception& ex) {
            return PrintException(ex);
        }

        std::cout << "--------------------------" << std::endl;
    }

    return CliResult::OK;
}

// Prints Gunderscript Application Description to stdout.
void PrintDescription() {
    std::cout << "Gunderscript 2 CLI Application, Compiled "
        << GunderscriptBuildConfigurationString()
        << " Configuration on "
        << GunderscriptBuildTimestampString()
        << std::endl;
    std::cout << "(C) 2014-2016 Christian Gunderman" << std::endl << std::endl;
    std::cout << "Uses NanoJIT (C) Mozilla, Subject to MPL https://www.mozilla.org/en-US/MPL/2.0/" << std::endl << std::endl;
    std::cout << "Usage: ./gunderscript_cli [parameters] [files]" << std::endl;
    std::cout << "Parameters:" << std::endl;
    std::cout << "  -l  : Feed code through lexer stage only and tokenize output." << std::endl;
    std::cout << "  -p  : Feed code through lexer and parser stages only and emit serialized AST." << std::endl;
    std::cout << "  -t  : Feed code through lexer and parser and typechecker and emit AST." << std::endl;

#ifdef NJ_VERBOSE
    std::cout << "  -a  : Feed code throgh lexer and parser and typechecker and emit IR and assembly." << std::endl;
#endif // NJ_VERBOSE

    std::cout << " None : Feed code through lexer and parser and typechecker and generate and run code." << std::endl;
}

// Handles command line arguments and performs appropriate program action.
CliResult ProcessArguments(int argc, const char** argv) {

    CliResult result = CliResult::INVALID_ARG;

    const char* foo = argv[1];
    // Check argument length, all flags are two, print help if not two.
    if (argc >= 2 && argv[1][0] == '-' && strlen(argv[1]) == 2) {

        // Determine which flag this is. Assume for now that flags are
        // mutually exclusive, we can change this later.
        // Return on success, goto on failure.
        switch (argv[1][1]) {
        case 'l':
        case 'L':
            result = LexFiles(argc - 2, argv + 2);
            break;
        case 'p':
        case 'P':
            result = ParseFiles(argc - 2, argv + 2);
            break;
        case 't':
        case 'T':
            result = TypeCheckFiles(argc - 2, argv + 2);
            break;
#ifdef NJ_VERBOSE
        case 'a':
        case 'A':
            result = AssembleFiles(argc - 2, argv + 2);
            break;
#endif // NJ_VERBOSE
        }
    }
    else {
        result = RunFiles(argc - 1, argv + 1);
    }

    // We're done here, if invalid args, let the user know.
    if (result == CliResult::INVALID_ARG ||
        result == CliResult::REQUIRES_FILES) {
        PrintDescription();
    }

    return result;
}

} // namespace cli
} // namespace gunderscript