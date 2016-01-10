# Gunderscript 2 Programming Language
[![Build Status](https://travis-ci.org/gundermanc/gunderscript-2.svg?branch=feature%2Fci_build)](https://travis-ci.org/gundermanc/gunderscript-2)

(C) 2014-2016 Christian Gunderman
Contact Email: gundermanc@gmail.com

## Introduction:
Gunderscript 2 is the successor to the original Gunderscript scripting language.
For more information on Gunderscript, see http://github.com/gundermanc/gunderscript

Gunderscript 2 attempts to learn from the mistakes of Gunderscript 1 and is a
completely new language that will be written from scratch in a combination of
C/C++ and Gunderscript. The focus is to create a clean, object oriented scripting
language that borrows the best features from C#, Java, C, and C++ and that is
small, efficient, and modular in design, allowing for the user to target either
the Gunderscript 2 interpreter or Gunderscript 2 VM.

Gunderscript 2 is intended to be a strongly typed language for the Javascript,
Lua, and Squirrel occupied market sector and will be aptly suited to use as a
game engine scripting language, Unix command shell, or general purpose scripting
environment.

## Roadmap:
  - Lexer : 100%
  - Parser (except for IF, FOR, WHILE, DO/WHILE and other control flow) : 100%
  - Syntax tree walker : 100%
  - Semantic (type checker), minus control flow : 100%
  - AST interpreter (non-compiled), minus control flow : 0%
  - End to end control flow support
    - IF : 0%
    - FOR : 0%
    - WHILE : 0%
    - DO/WHILE : 0%
  - Investigate more complex language features:
    - Exceptions
    - Switches
    - Enhanced or "shortcut" operators
  - Refactor and cleanup
  - Unified, extensible engine API.
  - Code generator for VM and VM : 0%

Gunderscript 2 is currently under initial construction. If you are interested in
joining this project, I am in need of developers and a program manager.

## Attention:
I am seeking developers to help me complete Gunderscript 2 and test it. If anyone
is interested, please email gundermanc@gmail.com.

## Project Layout:
  - **cli**: Contains the example command line interface for Gunderscript 2.
  - **library**: Contains the Gunderscript engine code and API.

## Building:
Build the current code by generating build files for your platform with CMake.
Project has been tested on Windows 10 Professional 10586 and Ubuntu 15.10 and
should be able to run cross platform.

Officially Gunderscript 2 supports the following build configurations via CMAKE:
  - Windows 7 and newer, Visual Studio 2015, MSVC++ 19.*
  - Ubuntu 15.10, GCC 4.9, GNU Make.
  - Mac OS X, GCC 4.9, GNU Make.

### Building on Windows:
  - Install:
    - Visual Studio 2015 Community or Newer
    - CMAKE or CMAKE GUI.
    - Git
  - Git Clone the repo
  - Run *git submodule init* and then *git submodule update* in root of repo.
  - Run *cmake .* in root of repo.
  - Launch GUNDERSCRIPT.sln solution file in Visual Studio.

### Building on Linux
  - Run *./build_self_test.sh* script to download all dependencies and build
  - Refer to the script's contents for more detailed build work flow.

### Testing
All engine code is unit tested. To run tests, compile the project and run the
gunderscript_tests executable.