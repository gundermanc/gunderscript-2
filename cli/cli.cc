// Gunderscript-2 CLI Methods
// (C) 2014-2015 Christian Gunderman

#include <cstring>
#include <iostream>
#include <vector>

#include "debug.h"
#include "exceptions.h"
#include "lexer_file_source.h"
#include "lexer.h"

using gunderscript::library::DebugPrintLexerToken;
using gunderscript::library::Exception;
using gunderscript::library::Lexer;
using gunderscript::library::LexerFileSource;
using gunderscript::library::LexerToken;

namespace gunderscript {
namespace cli {

// Lexes the files from the array and prints the symbols to the console in plain text
// debug format.
static void LexFiles(int file_count, const char** file_names) {

    // Iterate all files in the given memory.
    for (int i = 0; i < file_count; i++) {
        std::cout << "File: " << file_names[i] << "--------------------" << std::endl;

        try {
            LexerFileSource input(file_names[i]);
            Lexer lexer(input);

            for (const LexerToken* token = lexer.AdvanceNext(); lexer.has_next(); token = lexer.AdvanceNext()) {
                DebugPrintLexerToken(*token);
            }
        }
        catch (const Exception& ex) {
            std::cout << std::endl << "Error lexing, " << ex.what() << std::endl;
            break;
        }

        std::cout << "--------------------------" << std::endl;
    }
}

// Prints Gunderscript Application Description to stdout.
void PrintDescription() {
    std::cout << "Gunderscript 2 CLI Application" << std::endl;
    std::cout << "(C) 2014-2015 Christian Gunderman" << std::endl << std::endl;
    std::cout << "Usage: ./gunderscript_cli [parameters] [files]" << std::endl;
    std::cout << "Parameters:" << std::endl;
    std::cout << "  -l : Feed code through lexer stage only." << std::endl;
}

// Handles command line arguments and performs appropriate program action.
void ProcessArguments(int argc, const char** argv) {

    // While there are arguments left at the beginning starting with a hypen.
    for (int i = 1; i < argc && argv[i][0] == '-'; i++) {

        // Check argument length, all flags are two, print help if not two.
        if (strlen(argv[i]) != 2) {
            goto print_help;
        }

        // Determine which flag this is. Assume for now that flags are
        // mutually exclusive, we can change this later.
        // Return on success, goto on failure.
        switch (argv[i][1]) {
        case 'l':
        case 'L':
            LexFiles(argc - (i + 1), argv + (i + 1));
            return;
        default:
            // Screw you Dijstra! I'll use a goto here if I damn well please.
            // I agree that goto is OFTEN bad, but I disagree with blanket
            // statements on principle and Dijstra made lots of those.
            goto print_help;
        }
    }

    // No arguments or stuff otherwise went wrong.
    print_help:
    PrintDescription();
    return;
}

} // namespace cli
} // namespace gunderscript
