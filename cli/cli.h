// Gunderscript-2 CLI Methods
// (C) 2014-2015 Christian Gunderman

#ifndef GUNDERSCRIPT_CLI_CLI__H__
#define GUNDERSCRIPT_CLI_CLI__H__

#include <iostream>

namespace gunderscript {
namespace cli {

#define CLIRESULT_INT(r)  ((int)r)

// Command line exit codes.
enum class CliResult {
    OK = EXIT_SUCCESS,
    REQUIRES_FILES = EXIT_FAILURE + 1,
    INVALID_ARG = EXIT_FAILURE + 2,

    // When an exception occurs we WILL (as in not yet) return EXCEPTION_BASE + ex.code()
    // These are not currently guaranteed to be the same between iterations but can be used
    // for better scripting/IDE integration of the compilation process in the future.
    EXCEPTION_BASE = EXIT_FAILURE + 200
};

// Prints the command line application description and usage information.
void PrintDescription();

// Processes the command line arguments. argc is the number of arguments received
// and argv is an array of arguments passed in by the operating system.
CliResult ProcessArguments(int argc, const char** argv);

} // namespace cli
} // namespace gunderscript

#endif // GUNDERSCRIPT_CLI_CLI__H__