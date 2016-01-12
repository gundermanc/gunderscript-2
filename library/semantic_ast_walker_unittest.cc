// Gunderscript 2 Semantic (type) Checker for AST Unit Test
// (C) 2015-2016 Christian Gunderman

#include "gtest/gtest.h"
#include "testing_macros.h"

#include "gunderscript/node.h"

#include "lexer.h"
#include "parser.h"
#include "semantic_ast_walker.h"

using namespace gunderscript;
using gunderscript::library::Lexer;
using gunderscript::library::Parser;
using gunderscript::library::SemanticAstWalker;

// This module tests the semantic checker layer for Gunderscript 2.
// The tests cover only the expected positive and negative cases from
// ASTs generated by the parser and do not check for the STATUS_ILLEGAL_STATE
// thrown by invalid ASTs. This is done intentionally since the STATUS_ILLEGAL_STATE
// are there for the purpose of debug assertions.

TEST(SemanticAstWalker, ModuleNameTrailingPeriodThrows) {
    std::string input("package \"Gundersoft.\";");
    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_STATUS(semantic_walker.Walk(), STATUS_SEMANTIC_INVALID_PACKAGE);

    delete root;
}

TEST(SemanticAstWalker, ModuleNameEmptyThrows) {
    std::string input = std::string("package \"\";");
    CompilerStringSource source(input);

    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_STATUS(semantic_walker.Walk(), STATUS_SEMANTIC_INVALID_PACKAGE);

    delete root;
}

TEST(SemanticAstWalker, ModuleNameOnlyPeriodThrows) {
    std::string input("package \".\";");
    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_STATUS(semantic_walker.Walk(), STATUS_SEMANTIC_INVALID_PACKAGE);

    delete root;
}

TEST(SemanticAstWalker, ModuleNameStartsWithPeriodThrows) {
    std::string input("package \".Hello\";");
    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_STATUS(semantic_walker.Walk(), STATUS_SEMANTIC_INVALID_PACKAGE);

    delete root;
}

TEST(SemanticAstWalker, ModuleDependsNameTrailingPeriodThrows) {
    std::string input(
        "package \"Gundersoft\";"
        "depends \"Gundersoft.\";");
    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_STATUS(semantic_walker.Walk(), STATUS_SEMANTIC_INVALID_PACKAGE);

    delete root;
}

TEST(SemanticAstWalker, ModuleDependsNameEmptyThrows) {
    std::string input(
        "package \"Gundersoft\";"
        "depends \"\";");
    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_STATUS(semantic_walker.Walk(), STATUS_SEMANTIC_INVALID_PACKAGE);

    delete root;
}

TEST(SemanticAstWalker, ModuleDependsNameOnlyPeriodThrows) {
    std::string input(
        "package \"Gundersoft\";"
        "depends \".\";");
    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_STATUS(semantic_walker.Walk(), STATUS_SEMANTIC_INVALID_PACKAGE);

    delete root;
}

TEST(SemanticAstWalker, ModuleDependsNameStartsWithPeriodThrows) {
    std::string input(
        "package \"Gundersoft\";"
        "depends \".Foo\";");
    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_STATUS(semantic_walker.Walk(), STATUS_SEMANTIC_INVALID_PACKAGE);

    delete root;
}

TEST(SemanticAstWalker, SpecDuplicateDefinition) {
    std::string input(
        "package \"Gundersoft\";"
        "public spec Test { } "
        "public spec Test { }");
    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_STATUS(semantic_walker.Walk(), STATUS_SEMANTIC_DUPLICATE_SPEC);
    delete root;
}

TEST(SemanticAstWalker, FunctionDuplicateDefinition) {
    std::string input(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    public int main(int x, string y) { }"
        "    public int main(int x, string y) { }"
        "}");
    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_STATUS(semantic_walker.Walk(), STATUS_SEMANTIC_DUPLICATE_FUNCTION);
    delete root;
}

TEST(SemanticAstWalker, AutoPropertyDuplicateDefinition) {
    std::string input(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    int Y { concealed get; concealed set; }"
        "    int Y { public get; public set; }"
        "}");
    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_STATUS(semantic_walker.Walk(), STATUS_SEMANTIC_DUPLICATE_PROPERTY);
    delete root;
}

TEST(SemanticAstWalker, PropertyWithBodyDuplicateDefinition) {
    std::string input(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    int Y { concealed get { } concealed set { } }"
        "    int Y { public get { } public set { } }"
        "}");
    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);
    
    EXPECT_STATUS(semantic_walker.Walk(), STATUS_SEMANTIC_DUPLICATE_PROPERTY);
    delete root;
}

TEST(SemanticAstWalker, MixedAutoAndExpandedProperties) {
    // Case 1:
    {
        std::string input(
            "package \"Gundersoft\";"
            "public spec Test {"
            "    int Y { concealed get { } concealed set; }"
            "}");
        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        Node* root = parser.Parse();

        SemanticAstWalker semantic_walker(*root);

        EXPECT_NO_THROW(semantic_walker.Walk());
        delete root;
    }

    // Case 2:
    {
        std::string input(
            "package \"Gundersoft\";"
            "public spec Test {"
            "    int Y { concealed get; concealed set { } }"
            "}");
        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        Node* root = parser.Parse();

        SemanticAstWalker semantic_walker(*root);

        EXPECT_NO_THROW(semantic_walker.Walk());
        delete root;
    }
}

TEST(SemanticAstWalker, FunctionCallWithInvalidParameter) {
    std::string input(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    public int main(string x) { main(3); }"
        "}");
    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_STATUS(semantic_walker.Walk(), STATUS_SEMANTIC_FUNCTION_OVERLOAD_NOT_FOUND);
    delete root;
}

TEST(SemanticAstWalker, FunctionCallNeedingTypecastedParameter) {
    // Gunderscript does not allow autoboxing from int to float.
    std::string input(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    public int main(float x) { main(3); }"
        "}");
    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_STATUS(semantic_walker.Walk(), STATUS_SEMANTIC_FUNCTION_OVERLOAD_NOT_FOUND);
    delete root;
}

TEST(SemanticAstWalker, FunctionOverloads) {
    std::string input(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    public int main(int x) { }"
        "    public int main() { }"
        "}");
    CompilerStringSource source(input);
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
        std::string input(
            "package \"Gundersoft\";"
            "public spec Test {"
            "    public int main(int x) { }"
            "    public float main(int x) { }"
            "}");
        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        Node* root = parser.Parse();

        SemanticAstWalker semantic_walker(*root);

        EXPECT_STATUS(semantic_walker.Walk(), STATUS_SEMANTIC_DUPLICATE_FUNCTION);
        delete root;
    }

    // Case 2: different same params AND return type.
    {
        std::string input(
            "package \"Gundersoft\";"
            "public spec Test {"
            "    public int main(int x) { }"
            "    public int main(int x) { }"
            "}");
        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        Node* root = parser.Parse();

        SemanticAstWalker semantic_walker(*root);

        EXPECT_STATUS(semantic_walker.Walk(), STATUS_SEMANTIC_DUPLICATE_FUNCTION);
        delete root;
    }
}

TEST(SemanticAstWalker, FunctionAndPropertyAndVariableShareName) {
    // Check to make sure there are no collisions between symbols of different types.
    std::string input(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    public int X() {"
        "        X <- 3;"
        "    }"
        "    int X { public get; public set; }"
        "}");
    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_NO_THROW(semantic_walker.Walk());
    delete root;
}

TEST(SemanticAstWalker, FunctionParamTypeSymbols) {
    // Check to make sure that the Function Param symbols are being defined and are usable.
    std::string input(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    public int X(int X, int Y) {"
        "        X <- X + Y;"
        "    }"
        "}");
    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_NO_THROW(semantic_walker.Walk());
    delete root;
}

TEST(SemanticAstWalker, FunctionParamSymbolTypeReassign) {
    // Function params are declared in different scope than function variables and therefore
    // can be masked via reassignment. Masked variables take different types.
    std::string input(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    public int X(int X, int Y) {"
        "        X <- 3.0;"
        "    }"
        "}");
    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_NO_THROW(semantic_walker.Walk());
    delete root;
}

TEST(SemanticAstWalker, AttemptCrossTypeOperations) {
    // Cannot evaluate operations across types without explicit typecast.
    std::string input(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    public int X(int X, int Y) {"
        "        X <- 3.0;"
        "        Y <- X + 1;"
        "    }"
        "}");
    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_STATUS(semantic_walker.Walk(), STATUS_SEMANTIC_UNMATCHING_TYPE_IN_ADD);
    delete root;
}

TEST(SemanticAstWalker, AttemptTypeReassignment) {
    // Once a variable is declared, its type is final and cannot be changed until it goes out of scope.
    std::string input(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    public int X(int X, int Y) {"
        "        X <- 3.0;"
        "        Y <- X;"
        "        Y <- 1;"
        "    }"
        "}");
    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_STATUS(semantic_walker.Walk(), STATUS_SEMANTIC_TYPE_MISMATCH_IN_ASSIGN);
    delete root;
}

TEST(SemanticAstWalker, FunctionsOutOfOrder) {
    // Check to make sure that functions can call one another out of order.
    // This tests the prescan.
    std::string input(
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
        "}");
    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_NO_THROW(semantic_walker.Walk());
    delete root;
}

TEST(SemanticAstWalker, FunctionCorrectReturnStatement) {
    // Check for no exception when function returns correctly.
    std::string input(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    public int X() {"
        "        return 3 + 4;"
        "    }"
        "}");
    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_NO_THROW(semantic_walker.Walk());
    delete root;
}

TEST(SemanticAstWalker, FunctionIncorrectReturnStatement) {
    // Check for exception when function returns incorrect type.
    std::string input(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    public int X() {"
        "        return true;"
        "    }"
        "}");
    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_STATUS(semantic_walker.Walk(), STATUS_SEMANTIC_RETURN_TYPE_MISMATCH);
    delete root;
}

TEST(SemanticAstWalker, BlockStatementScoping) {
    // Check that variables are scoped by block without being replaced.
    std::string input(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    public int X() { "
        "        X <- 1;"
        "        { X <- true; } "
        "        X <- 2;"
        "    }"
        "}");
    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_NO_THROW(semantic_walker.Walk());
    delete root;
}

TEST(SemanticAstWalker, PropertyReturnCorrectly) {
    // Check that returning correctly from a property does not throw.
    std::string input(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    int X {"
        "        public get { return 0; }"
        "        public set { }"
        "    }"
        "}");
    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_NO_THROW(semantic_walker.Walk());
    delete root;
}

TEST(SemanticAstWalker, PropertyReturnFromSet) {
    // Check that returning from a setter does throw.
    std::string input(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    int X {"
        "        public get { return 0; }"
        "        public set { return 0; }"
        "    }"
        "}");
    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_STATUS(semantic_walker.Walk(), STATUS_SEMANTIC_RETURN_FROM_PROPERTY_SET);
    delete root;
}

TEST(SemanticAstWalker, PropertyReturnInvalidType) {
    // Check that we throw if we try to return invalid type from a property.
    std::string input(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    int X {"
        "        public get { return true; }"
        "        public set { }"
        "    }"
        "}");
    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_STATUS(semantic_walker.Walk(), STATUS_SEMANTIC_RETURN_TYPE_MISMATCH);
    delete root;
}

TEST(SemanticAstWalker, AddInvalidType) {
    std::string input(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    public int X(int x) {"
        "        main(3 + 3.0);"
        "    }"
        "}");
    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_STATUS(semantic_walker.Walk(), STATUS_SEMANTIC_UNMATCHING_TYPE_IN_ADD);
    delete root;
}

TEST(SemanticAstWalker, AddString) {
    std::string input(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    public int X(string x) {"
        "        X(\"sfsf\" + \"sfsf\");"
        "    }"
        "}");
    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_NO_THROW(semantic_walker.Walk());
    delete root;
}

TEST(SemanticAstWalker, AddBool) {
    std::string input(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    public int X(bool x) {"
        "        x <- true + true;"
        "    }"
        "}");
    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_STATUS(semantic_walker.Walk(), STATUS_SEMANTIC_INVALID_TYPE_IN_ADD);
    delete root;
}

TEST(SemanticAstWalker, SubtractString) {
    std::string input(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    public int X(string x) {"
        "        x <- \"sfsf\" - \"sfsf\";"
        "    }"
        "}");
    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_STATUS(semantic_walker.Walk(), STATUS_SEMANTIC_NONNUMERIC_OPERANDS);
    delete root;
}

TEST(SemanticAstWalker, ModString) {
    std::string input(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    public int X(string x) {"
        "        x <- \"sfsf\" % \"sfsf\";"
        "    }"
        "}");
    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_STATUS(semantic_walker.Walk(), STATUS_SEMANTIC_NONNUMERIC_OPERANDS);
    delete root;
}

TEST(SemanticAstWalker, MulInvalidType) {
    std::string input(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    public int X(bool x) {"
        "        x <- true * true;"
        "    }"
        "}");
    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_STATUS(semantic_walker.Walk(), STATUS_SEMANTIC_NONNUMERIC_OPERANDS);
    delete root;
}

TEST(SemanticAstWalker, DivInvalidType) {
    std::string input(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    public int X(bool x) {"
        "        x <- true / true;"
        "    }"
        "}");
    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_STATUS(semantic_walker.Walk(), STATUS_SEMANTIC_NONNUMERIC_OPERANDS);
    delete root;
}

TEST(SemanticAstWalker, AndInvalidType) {
    std::string input(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    public int X(bool x) {"
        "        x <- 3 && 3;"
        "    }"
        "}");
    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_STATUS(semantic_walker.Walk(), STATUS_SEMANTIC_NONBOOL_OPERANDS);
    delete root;
}

TEST(SemanticAstWalker, OrInvalidType) {
    std::string input(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    public int X(bool x) {"
        "        x <- 3.0 || 3.0;"
        "    }"
        "}");
    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_STATUS(semantic_walker.Walk(), STATUS_SEMANTIC_NONBOOL_OPERANDS);
    delete root;
}

TEST(SemanticAstWalker, GreaterInvalidType) {
    std::string input(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    public int X(bool x) {"
        "        x <- true > false;"
        "    }"
        "}");
    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_STATUS(semantic_walker.Walk(), STATUS_SEMANTIC_NONNUMERIC_OPERANDS);
    delete root;
}

TEST(SemanticAstWalker, LessInvalidType) {
    std::string input(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    public int X(bool x) {"
        "        x <- true < false;"
        "    }"
        "}");
    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_STATUS(semantic_walker.Walk(), STATUS_SEMANTIC_NONNUMERIC_OPERANDS);
    delete root;
}

TEST(SemanticAstWalker, GreaterEqualsInvalidType) {
    std::string input(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    public int X(string x) {"
        "        x <- \"h\" >= \"d\";"
        "    }"
        "}");
    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_STATUS(semantic_walker.Walk(), STATUS_SEMANTIC_NONNUMERIC_OPERANDS);
    delete root;
}

TEST(SemanticAstWalker, LessEqualsInvalidType) {
    std::string input(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    public int X(string x) {"
        "        x <- \"h\" <= \"d\";"
        "    }"
        "}");
    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_STATUS(semantic_walker.Walk(), STATUS_SEMANTIC_NONNUMERIC_OPERANDS);
    delete root;
}

TEST(SemanticAstWalker, Combined) {
    std::string input(
        "package \"Gundersoft\";"
        "public spec Test {"
        "    public int X(bool x) {"
        "        X( ((-(-1+2) / 3)) > (0 * (4 % -5)) || !(!(3 < 2)) && (4 >= 5) && (1 <= 6));"
        "    }"
        "}");
    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();

    SemanticAstWalker semantic_walker(*root);

    EXPECT_NO_THROW(semantic_walker.Walk());
    delete root;
}
