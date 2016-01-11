// Gunderscript-2 CLI Methods
// (C) 2014-2016 Christian Gunderman

#include <cstring>
#include <iostream>
#include <vector>

#include "cli.h"
#include "debug.h"
#include "gunderscript/compiler.h"

using namespace gunderscript;

namespace gunderscript {
namespace cli {

// Performs a debuggable compilation with access to the AST.
static CliResult DebugCompile(
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
            CompilerFileSource file_source(file_names[i]);
            Compiler compiler;

            // Run a debug compilation.
            compiler.DebugCompilation(
                file_source,
                stop_at,
                lexer_iteration_func,
                parser_walk_func,
                typecheck_walk_func);
        }
        catch (const Exception& ex) {
            std::cout << std::endl << "GS" << ex.status().code() << ": Near Line " << ex.line() << " Column " << ex.column() << " " << ex.what() << std::endl;

            // Uncomment this line to see where in the implementation an exception was thrown from.
            // std::cout << "Error originated from " << std::endl << ex.impl_file() << std::endl << " at line " << ex.impl_line() << std::endl;
            return (CliResult)(CLIRESULT_INT(CliResult::EXCEPTION_BASE) + ex.status().code());
        }

        std::cout << "--------------------------" << std::endl;
    }

    return CliResult::OK;
}

// Lexes the files from the array and prints the symbols to the console in plain text
// debug format.
static CliResult LexFiles(int file_count, const char** file_names) {
    return DebugCompile(
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
        file_count,
        file_names,
        CompilerStage::TYPE_CHECKER,
        NULL,
        NULL,
        [](const Node* root) { DebugPrintNode(root); });
}

// Prints Gunderscript Application Description to stdout.
void PrintDescription() {
    std::cout << "Gunderscript 2 CLI Application, Compiled " << __DATE__ << std::endl;
    std::cout << "(C) 2014-2016 Christian Gunderman" << std::endl << std::endl;
    std::cout << "Usage: ./gunderscript_cli [parameters] [files]" << std::endl;
    std::cout << "Parameters:" << std::endl;
    std::cout << "  -l : Feed code through lexer stage only and tokenize output." << std::endl;
    std::cout << "  -p : Feed code through lexer and parser stages only and emit serialized AST." << std::endl;
    std::cout << "  -t : Feed code through lexer and parser and typechecker and emit AST." << std::endl;
}

// Handles command line arguments and performs appropriate program action.
CliResult ProcessArguments(int argc, const char** argv) {

    CliResult result = CliResult::INVALID_ARG;

    // While there are arguments left at the beginning starting with a hypen.
    for (int i = 1; i < argc && argv[i][0] == '-'; i++) {

        // Check argument length, all flags are two, print help if not two.
        if (strlen(argv[i]) != 2) {
            result = CliResult::INVALID_ARG;
            goto eval_cli_result;
        }

        // Determine which flag this is. Assume for now that flags are
        // mutually exclusive, we can change this later.
        // Return on success, goto on failure.
        switch (argv[i][1]) {
        case 'l':
        case 'L':
            result = LexFiles(argc - (i + 1), argv + (i + 1));
            break;
        case 'p':
        case 'P':
            result = ParseFiles(argc - (i + 1), argv + (i + 1));
            break;
        case 't':
        case 'T':
            result = TypeCheckFiles(argc - (i + 1), argv + (i + 1));
            break;
        default:
            // Screw you Dijstra! I'll use a goto here if I damn well please.
            // I agree that goto is OFTEN bad, but I disagree with blanket
            // statements on principle and Dijstra made lots of those.
            goto eval_cli_result;
        }
    }

    // We're done here, if invalid args, let the user know.
    // Other errors are expected to have already printed.
eval_cli_result:
    if (result == CliResult::INVALID_ARG) {
        PrintDescription();
    }

    return result;
}

} // namespace cli
} // namespace gunderscript