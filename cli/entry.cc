// Gunderscript-2 CLI Application Entry Point
// (C) 2014-2015 Christian Gunderman

#include <iostream>

#include "cli.h"
#include "lexer.h"

using gunderscript::library::Lexer;

// Application Entry point
int main(int argc, const char** argv) {
    gunderscript::cli::ProcessArguments(argc, argv);
}
