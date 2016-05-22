// Gunderscript-2 Testing Macros
// (C) 2016 Christian Gunderman

#ifndef GUNDERSCRIPT_TESTING_MACROS__H__
#define GUNDERSCRIPT_TESTING_MACROS__H__

#include "gtest/gtest.h"

#include "gunderscript/compiler.h"
#include "gunderscript/virtual_machine.h"
#include "gunderscript/exceptions.h"

using namespace gunderscript;

// Pretty basic macro for checking that the exception thrown by stmt
// has the expected status.
#define EXPECT_STATUS(stmt, exception_status)                   \
do {                                                            \
try {                                                           \
    stmt;                                                       \
}                                                               \
catch (const Exception& ex) {                                   \
    EXPECT_EQ((exception_status).code(), ex.status().code());   \
    break;                                                     \
}                                                               \
FAIL();                                                         \
} while (0);


// Integration Testing Macros:
// These macros define quick and dirty entry points for end-to-end testing
// of Gunderscript. They lex, parse, typecheck, compile, and run the code.
// They are defined as macros so we are free to create a legit public API
// for Gunderscript in the future without changing hundreds of lines of code.

// Runs the Testing::main() method that returns INT32 with no arguments.
// Provides only a class and no methods. You must provide these yourself.
#define COMPILE_AND_RUN_INT_MAIN_CLASS(class_members)            \
([]() {                                                          \
std::string input("package \"Foo\"; " class_members); \
CommonResources common_resources;                               \
CompilerStringSource string_source(input);                      \
Compiler compiler(common_resources);                            \
Module module;                                                  \
compiler.Compile(string_source, module);                        \
VirtualMachine vm(common_resources);                            \
return vm.HackyRunScriptMainInt(module);                        \
})()

// Runs the Testing::main() method that returns INT32 with no arguments.
#define COMPILE_AND_RUN_INT_MAIN_LINES(lines)                   \
([]() {                                                          \
std::string input("package \"Foo\"; public int32 main() { " lines " }"); \
CommonResources common_resources;                               \
CompilerStringSource string_source(input);                      \
Compiler compiler(common_resources);                            \
Module module;                                                  \
compiler.Compile(string_source, module);                        \
VirtualMachine vm(common_resources);                            \
return vm.HackyRunScriptMainInt(module);                        \
})()

// Runs the Testing::main() method that returns FLOAT32 with no arguments.
#define COMPILE_AND_RUN_FLOAT_MAIN_LINES(lines)                   \
([]() {                                                          \
std::string input("package \"Foo\"; public float32 main() { " lines " }"); \
CommonResources common_resources;                               \
CompilerStringSource string_source(input);                      \
Compiler compiler(common_resources);                            \
Module module;                                                  \
compiler.Compile(string_source, module);                        \
VirtualMachine vm(common_resources);                            \
return vm.HackyRunScriptMainFloat(module);                      \
})()

// Runs the Testing::main() method that returns BOOL with no arguments.
#define COMPILE_AND_RUN_BOOL_MAIN_LINES(lines)                   \
([]() {                                                          \
std::string input("package \"Foo\"; public bool main() { " lines " }"); \
CommonResources common_resources;                               \
CompilerStringSource string_source(input);                      \
Compiler compiler(common_resources);                            \
Module module;                                                  \
compiler.Compile(string_source, module);                        \
VirtualMachine vm(common_resources);                            \
return vm.HackyRunScriptMainBool(module);                       \
})()

// Runs the Testing::main() method that returns int8 with no arguments.
#define COMPILE_AND_RUN_INT8_MAIN_LINES(lines)                   \
([]() {                                                          \
std::string input("package \"Foo\"; public int8 main() { " lines " }"); \
CommonResources common_resources;                               \
CompilerStringSource string_source(input);                      \
Compiler compiler(common_resources);                            \
Module module;                                                  \
compiler.Compile(string_source, module);                        \
VirtualMachine vm(common_resources);                            \
return vm.HackyRunScriptMainChar(module);                       \
})()
#endif // GUNDERSCRIPT_TESTING_MACROS__H__
