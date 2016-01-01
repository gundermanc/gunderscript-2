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
using gunderscript::library::SemanticAstWalkerTypeMismatchException;
using gunderscript::library::SymbolTableDuplicateKeyException;
using gunderscript::library::SymbolTableUndefinedSymbolException;

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
    delete root;
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
    delete root;
}

TEST(SemanticAstWalker, AutoPropertyDuplicateDefinition) {
    LexerStringSource source(std::string(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    int Y { concealed get; concealed set; }"
        "    int Y { public get; public set; }"
        "}"));
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_THROW(semantic_walker.Walk(), SymbolTableDuplicateKeyException);
    delete root;
}

TEST(SemanticAstWalker, PropertyWithBodyDuplicateDefinition) {
    LexerStringSource source(std::string(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    int Y { concealed get { } concealed set { } }"
        "    int Y { public get { } public set { } }"
        "}"));
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);
    
    EXPECT_THROW(semantic_walker.Walk(), SymbolTableDuplicateKeyException);
    delete root;
}

TEST(SemanticAstWalker, MixedAutoAndExpandedProperties) {
    // Case 1:
    {
        LexerStringSource source(std::string(
            "package \"Gundersoft\";"
            "public spec Test {"
            "    int Y { concealed get { } concealed set; }"
            "}"));
        Lexer lexer(source);
        Parser parser(lexer);

        Node* root = parser.Parse();

        SemanticAstWalker semantic_walker(*root);

        EXPECT_NO_THROW(semantic_walker.Walk());
        delete root;
    }

    // Case 2:
    {
        LexerStringSource source(std::string(
            "package \"Gundersoft\";"
            "public spec Test {"
            "    int Y { concealed get; concealed set { } }"
            "}"));
        Lexer lexer(source);
        Parser parser(lexer);

        Node* root = parser.Parse();

        SemanticAstWalker semantic_walker(*root);

        EXPECT_NO_THROW(semantic_walker.Walk());
        delete root;
    }
}

TEST(SemanticAstWalker, FunctionCallWithInvalidParameter) {
    LexerStringSource source(std::string(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    public int main(string x) { main(3); }"
        "}"));
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_THROW(semantic_walker.Walk(), SymbolTableUndefinedSymbolException);
    delete root;
}

TEST(SemanticAstWalker, FunctionCallNeedingTypecastedParameter) {
    // Gunderscript does not allow autoboxing from int to float.
    LexerStringSource source(std::string(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    public int main(float x) { main(3); }"
        "}"));
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_THROW(semantic_walker.Walk(), SymbolTableUndefinedSymbolException);
    delete root;
}

TEST(SemanticAstWalker, FunctionOverloads) {
    LexerStringSource source(std::string(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    public int main(int x) { }"
        "    public int main() { }"
        "}"));
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_NO_THROW(semantic_walker.Walk());
    delete root;
}

TEST(SemanticAstWalker, DuplicateFunctions) {
    // Case 1: same params and different return type.
    {
        LexerStringSource source(std::string(
            "package \"Gundersoft\";"
            "public spec Test {"
            "    public int main(int x) { }"
            "    public float main(int x) { }"
            "}"));
        Lexer lexer(source);
        Parser parser(lexer);

        Node* root = parser.Parse();

        SemanticAstWalker semantic_walker(*root);

        EXPECT_THROW(semantic_walker.Walk(), SymbolTableDuplicateKeyException);
        delete root;
    }

    // Case 2: different same params AND return type.
    {
        LexerStringSource source(std::string(
            "package \"Gundersoft\";"
            "public spec Test {"
            "    public int main(int x) { }"
            "    public int main(int x) { }"
            "}"));
        Lexer lexer(source);
        Parser parser(lexer);

        Node* root = parser.Parse();

        SemanticAstWalker semantic_walker(*root);

        EXPECT_THROW(semantic_walker.Walk(), SymbolTableDuplicateKeyException);
        delete root;
    }
}

TEST(SemanticAstWalker, FunctionAndPropertyAndVariableShareName) {
    // Check to make sure there are no collisions between symbols of different types.
    LexerStringSource source(std::string(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    public int X() {"
        "        X <- 3;"
        "    }"
        "    int X { public get; public set; }"
        "}"));
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_NO_THROW(semantic_walker.Walk());
    delete root;
}

TEST(SemanticAstWalker, FunctionParamTypeSymbols) {
    // Check to make sure that the Function Param symbols are being defined and are usable.
    LexerStringSource source(std::string(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    public int X(int X, int Y) {"
        "        X <- X + Y;"
        "    }"
        "}"));
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_NO_THROW(semantic_walker.Walk());
    delete root;
}

TEST(SemanticAstWalker, FunctionParamSymbolTypeReassign) {
    // Function params are declared in same scope as function variables and therefore
    // must only be assigned to the type with which they were originally declared.
    LexerStringSource source(std::string(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    public int X(int X, int Y) {"
        "        X <- 3.0;"
        "    }"
        "}"));
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_THROW(semantic_walker.Walk(), SemanticAstWalkerTypeMismatchException);
    delete root;
}

TEST(SemanticAstWalker, AttemptCrossTypeOperations) {
    // Cannot evaluate operations across types without explicit typecast.
    LexerStringSource source(std::string(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    public int X(int X, int Y) {"
        "        X <- 3.0;"
        "        Y <- X + 1;"
        "    }"
        "}"));
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_THROW(semantic_walker.Walk(), SemanticAstWalkerTypeMismatchException);
    delete root;
}

TEST(SemanticAstWalker, AttemptTypeReassignment) {
    // Once a variable is declared, its type is final and cannot be changed until it goes out of scope.
    LexerStringSource source(std::string(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    public int X(int X, int Y) {"
        "        X <- 3.0;"
        "        Y <- X;"
        "        Y <- 1;"
        "    }"
        "}"));
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_THROW(semantic_walker.Walk(), SemanticAstWalkerTypeMismatchException);
    delete root;
}

TEST(SemanticAstWalker, FunctionsOutOfOrder) {
    // Check to make sure that functions can call one another out of order.
    // This tests the prescan.
    LexerStringSource source(std::string(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    public int X() {"
        "        Y();"
        "        X();"
        "    }"
        "    public int Y() {"
        "        Y();"
        "        X();"
        "    }"
        "}"));
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_NO_THROW(semantic_walker.Walk());
    delete root;
}

TEST(SemanticAstWalker, FunctionCorrectReturnStatement) {
    // Check for no exception when function returns correctly.
    LexerStringSource source(std::string(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    public int X() {"
        "        return 3 + 4;"
        "    }"
        "}"));
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_NO_THROW(semantic_walker.Walk());
    delete root;
}

TEST(SemanticAstWalker, FunctionIncorrectReturnStatement) {
    // Check for exception when function returns incorrect type.
    LexerStringSource source(std::string(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    public int X() {"
        "        return true;"
        "    }"
        "}"));
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_THROW(semantic_walker.Walk(), SemanticAstWalkerTypeMismatchException);
    delete root;
}

TEST(SemanticAstWalker, BlockStatementScoping) {
    // Check that variables are scoped by block without being replaced.
    LexerStringSource source(std::string(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    public int X() { "
        "        X <- 1;"
        "        { X <- true; } "
        "        X <- 2;"
        "    }"
        "}"));
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);
    semantic_walker.Walk();
    //EXPECT_NO_THROW(semantic_walker.Walk());
    delete root;
}

TEST(SemanticAstWalker, PropertyReturnCorrectly) {
    // Check that returning correctly from a property does not throw.
    LexerStringSource source(std::string(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    int X {"
        "        public get { return 0; }"
        "        public set { }"
        "    }"
        "}"));
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_NO_THROW(semantic_walker.Walk());
    delete root;
}

TEST(SemanticAstWalker, PropertyReturnFromSet) {
    // Check that returning from a setter does not throw.
    LexerStringSource source(std::string(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    int X {"
        "        public get { return 0; }"
        "        public set { return 0; }"
        "    }"
        "}"));
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_THROW(semantic_walker.Walk(), SemanticAstWalkerTypeMismatchException);
    delete root;
}

TEST(SemanticAstWalker, PropertyReturnInvalidType) {
    // Check that we throw if we try to return invalid type from a property.
    LexerStringSource source(std::string(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    int X {"
        "        public get { return true; }"
        "        public set { }"
        "    }"
        "}"));
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_THROW(semantic_walker.Walk(), SemanticAstWalkerTypeMismatchException);
    delete root;
}

TEST(SemanticAstWalker, AddInvalidType) {
    LexerStringSource source(std::string(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    public int X(int x) {"
        "        main(3 + 3.0);"
        "    }"
        "}"));
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_THROW(semantic_walker.Walk(), SemanticAstWalkerTypeMismatchException);
    delete root;
}

TEST(SemanticAstWalker, AddString) {
    LexerStringSource source(std::string(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    public int X(string x) {"
        "        main(\"sfsf\" + \"sfsf\");"
        "    }"
        "}"));
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_NO_THROW(semantic_walker.Walk());
    delete root;
}

TEST(SemanticAstWalker, SubtractString) {
    LexerStringSource source(std::string(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    public int X(string x) {"
        "        main(\"sfsf\" - \"sfsf\");"
        "    }"
        "}"));
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_THROW(semantic_walker.Walk(), SemanticAstWalkerTypeMismatchException);
    delete root;
}

TEST(SemanticAstWalker, ModString) {
    LexerStringSource source(std::string(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    public int X(string x) {"
        "        main(\"sfsf\" % \"sfsf\");"
        "    }"
        "}"));
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_THROW(semantic_walker.Walk(), SemanticAstWalkerTypeMismatchException);
    delete root;
}

TEST(SemanticAstWalker, MulInvalidType) {
    LexerStringSource source(std::string(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    public int X(bool x) {"
        "        main(true * true);"
        "    }"
        "}"));
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_THROW(semantic_walker.Walk(), SemanticAstWalkerTypeMismatchException);
    delete root;
}

TEST(SemanticAstWalker, DivInvalidType) {
    LexerStringSource source(std::string(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    public int X(bool x) {"
        "        main(true \ true);"
        "    }"
        "}"));
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_THROW(semantic_walker.Walk(), SemanticAstWalkerTypeMismatchException);
    delete root;
}

TEST(SemanticAstWalker, AndInvalidType) {
    LexerStringSource source(std::string(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    public int X(bool x) {"
        "        main(3 && 3);"
        "    }"
        "}"));
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_THROW(semantic_walker.Walk(), SemanticAstWalkerTypeMismatchException);
    delete root;
}

TEST(SemanticAstWalker, OrInvalidType) {
    LexerStringSource source(std::string(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    public int X(bool x) {"
        "        main(3.0 || 3.0);"
        "    }"
        "}"));
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_THROW(semantic_walker.Walk(), SemanticAstWalkerTypeMismatchException);
    delete root;
}

TEST(SemanticAstWalker, GreaterInvalidType) {
    LexerStringSource source(std::string(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    public int X(bool x) {"
        "        main(true > false);"
        "    }"
        "}"));
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_THROW(semantic_walker.Walk(), SemanticAstWalkerTypeMismatchException);
    delete root;
}

TEST(SemanticAstWalker, LessInvalidType) {
    LexerStringSource source(std::string(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    public int X(bool x) {"
        "        main(true < false);"
        "    }"
        "}"));
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_THROW(semantic_walker.Walk(), SemanticAstWalkerTypeMismatchException);
    delete root;
}

TEST(SemanticAstWalker, GreaterEqualsInvalidType) {
    LexerStringSource source(std::string(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    public int X(string x) {"
        "        main(\"h\" >= \"d\");"
        "    }"
        "}"));
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_THROW(semantic_walker.Walk(), SemanticAstWalkerTypeMismatchException);
    delete root;
}

TEST(SemanticAstWalker, LessEqualsInvalidType) {
    LexerStringSource source(std::string(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    public int X(string x) {"
        "        main(\"h\" <= \"d\");"
        "    }"
        "}"));
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_THROW(semantic_walker.Walk(), SemanticAstWalkerTypeMismatchException);
    delete root;
}

TEST(SemanticAstWalker, Combined) {
    LexerStringSource source(std::string(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    public int X(bool x) {"
        "        main( ((-(-1+2) / 3)) > (0 * (4 % -5)) || !(!(3 < 2)) && (4 >= 5) && (1 <= 6));"
        "    }"
        "}"));
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);
    semantic_walker.Walk();
    //EXPECT_NO_THROW(semantic_walker.Walk());
    delete root;
}