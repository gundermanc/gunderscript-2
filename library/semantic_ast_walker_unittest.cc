// Gunderscript 2 Semantic (type) Checker for AST Unit Test
// (C) 2015 Christian Gunderman

#include "gtest/gtest.h"

#include "lexer.h"
#include "node.h"
#include "parser.h"
#include "semantic_ast_walker.h"

using gunderscript::library::LexerStringSource;
using gunderscript::library::Lexer;
using gunderscript::library::Node;
using gunderscript::library::Parser;
using gunderscript::library::SemanticAstWalker;
using gunderscript::library::SemanticAstWalkerInvalidPackageNameException;
using gunderscript::library::SymbolTableDuplicateKeyException;

// This module tests the semantic checker layer for Gunderscript 2.
// The tests cover only the expected positive and negative cases from
// ASTs generated by the parser and do not check for the IllegalStateExceptions
// thrown by invalid ASTs. This is done intentionally since the IllegalStateExceptions
// are there for the purpose of debug assertions.

TEST(SemanticAstWalker, ModuleNameTrailingPeriodThrows) {
    LexerStringSource source(std::string("package \"Gundersoft.\";"));
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_THROW(semantic_walker.Walk(), SemanticAstWalkerInvalidPackageNameException);

    delete root;
}

TEST(SemanticAstWalker, ModuleNameEmptyThrows) {
    LexerStringSource source(std::string("package \"\";"));
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_THROW(semantic_walker.Walk(), SemanticAstWalkerInvalidPackageNameException);

    delete root;
}

TEST(SemanticAstWalker, ModuleNameOnlyPeriodThrows) {
    LexerStringSource source(std::string("package \".\";"));
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_THROW(semantic_walker.Walk(), SemanticAstWalkerInvalidPackageNameException);

    delete root;
}

TEST(SemanticAstWalker, ModuleNameStartsWithPeriodThrows) {
    LexerStringSource source(std::string("package \".Hello\";"));
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_THROW(semantic_walker.Walk(), SemanticAstWalkerInvalidPackageNameException);

    delete root;
}

TEST(SemanticAstWalker, ModuleDependsNameTrailingPeriodThrows) {
    LexerStringSource source(std::string(
        "package \"Gundersoft\";"
        "depends \"Gundersoft.\";"));
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_THROW(semantic_walker.Walk(), SemanticAstWalkerInvalidPackageNameException);

    delete root;
}

TEST(SemanticAstWalker, ModuleDependsNameEmptyThrows) {
    LexerStringSource source(std::string(
        "package \"Gundersoft\";"
        "depends \"\";"));
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_THROW(semantic_walker.Walk(), SemanticAstWalkerInvalidPackageNameException);

    delete root;
}

TEST(SemanticAstWalker, ModuleDependsNameOnlyPeriodThrows) {
    LexerStringSource source(std::string(
        "package \"Gundersoft\";"
        "depends \".\";"));
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_THROW(semantic_walker.Walk(), SemanticAstWalkerInvalidPackageNameException);

    delete root;
}

TEST(SemanticAstWalker, ModuleDependsNameStartsWithPeriodThrows) {
    LexerStringSource source(std::string(
        "package \"Gundersoft\";"
        "depends \".Foo\";"));
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_THROW(semantic_walker.Walk(), SemanticAstWalkerInvalidPackageNameException);

    delete root;
}

TEST(SemanticAstWalker, SpecDuplicateDefinition) {
    LexerStringSource source(std::string(
        "package \"Gundersoft\";"
        "public spec Test { } "
        "public spec Test { }"));
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_THROW(semantic_walker.Walk(), SymbolTableDuplicateKeyException);
}

TEST(SemanticAstWalker, FunctionDuplicateDefinition) {
    LexerStringSource source(std::string(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    public int main(int x, string y) { }"
        "    public int main(int x, string y) { }"
        "}"));
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_THROW(semantic_walker.Walk(), SymbolTableDuplicateKeyException);
}