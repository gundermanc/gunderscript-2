# Gunderscript 2 Scripting Engine
[![Build Status](https://travis-ci.org/gundermanc/gunderscript-2.svg?branch=feature%2Fci_build)](https://travis-ci.org/gundermanc/gunderscript-2)

(C) 2014-2016 Christian Gunderman
Contact Email: gundermanc@gmail.com

## Introduction:
Gunderscript 2 is the successor to the original Gunderscript scripting language.
For more information on Gunderscript, see http://github.com/gundermanc/gunderscript

Gunderscript 2 attempts to learn from the mistakes of Gunderscript 1 and is a
completely new language that is written mostly from scratch in a combination of
C/C++ and Gunderscript. The focus is to create a clean, object oriented scripting
language that borrows the best features from C#, Java, C, and C++ and that is
small, efficient, and modular in design and that support native code compilation
and garbage collected memory management.

Gunderscript 2 is intended to be a strongly typed language for the Javascript,
Lua, and Squirrel occupied market sector and will be aptly suited to use as a
game engine scripting language, Unix command shell, or general purpose scripting
environment.

## Case Western Reserve University Senior Project
This project is Christian Gunderman's senior project in Computer Science
bachelors of science degree. The main focus of this project is to create a very
solid, extremely well tested scripting engine that is easy to embed into an existing
C++ CMAKE project. Emphasis is given to testing, software engineering principles,
cross platform compatibility, and ongoing value after graduation.

This project will ideally become a community alternative to Lua, Squirrel, and
Javascript in the years to come.

## Roadmap and progress:
Below is a high level over view of the ongoing road map. For specific work items,
please see the Github issues page.
  - Lexer : 100%
  - Parser (except for IF, FOR, WHILE, DO/WHILE and other control flow) : 100%
  - Syntax tree walker : 100%
  - Semantic (type checker), minus control flow : 100%
  - Code generator for VM and VM : 35%
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

## Project Layout:
  - **cli**: Contains the example command line executable for Gunderscript 2.
  - **cmake**: Some additional CMAKE macros.
  - **common**: The Gunderscript engine common static library.
  - **compiler**: The Gunderscript compiler code static library.
  - **googletest**: The Google C++ Test framework.
  - **include**: Headers for the Gunderscript static libraries.
  - **nanojit**: A fork of Mozilla NanoJIT library.
  - **runtime**: Th Gunderscript runtime static library.

## Building:
Build the current code by generating build files for your platform with CMake.
Project has been tested  on Windows 10 Professional 10586 and is CI built on
Ubuntu 15.10 and Apple OS X and should be able to run cross platform with no issue.

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
  - Build. The gunderscript_cli.exe file is the command line executable.
    - To see intermediate script compilation steps use -l, -p, and -t.
    - To see NanoJIT IR code and assembly output you must define CMAKE_BUILD_TYPE=Debug
  - Other project files are Gunderscript components.

### Building on Linux and OS X
  - Run *./build_self_test.sh* script to download all dependencies and build
  - Refer to the script's contents for more detailed build work flow.

### Testing
All engine code is unit tested. To run tests, compile the project and run the
gunderscript_compiler_tests and gunderscript_runtime_tests executables. There are
currently over 270 tests covering almost all of the lexer, parser, and type checker
all well as all completed code gen functionality.

### Running a script
To run a script, download or compile the Gunderscript executable for your platform.
Run ./gunderscript_cli [filename] and Gunderscript will compile and run the script.
At the moment, the only end-to-end completed functionality is:
  - Main Method called at start.
  - +, -, *, /, % =, !=, &&, ||, !, <- operators
  - Variable assignment and reference.
  - Typecast between primitive types.
  - Primitive types:
    - bool
    - int8
    - int32
    - float32
  - Call functions, overload functions, etc.
  - Return values from functions.

No built in functions are provided yet. Return values printed on screen are the only
currently supported output.