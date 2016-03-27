// Gunderscript 2 Parser Tests
// (C) 2014-2016 Christian Gunderman
// Technically more "functional" than unit.
#include "gtest/gtest.h"
#include "testing_macros.h"

#include "lexer.h"
#include "parser.h"

namespace gunderscript {
namespace compiler {

TEST(Parser, Empty) {
    std::string input("");

    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    EXPECT_STATUS(parser.Parse(), STATUS_PARSER_EOF);
}

TEST(Parser, PackageOnly) {
    // Default success case with package only.
    std::string input("package \"FooPackage\";");

    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();
    EXPECT_EQ(NodeRule::MODULE, root->rule());
    ASSERT_EQ(4, root->child_count());

    Node* name_node = root->child(0);
    EXPECT_EQ(NodeRule::NAME, name_node->rule());
    EXPECT_STREQ("FooPackage", name_node->string_value()->c_str());

    Node* depends_node = root->child(1);
    EXPECT_EQ(NodeRule::DEPENDS, depends_node->rule());
    EXPECT_EQ(0, depends_node->child_count());

    Node* specs_node = root->child(2);
    EXPECT_EQ(NodeRule::SPECS, specs_node->rule());
    EXPECT_EQ(0, specs_node->child_count());

    delete root;
}

TEST(Parser, MalformedPackage) {

    // Case 1: partial package keyword.
    {
        std::string input("packa");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MISSING_PACKAGE);
    }

    // Case 2: incorrect package name type.
    {
        std::string input("package 34");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_INVALID_PACKAGE);
    }

    // Case 3: missing semicolon.
    {
        std::string input("package \"package_name\" depends \"foo\";");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        // TODO: FIX: This test causes a memory leak in lexer.cc
        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_EXPECTED_SEMICOLON);
    }
}

TEST(Parser, MalformedDepends) {

    // Case 1: partial depends.
    {
        std::string input("package \"food\"; depends");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_EOF);
    }

    // Case 2: incorrect depends type.
    {
        std::string input("package \"food\"; depends 34;");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_DEPENDS);
    }

    // Case 3: missing semicolon.
    {
        std::string input("package \"Foo\"; depends \"Foo2Package\"");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_EOF);
    }
}

TEST(Parser, PackageDependsOnly) {

    // Case 1: partial depends.
    std::string input("package \"Foo\"; depends \"Foo2\"; depends \"Foo3\";");

    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();
    EXPECT_EQ(NodeRule::MODULE, root->rule());
    ASSERT_EQ(4, root->child_count());

    Node* name_node = root->child(0);
    EXPECT_EQ(NodeRule::NAME, name_node->rule());
    EXPECT_STREQ("Foo", name_node->string_value()->c_str());

    Node* depends_node = root->child(1);
    EXPECT_EQ(NodeRule::DEPENDS, depends_node->rule());
    EXPECT_EQ(2, depends_node->child_count());

    Node* specs_node = root->child(2);
    EXPECT_EQ(NodeRule::SPECS, specs_node->rule());
    EXPECT_EQ(0, specs_node->child_count());

    Node* dependency_node_0 = depends_node->child(0);
    EXPECT_EQ(NodeRule::NAME, dependency_node_0->rule());
    EXPECT_EQ(0, dependency_node_0->child_count());
    EXPECT_STREQ("Foo2", dependency_node_0->string_value()->c_str());

    Node* dependency_node_1 = depends_node->child(1);
    EXPECT_EQ(NodeRule::NAME, dependency_node_1->rule());
    EXPECT_EQ(0, dependency_node_1->child_count());
    EXPECT_STREQ("Foo3", dependency_node_1->string_value()->c_str());

    delete root;
}

// In response to Github issue #64 to prevent regression.
TEST(Parser, SingleTokenIncompleteDecl) {
    std::string input("package \"FooPackage\"; public");

    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    EXPECT_STATUS(parser.Parse(), STATUS_PARSER_EOF);
}

TEST(Parser, MalformedSpec) {

    // Case 1: missing access modifier.
    {
        std::string input("package \"FooPackage\"; spec MySpec { }");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        // TODO: Fix: This test causes a memory leak in lexer.cc
        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_SPEC_OR_FUNC_ACCESS_MODIFIER_MISSING);
    }

    // Case 2: missing spec keyword.
    {
        std::string input("package \"FooPackage\"; concealed MySpec { }");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_SPEC_SPEC_KEYWORD_MISSING);
    }

    // Case 4: incorrect spec name format.
    {
        std::string input("package \"FooPackage\"; concealed spec \"MySpec\" { }");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_SPEC_NAME_MISSING);
    }

    // Case 4: missing opening brace.
    {
        std::string input("package \"FooPackage\"; concealed spec MySpec  }");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_SPEC_LBRACE_MISSING);
    }

    // Case 5: missing closing brace.
    {
        std::string input("package \"FooPackage\"; concealed spec MySpec { ");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_EOF);
    }
}

TEST(Parser, EmptySpec) {

    std::string input("package \"FooPackage\";"
        "public spec MySpec { }"
        "concealed spec Foo { }");

    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();
    EXPECT_EQ(NodeRule::MODULE, root->rule());
    ASSERT_EQ(4, root->child_count());

    Node* name_node = root->child(0);
    EXPECT_EQ(NodeRule::NAME, name_node->rule());
    EXPECT_STREQ("FooPackage", name_node->string_value()->c_str());

    Node* depends_node = root->child(1);
    EXPECT_EQ(NodeRule::DEPENDS, depends_node->rule());
    EXPECT_EQ(0, depends_node->child_count());

    Node* specs_node = root->child(2);
    EXPECT_EQ(NodeRule::SPECS, specs_node->rule());
    EXPECT_EQ(2, specs_node->child_count());

    Node* spec_node_0 = specs_node->child(0);
    EXPECT_EQ(NodeRule::SPEC, spec_node_0->rule());
    EXPECT_EQ(4, spec_node_0->child_count());

    Node* spec_node_1 = specs_node->child(1);
    EXPECT_EQ(NodeRule::SPEC, spec_node_1->rule());
    EXPECT_EQ(4, spec_node_1->child_count());

    Node* spec_node_0_access_modifier = spec_node_0->child(0);
    EXPECT_EQ(NodeRule::ACCESS_MODIFIER, spec_node_0_access_modifier->rule());
    EXPECT_EQ(0, spec_node_0_access_modifier->child_count());
    EXPECT_EQ(LexerSymbol::PUBLIC, spec_node_0_access_modifier->symbol_value());

    Node* spec_node_0_name = spec_node_0->child(1);
    EXPECT_EQ(NodeRule::TYPE, spec_node_0_name->rule());
    EXPECT_EQ(0, spec_node_0_name->child_count());
    EXPECT_STREQ("MySpec", spec_node_0_name->string_value()->c_str());

    Node* spec_node_0_functions = spec_node_0->child(2);
    EXPECT_EQ(NodeRule::FUNCTIONS, spec_node_0_functions->rule());
    EXPECT_EQ(0, spec_node_0_functions->child_count());

    Node* spec_node_0_properties = spec_node_0->child(3);
    EXPECT_EQ(NodeRule::PROPERTIES, spec_node_0_properties->rule());
    EXPECT_EQ(0, spec_node_0_properties->child_count());

    Node* spec_node_1_access_modifier = spec_node_1->child(0);
    EXPECT_EQ(NodeRule::ACCESS_MODIFIER, spec_node_1_access_modifier->rule());
    EXPECT_EQ(0, spec_node_1_access_modifier->child_count());
    EXPECT_EQ(LexerSymbol::CONCEALED, spec_node_1_access_modifier->symbol_value());

    Node* spec_node_1_name = spec_node_1->child(1);
    EXPECT_EQ(NodeRule::TYPE, spec_node_1_name->rule());
    EXPECT_EQ(0, spec_node_1_name->child_count());
    EXPECT_STREQ("Foo", spec_node_1_name->string_value()->c_str());

    Node* spec_node_1_functions = spec_node_1->child(2);
    EXPECT_EQ(NodeRule::FUNCTIONS, spec_node_1_functions->rule());
    EXPECT_EQ(0, spec_node_1_functions->child_count());

    Node* spec_node_1_properties = spec_node_1->child(3);
    EXPECT_EQ(NodeRule::PROPERTIES, spec_node_1_properties->rule());
    EXPECT_EQ(0, spec_node_1_properties->child_count());

    delete root;
}

TEST(Parser, ParseMalformedProperty) {

    // Case 1: leading access modifier.
    {
        std::string input("package \"FooPackage\";"
            "public spec MySpec { "
            "  concealed float X { public get; concealed set; }"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_FUNCTION_MISSING_LPAREN);
    }

    // Case 2: incorrect symbol type property name.
    {
        std::string input("package \"FooPackage\";"
            "public spec MySpec { "
            "  float 34 { public get; concealed set; }"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_PROPERTY_NAME_MISSING);
    }

    // Case 3: missing opening brace.
    {
        std::string input("package \"FooPackage\";"
            "public spec MySpec { "
            "  float X  public get; concealed set; }"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_PROPERTY_LBRACE_MISSING);
    }

    // Case 4: missing closing brace.
    {
        std::string input("package \"FooPackage\";"
            "public spec MySpec { "
            "  float X { public get; concealed set; "
            "  float Y { }"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_PROPERTY_RBRACE_MISSING);
    }

    // Case 5: missing property accessor/mutator access modifier.
    {
        std::string input("package \"FooPackage\";"
            "public spec MySpec { "
            "  float X { get; concealed set; }"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_PROPERTYFUNCTION_MISSING_ACCESS_MODIFIER);
    }

    // Case 6: multiple get/set.
    {
        std::string input("package \"FooPackage\";"
            "public spec MySpec { "
            "  float X { public set; concealed set; }"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_PROPERTYFUNCTION_DUPLICATE);
    }

    // Case 7: semicolon following property accessor body
    {
        std::string input("package \"FooPackage\";"
            "public spec MySpec { "
            "  float X { public get { }; concealed set; }"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        // TODO: better exception, this is confusing.
        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_PROPERTYFUNCTION_MISSING_ACCESS_MODIFIER);
    }

    // Case 8: missing accessor/mutator.
    {
        std::string input("package \"FooPackage\";"
            "public spec MySpec { "
            "  float X { public get; }"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_PROPERTY_MISSING_PROPERTY_FUNCTION);
    }
}

TEST(Parser, ParsePropertyEmpty) {

    std::string input("package \"FooPackage\";"
        "public spec MySpec {"
        "  int X { }"
        "  string Y { }"
        "}");

    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_PROPERTY_MISSING_PROPERTY_FUNCTION);
}

TEST(Parser, ParsePropertyAuto) {

    // Case 1: get first, set second.
    {
        std::string input("package \"FooPackage\";"
            "public spec MySpec { "
            "  float X { public get; concealed set; }"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        Node* root = parser.Parse();
        ASSERT_EQ(4, root->child_count());

        Node* specs_node = root->child(2);
        ASSERT_EQ(1, specs_node->child_count());

        Node* spec_node = specs_node->child(0);
        ASSERT_EQ(4, spec_node->child_count());

        Node* properties_node = spec_node->child(3);
        ASSERT_EQ(1, properties_node->child_count());

        Node* x_node = properties_node->child(0);
        EXPECT_EQ(NodeRule::PROPERTY, x_node->rule());
        ASSERT_EQ(4, x_node->child_count());

        Node* x_type_node = x_node->child(0);
        EXPECT_EQ(NodeRule::TYPE, x_type_node->rule());
        EXPECT_STREQ("float", x_type_node->string_value()->c_str());

        Node* x_name_node = x_node->child(1);
        EXPECT_EQ(NodeRule::NAME, x_name_node->rule());
        EXPECT_STREQ("X", x_name_node->string_value()->c_str());

        Node*  x_getter_node = x_node->child(2);
        EXPECT_EQ(NodeRule::PROPERTY_FUNCTION, x_getter_node->rule());
        ASSERT_EQ(1, x_getter_node->child_count());

        Node* x_getter_access_modifier_node = x_getter_node->child(0);
        EXPECT_EQ(NodeRule::ACCESS_MODIFIER, x_getter_access_modifier_node->rule());
        EXPECT_EQ(LexerSymbol::PUBLIC, x_getter_access_modifier_node->symbol_value());

        Node* x_setter_node = x_node->child(3);
        EXPECT_EQ(NodeRule::PROPERTY_FUNCTION, x_setter_node->rule());
        ASSERT_EQ(1, x_setter_node->child_count());

        Node* x_setter_access_modifier_node = x_setter_node->child(0);
        EXPECT_EQ(NodeRule::ACCESS_MODIFIER, x_setter_access_modifier_node->rule());
        EXPECT_EQ(LexerSymbol::CONCEALED, x_setter_access_modifier_node->symbol_value());

        delete root;
    }

    // Case 2: get second, set first.
    {
        std::string input("package \"FooPackage\";"
            "public spec MySpec { "
            "  float X { concealed set; internal get; }"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        Node* root = parser.Parse();
        Node* specs_node = root->child(2);
        Node* spec_node = specs_node->child(0);
        Node* properties_node = spec_node->child(3);
        Node* x_node = properties_node->child(0);

        Node*  x_getter_node = x_node->child(2);
        Node* x_getter_access_modifier_node = x_getter_node->child(0);
        EXPECT_EQ(NodeRule::ACCESS_MODIFIER, x_getter_access_modifier_node->rule());
        EXPECT_EQ(LexerSymbol::INTERNAL, x_getter_access_modifier_node->symbol_value());

        Node* x_setter_node = x_node->child(3);
        Node* x_setter_access_modifier_node = x_setter_node->child(0);
        EXPECT_EQ(NodeRule::ACCESS_MODIFIER, x_setter_access_modifier_node->rule());
        EXPECT_EQ(LexerSymbol::CONCEALED, x_setter_access_modifier_node->symbol_value());

        delete root;
    }
}

TEST(Parser, ParsePropertyWithFunctionBody) {
    std::string input(
        "package \"FooPackage\";"
        "public spec MySpec {"
        "  int Foo {"
        "    public get { return 3; }"
        "    public set;"
        "  }"
        "}");

    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();
    Node* specs_node = root->child(2);
    Node* spec_node = specs_node->child(0);
    Node* properties_node = spec_node->child(3);
    Node* foo_node = properties_node->child(0);
    EXPECT_EQ(NodeRule::PROPERTY, foo_node->rule());
    ASSERT_EQ(4, foo_node->child_count());

    Node* foo_type_node = foo_node->child(0);
    Node* foo_name_node = foo_node->child(1);
    Node* foo_getter_node = foo_node->child(2);
    Node* foo_setter_node = foo_node->child(3);

    EXPECT_EQ(NodeRule::TYPE, foo_type_node->rule());
    ASSERT_STREQ("int", foo_type_node->string_value()->c_str());

    EXPECT_EQ(NodeRule::NAME, foo_name_node->rule());
    ASSERT_STREQ("Foo", foo_name_node->string_value()->c_str());

    EXPECT_EQ(2, foo_getter_node->child_count());

    Node* access_modifier_node = foo_getter_node->child(0);

    EXPECT_EQ(NodeRule::ACCESS_MODIFIER, access_modifier_node->rule());
    ASSERT_EQ(LexerSymbol::PUBLIC, access_modifier_node->symbol_value());

    Node* block_node = foo_getter_node->child(1);

    EXPECT_EQ(NodeRule::BLOCK, block_node->rule());
    ASSERT_EQ(1, block_node->child_count());

    Node* return_node = block_node->child(0);

    EXPECT_EQ(1, return_node->child_count());

    Node* expression_node = return_node->child(0);

    EXPECT_EQ(NodeRule::EXPRESSION, expression_node->rule());
    ASSERT_EQ(1, expression_node->child_count());

    Node* value_node = expression_node->child(0);
    EXPECT_EQ(NodeRule::INT, value_node->rule());
    ASSERT_EQ(3, value_node->int_value());

    delete root;
}

TEST(Parser, ParseFunctionEmpty) {

    std::string input("package \"FooPackage\";"
        "public spec MySpec {"
        "  public int Foo() {"
        "  }"
        "}");

    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();
    Node* specs_node = root->child(2);
    Node* spec_node = specs_node->child(0);
    Node* functions_node = spec_node->child(2);
    Node* foo_node = functions_node->child(0);
    EXPECT_EQ(NodeRule::FUNCTION, foo_node->rule());
    ASSERT_EQ(5, foo_node->child_count());

    Node* foo_access_modifier_node = foo_node->child(0);
    EXPECT_EQ(NodeRule::ACCESS_MODIFIER, foo_access_modifier_node->rule());
    EXPECT_EQ(LexerSymbol::PUBLIC, foo_access_modifier_node->symbol_value());

    Node* foo_type_node = foo_node->child(1);
    EXPECT_EQ(NodeRule::TYPE, foo_type_node->rule());
    EXPECT_STREQ("int", foo_type_node->string_value()->c_str());

    Node* foo_name_node = foo_node->child(2);
    EXPECT_EQ(NodeRule::NAME, foo_name_node->rule());
    EXPECT_STREQ("Foo", foo_name_node->string_value()->c_str());

    Node* foo_parameters_node = foo_node->child(3);
    EXPECT_EQ(NodeRule::FUNCTION_PARAMETERS, foo_parameters_node->rule());
    EXPECT_EQ(0, foo_parameters_node->child_count());

    Node* foo_block_node = foo_node->child(4);
    EXPECT_EQ(0, foo_block_node->child_count());

    delete root;
}

TEST(Parser, ParseConstructorEmpty) {

    std::string input("package \"FooPackage\";"
        "public spec MySpec {"
        "  public construct() {"
        "  }"
        "}");

    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();
    Node* specs_node = root->child(2);
    Node* spec_node = specs_node->child(0);
    Node* functions_node = spec_node->child(2);
    Node* foo_node = functions_node->child(0);
    EXPECT_EQ(NodeRule::FUNCTION, foo_node->rule());
    ASSERT_EQ(5, foo_node->child_count());

    Node* foo_access_modifier_node = foo_node->child(0);
    EXPECT_EQ(NodeRule::ACCESS_MODIFIER, foo_access_modifier_node->rule());
    EXPECT_EQ(LexerSymbol::PUBLIC, foo_access_modifier_node->symbol_value());

    Node* foo_type_node = foo_node->child(1);
    EXPECT_EQ(NodeRule::TYPE, foo_type_node->rule());
    EXPECT_STREQ("void", foo_type_node->string_value()->c_str());

    Node* foo_name_node = foo_node->child(2);
    EXPECT_EQ(NodeRule::NAME, foo_name_node->rule());
    EXPECT_STREQ("%construct%", foo_name_node->string_value()->c_str());

    Node* foo_parameters_node = foo_node->child(3);
    EXPECT_EQ(NodeRule::FUNCTION_PARAMETERS, foo_parameters_node->rule());
    EXPECT_EQ(0, foo_parameters_node->child_count());

    Node* foo_block_node = foo_node->child(4);
    EXPECT_EQ(0, foo_block_node->child_count());

    delete root;
}

TEST(Parser, ParseMultipleFunctions) {

    std::string input("package \"FooPackage\";"
        "public spec MySpec {"
        "  concealed string Foo2(int x, string y) { }"
        "  public int Add() { }"
        "  internal int Sub(string foo, int foo2) {"
        "  }  "
        "  public int Mul() {"
        "  }"
        "  string FooStringProperty { public get; public set; }"
        "}");

    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();
    delete root;
}

TEST(Parser, ParseMalformedFunctions) {

    // Case 1: missing access modifier.
    {
        std::string input("package \"FooPackage\";"
            "public spec MySpec {"
            "  string Foo2(int x, string y) { }"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_PROPERTY_LBRACE_MISSING);
    }

    // Case 2: incorrect token type for NAME.
    {
        std::string input("package \"FooPackage\";"
            "public spec MySpec {"
            "  public string 45(int x, string y) { }"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_FUNCTION_MISSING_NAME);
    }

    // Case 3: extraneous comma.
    {
        std::string input("package \"FooPackage\";"
            "public spec MySpec {"
            "  public string Foo(int x,) { }"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_FUNCTIONPARAMS_MISSING_TYPE);
    }

    // Case 4: incorrect argument.
    {
        std::string input("package \"FooPackage\";"
            "public spec MySpec {"
            "  public string Foo(public int x) { }"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_FUNCTIONPARAMS_MISSING_TYPE);
    }

    // Case 5: missing comma.
    {
        std::string input("package \"FooPackage\";"
            "public spec MySpec {"
            "  public string Foo(int x int y) { }"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_FUNCTIONPARAMS_MISSING_COMMA);
    }

    // Case 6: missing LPAREN.
    {
        std::string input("package \"FooPackage\";"
            "public spec MySpec {"
            "  public string Foo int x) { }"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_FUNCTION_MISSING_LPAREN);
    }

    // Case 7: missing RPAREN.
    {
        std::string input("package \"FooPackage\";"
            "public spec MySpec {"
            "  public string Foo(int x { }"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_FUNCTIONPARAMS_MISSING_COMMA);
    }

    // Case 8: missing brace.
    {
        std::string input("package \"FooPackage\";"
            "public spec MySpec {"
            "  public string Foo(int x) }"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_BLOCK_MISSING_LBRACE);
    }

    // Case 9: constructor outside of spec.
    {
        std::string input("package \"FooPackage\";"
            "public construct() { }");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_CONSTRUCTOR_OUTSIDE_SPEC);
    }
}

TEST(Parser, ParseEmptyReturn) {

    std::string input("package \"FooPackage\";"
        "public spec MySpec {"
        "  public string Foo() {"
        "    return;"
        "  }"
        "}");

    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();
    Node* specs_node = root->child(2);
    Node* spec_node = specs_node->child(0);
    Node* functions_node = spec_node->child(2);
    Node* foo_node = functions_node->child(0);

    Node* foo_block_node = foo_node->child(4);
    EXPECT_EQ(1, foo_block_node->child_count());

    Node* foo_return_node = foo_block_node->child(0);
    EXPECT_EQ(NodeRule::RETURN, foo_return_node->rule());
    EXPECT_EQ(0, foo_return_node->child_count());

    delete root;
}


TEST(Parser, ParseReturnWithExpression) {

    std::string input("package \"FooPackage\";"
        "public spec MySpec {"
        "  public string Foo() {"
        "    return 15;"
        "  }"
        "}");

    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();
    Node* specs_node = root->child(2);
    Node* spec_node = specs_node->child(0);
    Node* functions_node = spec_node->child(2);
    Node* foo_node = functions_node->child(0);

    Node* foo_block_node = foo_node->child(4);
    EXPECT_EQ(1, foo_block_node->child_count());

    Node* foo_return_node = foo_block_node->child(0);
    EXPECT_EQ(NodeRule::RETURN, foo_return_node->rule());
    ASSERT_EQ(1, foo_return_node->child_count());

    Node* foo_return_expression_node = foo_return_node->child(0);
    EXPECT_EQ(NodeRule::EXPRESSION, foo_return_expression_node->rule());
    EXPECT_EQ(1, foo_return_expression_node->child_count());

    Node* foo_int_node = foo_return_expression_node->child(0);
    EXPECT_EQ(NodeRule::INT, foo_int_node->rule());
    EXPECT_EQ(0, foo_int_node->child_count());
    EXPECT_EQ(15, foo_int_node->int_value());

    delete root;
}

TEST(Parser, ParseMalformedReturn) {

    // Case 1: expression but missing semicolon.
    {
        std::string input("package \"FooPackage\";"
            "public spec MySpec {"
            "  public string Foo() {"
            "    return 15"
            "  }"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_EXPECTED_SEMICOLON);
    }

    // Case 2: no expression but missing semicolon.
    {
        std::string input("package \"FooPackage\";"
            "public spec MySpec {"
            "  public string Foo() {"
            "    return "
            "  }"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_EXPRESSION_INVALID_TOKEN);
    }
}

TEST(Parser, ParseArithmeticExpression) {

    std::string input("package \"FooPackage\";"
        "public spec MySpec {"
        "  public string Foo() {"
        "    return (((15.55 + 5.0) * 3.0) - -1.0 % 2.0) / 'c';"
        "  }"
        "}");

    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();
    Node* specs_node = root->child(2);
    Node* spec_node = specs_node->child(0);
    Node* functions_node = spec_node->child(2);
    Node* foo_node = functions_node->child(0);

    Node* foo_block_node = foo_node->child(4);
    EXPECT_EQ(1, foo_block_node->child_count());

    Node* foo_return_node = foo_block_node->child(0);
    EXPECT_EQ(NodeRule::RETURN, foo_return_node->rule());
    ASSERT_EQ(1, foo_return_node->child_count());

    Node* foo_return_expression_node = foo_return_node->child(0);
    EXPECT_EQ(NodeRule::EXPRESSION, foo_return_expression_node->rule());
    EXPECT_EQ(1, foo_return_expression_node->child_count());

    Node* foo_expression_root_node = foo_return_expression_node->child(0);
    EXPECT_EQ(NodeRule::DIV, foo_expression_root_node->rule());
    EXPECT_EQ(2, foo_expression_root_node->child_count());

    Node* foo_numerator_node = foo_expression_root_node->child(0);
    EXPECT_EQ(NodeRule::SUB, foo_numerator_node->rule());
    EXPECT_EQ(2, foo_numerator_node->child_count());

    Node* foo_left_multiply_node = foo_numerator_node->child(0);
    EXPECT_EQ(NodeRule::MUL, foo_left_multiply_node->rule());
    EXPECT_EQ(2, foo_left_multiply_node->child_count());

    Node* foo_add_node = foo_left_multiply_node->child(0);
    EXPECT_EQ(NodeRule::ADD, foo_add_node->rule());
    EXPECT_EQ(2, foo_add_node->child_count());

    Node* foo_add_left_operand_node = foo_add_node->child(0);
    EXPECT_EQ(NodeRule::FLOAT, foo_add_left_operand_node->rule());
    EXPECT_EQ(15.55, foo_add_left_operand_node->float_value());

    Node* foo_add_right_operand_node = foo_add_node->child(1);
    EXPECT_EQ(NodeRule::FLOAT, foo_add_right_operand_node->rule());
    EXPECT_EQ(5.0, foo_add_right_operand_node->float_value());

    Node* foo_lm_right_operand_node = foo_left_multiply_node->child(1);
    EXPECT_EQ(NodeRule::FLOAT, foo_lm_right_operand_node->rule());
    EXPECT_EQ(3.0, foo_lm_right_operand_node->float_value());

    Node* foo_right_multiply_node = foo_numerator_node->child(1);
    EXPECT_EQ(NodeRule::MOD, foo_right_multiply_node->rule());
    EXPECT_EQ(2, foo_right_multiply_node->child_count());

    Node* foo_negate_node = foo_right_multiply_node->child(0);
    EXPECT_EQ(NodeRule::SUB, foo_negate_node->rule());
    EXPECT_EQ(2, foo_negate_node->child_count());

    // This node is an ANY_TYPE node indicating to the type checker that it
    // matches any type and implicitly has the default value for whichever
    // type it operates with.
    Node* foo_negate_left_operand_node = foo_negate_node->child(0);
    EXPECT_EQ(NodeRule::ANY_TYPE, foo_negate_left_operand_node->rule());
    EXPECT_EQ(0, foo_negate_left_operand_node->int_value());

    Node* foo_negate_right_operand_node = foo_negate_node->child(1);
    EXPECT_EQ(NodeRule::FLOAT, foo_negate_right_operand_node->rule());
    EXPECT_EQ(1.0, foo_negate_right_operand_node->float_value());

    Node* foo_rm_right_operand_node = foo_right_multiply_node->child(1);
    EXPECT_EQ(NodeRule::FLOAT, foo_lm_right_operand_node->rule());
    EXPECT_EQ(2.0, foo_rm_right_operand_node->float_value());

    Node* foo_denominator_node = foo_expression_root_node->child(1);
    EXPECT_EQ(NodeRule::CHAR, foo_denominator_node->rule());
    EXPECT_EQ('c', foo_denominator_node->int_value());

    delete root;
}

TEST(Parser, ParseDoubleNegative) {

    std::string input("package \"FooPackage\";"
        "public spec MySpec {"
        "  public int Foo() {"
        "    return -(-3);"
        "  }"
        "}");

    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();
    Node* specs_node = root->child(2);
    Node* spec_node = specs_node->child(0);
    Node* functions_node = spec_node->child(2);
    Node* foo_node = functions_node->child(0);

    Node* foo_block_node = foo_node->child(4);
    EXPECT_EQ(1, foo_block_node->child_count());

    Node* foo_return_node = foo_block_node->child(0);
    EXPECT_EQ(NodeRule::RETURN, foo_return_node->rule());
    ASSERT_EQ(1, foo_return_node->child_count());

    Node* foo_return_expression_node = foo_return_node->child(0);
    EXPECT_EQ(NodeRule::EXPRESSION, foo_return_expression_node->rule());
    EXPECT_EQ(1, foo_return_expression_node->child_count());

    Node* foo_expression_root_node = foo_return_expression_node->child(0);
    EXPECT_EQ(NodeRule::SUB, foo_expression_root_node->rule());
    EXPECT_EQ(2, foo_expression_root_node->child_count());

    Node* left_root_child_node = foo_expression_root_node->child(0);
    EXPECT_EQ(NodeRule::ANY_TYPE, left_root_child_node->rule());
    EXPECT_EQ(0, left_root_child_node->child_count());
    EXPECT_EQ(0, left_root_child_node->int_value());

    Node* foo_sub_2_node = foo_expression_root_node->child(1);
    EXPECT_EQ(NodeRule::SUB, foo_expression_root_node->rule());
    EXPECT_EQ(2, foo_expression_root_node->child_count());

    Node* left_sub_child_node = foo_sub_2_node->child(0);
    EXPECT_EQ(NodeRule::ANY_TYPE, left_sub_child_node->rule());
    EXPECT_EQ(0, left_sub_child_node->child_count());
    EXPECT_EQ(0, left_sub_child_node->int_value());

    Node* right_sub_child_node = foo_sub_2_node->child(1);
    EXPECT_EQ(NodeRule::INT, right_sub_child_node->rule());
    EXPECT_EQ(0, right_sub_child_node->child_count());
    EXPECT_EQ(3, right_sub_child_node->int_value());

    delete root;
}

TEST(Parser, ParseDoubleNot) {

    std::string input("package \"FooPackage\";"
        "public spec MySpec {"
        "  public bool Foo() {"
        "    return !(!true);"
        "  }"
        "}");

    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();
    Node* specs_node = root->child(2);
    Node* spec_node = specs_node->child(0);
    Node* functions_node = spec_node->child(2);
    Node* foo_node = functions_node->child(0);

    Node* foo_block_node = foo_node->child(4);
    EXPECT_EQ(1, foo_block_node->child_count());

    Node* foo_return_node = foo_block_node->child(0);
    EXPECT_EQ(NodeRule::RETURN, foo_return_node->rule());
    ASSERT_EQ(1, foo_return_node->child_count());

    Node* foo_return_expression_node = foo_return_node->child(0);
    EXPECT_EQ(NodeRule::EXPRESSION, foo_return_expression_node->rule());
    EXPECT_EQ(1, foo_return_expression_node->child_count());

    Node* foo_expression_root_node = foo_return_expression_node->child(0);
    EXPECT_EQ(NodeRule::LOGNOT, foo_expression_root_node->rule());
    EXPECT_EQ(1, foo_expression_root_node->child_count());

    Node* foo_not_2_node = foo_expression_root_node->child(0);
    EXPECT_EQ(NodeRule::LOGNOT, foo_not_2_node->rule());
    EXPECT_EQ(1, foo_not_2_node->child_count());

    Node* bool_node = foo_not_2_node->child(0);
    EXPECT_EQ(NodeRule::BOOL, bool_node->rule());
    EXPECT_EQ(0, bool_node->child_count());
    EXPECT_EQ(true, bool_node->bool_value());

    delete root;
}

TEST(Parser, ParseDoubleNegativeNoParens) {

    // This case is not TECHNICALLY supported by our intended language spec 
    // and should not be used but it seems to work with the current parser 
    // implementation and therefore we MUST have tests to make sure that we
    // don't have a weird, broken, or inconsistent AST or a parser crash if 
    // this pops up in the code.
    // Eventually I'd like to make '--' it's own operator but for now the
    // important thing is stability.
    std::string input("package \"FooPackage\";"
        "public spec MySpec {"
        "  public int Foo() {"
        "    return --3;"
        "  }"
        "}");

    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();
    Node* specs_node = root->child(2);
    Node* spec_node = specs_node->child(0);
    Node* functions_node = spec_node->child(2);
    Node* foo_node = functions_node->child(0);

    Node* foo_block_node = foo_node->child(4);
    EXPECT_EQ(1, foo_block_node->child_count());

    Node* foo_return_node = foo_block_node->child(0);
    EXPECT_EQ(NodeRule::RETURN, foo_return_node->rule());
    ASSERT_EQ(1, foo_return_node->child_count());

    Node* foo_return_expression_node = foo_return_node->child(0);
    EXPECT_EQ(NodeRule::EXPRESSION, foo_return_expression_node->rule());
    EXPECT_EQ(1, foo_return_expression_node->child_count());

    Node* foo_expression_root_node = foo_return_expression_node->child(0);
    EXPECT_EQ(NodeRule::SUB, foo_expression_root_node->rule());
    EXPECT_EQ(2, foo_expression_root_node->child_count());

    Node* left_root_child_node = foo_expression_root_node->child(0);
    EXPECT_EQ(NodeRule::ANY_TYPE, left_root_child_node->rule());
    EXPECT_EQ(0, left_root_child_node->child_count());
    EXPECT_EQ(0, left_root_child_node->int_value());

    Node* foo_sub_2_node = foo_expression_root_node->child(1);
    EXPECT_EQ(NodeRule::SUB, foo_expression_root_node->rule());
    EXPECT_EQ(2, foo_expression_root_node->child_count());

    Node* left_sub_child_node = foo_sub_2_node->child(0);
    EXPECT_EQ(NodeRule::ANY_TYPE, left_sub_child_node->rule());
    EXPECT_EQ(0, left_sub_child_node->child_count());
    EXPECT_EQ(0, left_sub_child_node->int_value());

    Node* right_sub_child_node = foo_sub_2_node->child(1);
    EXPECT_EQ(NodeRule::INT, right_sub_child_node->rule());
    EXPECT_EQ(0, right_sub_child_node->child_count());
    EXPECT_EQ(3, right_sub_child_node->int_value());

    delete root;
}

TEST(Parser, ParseDoubleNotNoParens) {

    // This case is not TECHNICALLY supported by our intended language spec 
    // and should not be used but it seems to work with the current parser 
    // implementation and therefore we MUST have tests to make sure that we
    // don't have a weird, broken, or inconsistent AST or a parser crash if 
    // this pops up in the code.
    // Eventually I'd like to make '!!' it's own operator but for now the
    // important thing is stability.
    std::string input("package \"FooPackage\";"
        "public spec MySpec {"
        "  public bool Foo() {"
        "    return !!true;"
        "  }"
        "}");

    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();
    Node* specs_node = root->child(2);
    Node* spec_node = specs_node->child(0);
    Node* functions_node = spec_node->child(2);
    Node* foo_node = functions_node->child(0);

    Node* foo_block_node = foo_node->child(4);
    EXPECT_EQ(1, foo_block_node->child_count());

    Node* foo_return_node = foo_block_node->child(0);
    EXPECT_EQ(NodeRule::RETURN, foo_return_node->rule());
    ASSERT_EQ(1, foo_return_node->child_count());

    Node* foo_return_expression_node = foo_return_node->child(0);
    EXPECT_EQ(NodeRule::EXPRESSION, foo_return_expression_node->rule());
    EXPECT_EQ(1, foo_return_expression_node->child_count());

    Node* foo_expression_root_node = foo_return_expression_node->child(0);
    EXPECT_EQ(NodeRule::LOGNOT, foo_expression_root_node->rule());
    EXPECT_EQ(1, foo_expression_root_node->child_count());

    Node* foo_not_2_node = foo_expression_root_node->child(0);
    EXPECT_EQ(NodeRule::LOGNOT, foo_not_2_node->rule());
    EXPECT_EQ(1, foo_not_2_node->child_count());

    Node* bool_node = foo_not_2_node->child(0);
    EXPECT_EQ(NodeRule::BOOL, bool_node->rule());
    EXPECT_EQ(0, bool_node->child_count());
    EXPECT_EQ(true, bool_node->bool_value());

    delete root;
}

TEST(Parser, ParseMalformedArithmeticExpression) {

    // Case 1: missing right operand add.
    {
        std::string input("package \"FooPackage\";"
            "public spec MySpec {"
            "  public string Foo() {"
            "    return 3 + ;"
            "  }"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_EXPRESSION_INVALID_TOKEN);
    }

    // Case 2: missing left operand add.
    {
        std::string input("package \"FooPackage\";"
            "public spec MySpec {"
            "  public string Foo() {"
            "    return + 3;"
            "  }"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_EXPRESSION_INVALID_TOKEN);
    }

    // Case 3: missing right operand multiply.
    {
        std::string input("package \"FooPackage\";"
            "public spec MySpec {"
            "  public string Foo() {"
            "    return 3 * ;"
            "  }"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_EXPRESSION_INVALID_TOKEN);
    }

    // Case 4: missing left operand divide.
    {
        std::string input("package \"FooPackage\";"
            "public spec MySpec {"
            "  public string Foo() {"
            "    return / 3;"
            "  }"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_EXPRESSION_INVALID_TOKEN);
    }

    // Case 5: keyword in expression
    {
        std::string input("package \"FooPackage\";"
            "public spec MySpec {"
            "  public string Foo() {"
            "    return 3 * return;"
            "  }"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_EXPRESSION_INVALID_TOKEN);
    }

    // Case 6: extraneous RPAREN.
    {
        std::string input("package \"FooPackage\";"
            "public spec MySpec {"
            "  public string Foo() {"
            "    return -3);"
            "  }"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        // This technically works fine but is a confusing error message.
        // TODO: more context relevant error message.
        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_EXPECTED_SEMICOLON);
    }

    // Case 7: Unclosed parenthesis.
    {
        std::string input("package \"FooPackage\";"
            "public spec MySpec {"
            "  public string Foo() {"
            "    return (-(3 + 34);"
            "  }"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_EXPRESSION_MISSING_RPAREN);
    }
}

TEST(Parser, ParseBooleanExpression) {

    std::string input("package \"FooPackage\";"
        "public spec MySpec {"
        "  public bool Foo() {"
        "    return (true || false) && !(true && false || false);"
        "  }"
        "}");

    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();
    Node* specs_node = root->child(2);
    Node* spec_node = specs_node->child(0);
    Node* functions_node = spec_node->child(2);
    Node* foo_node = functions_node->child(0);

    Node* foo_block_node = foo_node->child(4);
    EXPECT_EQ(1, foo_block_node->child_count());

    Node* foo_return_node = foo_block_node->child(0);
    EXPECT_EQ(NodeRule::RETURN, foo_return_node->rule());
    ASSERT_EQ(1, foo_return_node->child_count());

    Node* foo_return_expression_node = foo_return_node->child(0);
    EXPECT_EQ(NodeRule::EXPRESSION, foo_return_expression_node->rule());
    ASSERT_EQ(1, foo_return_expression_node->child_count());

    Node* foo_expression_root_node = foo_return_expression_node->child(0);
    EXPECT_EQ(NodeRule::LOGAND, foo_expression_root_node->rule());
    ASSERT_EQ(2, foo_expression_root_node->child_count());

    Node* foo_expression_left_node = foo_expression_root_node->child(0);
    EXPECT_EQ(NodeRule::LOGOR, foo_expression_left_node->rule());
    ASSERT_EQ(2, foo_expression_left_node->child_count());

    Node* foo_expression_left_left_node = foo_expression_left_node->child(0);
    EXPECT_EQ(NodeRule::BOOL, foo_expression_left_left_node->rule());
    EXPECT_EQ(true, foo_expression_left_left_node->bool_value());

    Node* foo_expression_left_right_node = foo_expression_left_node->child(1);
    EXPECT_EQ(NodeRule::BOOL, foo_expression_left_right_node->rule());
    EXPECT_FALSE(foo_expression_left_right_node->bool_value());

    Node* foo_expression_right_node = foo_expression_root_node->child(1);
    EXPECT_EQ(NodeRule::LOGNOT, foo_expression_right_node->rule());
    ASSERT_EQ(1, foo_expression_right_node->child_count());

    Node* foo_expression_right_child_node = foo_expression_right_node->child(0);
    EXPECT_EQ(NodeRule::LOGOR, foo_expression_right_child_node->rule());
    ASSERT_EQ(2, foo_expression_right_child_node->child_count());

    Node* foo_expression_right_child_left_node = foo_expression_right_child_node->child(0);
    EXPECT_EQ(NodeRule::LOGAND, foo_expression_right_child_left_node->rule());
    ASSERT_EQ(2, foo_expression_right_child_left_node->child_count());

    Node* foo_expression_right_child_left_left_node
        = foo_expression_right_child_left_node->child(0);
    EXPECT_EQ(NodeRule::BOOL, foo_expression_right_child_left_left_node->rule());
    ASSERT_EQ(true, foo_expression_right_child_left_left_node->bool_value());

    Node* foo_expression_right_child_left_right_node
        = foo_expression_right_child_left_node->child(1);
    EXPECT_EQ(NodeRule::BOOL, foo_expression_right_child_left_right_node->rule());
    ASSERT_FALSE(foo_expression_right_child_left_right_node->bool_value());

    Node* foo_expression_right_child_right_node = foo_expression_right_child_node->child(1);
    EXPECT_EQ(NodeRule::BOOL, foo_expression_right_child_right_node->rule());
    ASSERT_FALSE(foo_expression_right_child_right_node->bool_value());

    delete root;
}

TEST(Parser, ParseStringExpression) {

    std::string input("package \"FooPackage\";"
        "public spec MySpec {"
        "  public bool Foo() {"
        "    return \"Hello\" + \"World\" + \"Improved\";"
        "  }"
        "}");

    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();
    Node* specs_node = root->child(2);
    Node* spec_node = specs_node->child(0);
    Node* functions_node = spec_node->child(2);
    Node* foo_node = functions_node->child(0);

    Node* foo_block_node = foo_node->child(4);
    EXPECT_EQ(1, foo_block_node->child_count());

    Node* foo_return_node = foo_block_node->child(0);
    EXPECT_EQ(NodeRule::RETURN, foo_return_node->rule());
    ASSERT_EQ(1, foo_return_node->child_count());

    Node* foo_return_expression_node = foo_return_node->child(0);
    EXPECT_EQ(NodeRule::EXPRESSION, foo_return_expression_node->rule());
    ASSERT_EQ(1, foo_return_expression_node->child_count());

    Node* foo_expression_root_node = foo_return_expression_node->child(0);
    EXPECT_EQ(NodeRule::ADD, foo_expression_root_node->rule());
    ASSERT_EQ(2, foo_expression_root_node->child_count());

    Node* foo_subexpression_root_node = foo_expression_root_node->child(0);
    EXPECT_EQ(NodeRule::ADD, foo_subexpression_root_node->rule());
    ASSERT_EQ(2, foo_subexpression_root_node->child_count());

    Node* foo_subexpression_left_node = foo_subexpression_root_node->child(0);
    EXPECT_EQ(NodeRule::STRING, foo_subexpression_left_node->rule());
    EXPECT_STREQ("Hello", foo_subexpression_left_node->string_value()->c_str());

    Node* foo_subexpression_right_node = foo_subexpression_root_node->child(1);
    EXPECT_EQ(NodeRule::STRING, foo_subexpression_right_node->rule());
    EXPECT_STREQ("World", foo_subexpression_right_node->string_value()->c_str());

    Node* foo_expression_right_node = foo_expression_root_node->child(1);
    EXPECT_EQ(NodeRule::STRING, foo_expression_right_node->rule());
    EXPECT_STREQ("Improved", foo_expression_right_node->string_value()->c_str());

    delete root;
}

TEST(Parser, ParseVariableExpression) {

    std::string input("package \"FooPackage\";"
        "public spec MySpec {"
        "  public bool Foo() {"
        "    return 3 + x;"
        "  }"
        "}");

    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();
    Node* specs_node = root->child(2);
    Node* spec_node = specs_node->child(0);
    Node* functions_node = spec_node->child(2);
    Node* foo_node = functions_node->child(0);

    Node* foo_block_node = foo_node->child(4);
    EXPECT_EQ(1, foo_block_node->child_count());

    Node* foo_return_node = foo_block_node->child(0);
    EXPECT_EQ(NodeRule::RETURN, foo_return_node->rule());
    ASSERT_EQ(1, foo_return_node->child_count());

    Node* foo_return_expression_node = foo_return_node->child(0);
    EXPECT_EQ(NodeRule::EXPRESSION, foo_return_expression_node->rule());
    ASSERT_EQ(1, foo_return_expression_node->child_count());

    Node* foo_expression_root_node = foo_return_expression_node->child(0);
    EXPECT_EQ(NodeRule::ADD, foo_expression_root_node->rule());
    ASSERT_EQ(2, foo_expression_root_node->child_count());

    Node* foo_expression_left_node = foo_expression_root_node->child(0);
    EXPECT_EQ(NodeRule::INT, foo_expression_left_node->rule());
    EXPECT_EQ(3, foo_expression_left_node->int_value());

    Node* foo_expression_right_node = foo_expression_root_node->child(1);
    EXPECT_EQ(NodeRule::SYMBOL, foo_expression_right_node->rule());
    ASSERT_EQ(1, foo_expression_right_node->child_count());

    Node* foo_expression_x_node = foo_expression_right_node->child(0);
    EXPECT_EQ(NodeRule::NAME, foo_expression_x_node->rule());
    EXPECT_STREQ("x", foo_expression_x_node->string_value()->c_str());

    delete root;
}

TEST(Parser, ParseFunctionExpressionWithoutArgs) {

    std::string input("package \"FooPackage\";"
        "public spec MySpec {"
        "  public bool Foo() {"
        "    return 1 % Foo();"
        "  }"
        "}");

    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();
    Node* specs_node = root->child(2);
    Node* spec_node = specs_node->child(0);
    Node* functions_node = spec_node->child(2);
    Node* foo_node = functions_node->child(0);

    Node* foo_block_node = foo_node->child(4);
    EXPECT_EQ(1, foo_block_node->child_count());

    Node* foo_return_node = foo_block_node->child(0);
    EXPECT_EQ(NodeRule::RETURN, foo_return_node->rule());
    ASSERT_EQ(1, foo_return_node->child_count());

    Node* foo_return_expression_node = foo_return_node->child(0);
    EXPECT_EQ(NodeRule::EXPRESSION, foo_return_expression_node->rule());
    ASSERT_EQ(1, foo_return_expression_node->child_count());

    Node* foo_expression_root_node = foo_return_expression_node->child(0);
    EXPECT_EQ(NodeRule::MOD, foo_expression_root_node->rule());
    ASSERT_EQ(2, foo_expression_root_node->child_count());

    Node* foo_expression_left_node = foo_expression_root_node->child(0);
    EXPECT_EQ(NodeRule::INT, foo_expression_left_node->rule());
    EXPECT_EQ(1, foo_expression_left_node->int_value());

    Node* foo_expression_call_node = foo_expression_root_node->child(1);
    EXPECT_EQ(NodeRule::CALL, foo_expression_call_node->rule());
    ASSERT_EQ(2, foo_expression_call_node->child_count());

    Node* foo_expression_call_name_node = foo_expression_call_node->child(0);
    EXPECT_EQ(NodeRule::NAME, foo_expression_call_name_node->rule());
    EXPECT_STREQ("Foo", foo_expression_call_name_node->string_value()->c_str());

    Node* foo_expression_call_parameters_node = foo_expression_call_node->child(1);
    EXPECT_EQ(NodeRule::CALL_PARAMETERS, foo_expression_call_parameters_node->rule());
    EXPECT_EQ(0, foo_expression_call_parameters_node->child_count());

    delete root;
}

TEST(Parser, ParseFunctionExpressionWithArgs) {

    std::string input("package \"FooPackage\";"
        "public spec MySpec {"
        "  public bool Foo() {"
        "    return Another(3 + 9, (Single(3))) / 2.5;"
        "  }"
        "}");

    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();
    Node* specs_node = root->child(2);
    Node* spec_node = specs_node->child(0);
    Node* functions_node = spec_node->child(2);
    Node* foo_node = functions_node->child(0);

    Node* foo_block_node = foo_node->child(4);
    EXPECT_EQ(1, foo_block_node->child_count());

    Node* foo_return_node = foo_block_node->child(0);
    EXPECT_EQ(NodeRule::RETURN, foo_return_node->rule());
    ASSERT_EQ(1, foo_return_node->child_count());

    Node* foo_return_expression_node = foo_return_node->child(0);
    EXPECT_EQ(NodeRule::EXPRESSION, foo_return_expression_node->rule());
    ASSERT_EQ(1, foo_return_expression_node->child_count());

    Node* foo_expression_root_node = foo_return_expression_node->child(0);
    EXPECT_EQ(NodeRule::DIV, foo_expression_root_node->rule());
    ASSERT_EQ(2, foo_expression_root_node->child_count());

    Node* foo_expression_call_node = foo_expression_root_node->child(0);
    EXPECT_EQ(NodeRule::CALL, foo_expression_call_node->rule());
    ASSERT_EQ(2, foo_expression_call_node->child_count());

    Node* foo_expression_call_name_node = foo_expression_call_node->child(0);
    EXPECT_EQ(NodeRule::NAME, foo_expression_call_name_node->rule());
    EXPECT_STREQ("Another", foo_expression_call_name_node->string_value()->c_str());

    Node* foo_expression_call_parameters_node = foo_expression_call_node->child(1);
    EXPECT_EQ(NodeRule::CALL_PARAMETERS, foo_expression_call_parameters_node->rule());
    EXPECT_EQ(2, foo_expression_call_parameters_node->child_count());

    Node* foo_parameter_0_node = foo_expression_call_parameters_node->child(0);
    EXPECT_EQ(NodeRule::EXPRESSION, foo_parameter_0_node->rule());
    ASSERT_EQ(1, foo_parameter_0_node->child_count());

    Node* foo_parameter_0_root_node = foo_parameter_0_node->child(0);
    EXPECT_EQ(NodeRule::ADD, foo_parameter_0_root_node->rule());
    ASSERT_EQ(2, foo_parameter_0_root_node->child_count());

    Node* foo_parameter_0_left_node = foo_parameter_0_root_node->child(0);
    EXPECT_EQ(NodeRule::INT, foo_parameter_0_left_node->rule());
    ASSERT_EQ(3, foo_parameter_0_left_node->int_value());

    Node* foo_parameter_0_right_node = foo_parameter_0_root_node->child(1);
    EXPECT_EQ(NodeRule::INT, foo_parameter_0_right_node->rule());
    ASSERT_EQ(9, foo_parameter_0_right_node->int_value());

    Node* foo_parameter_1_node = foo_expression_call_parameters_node->child(1);
    EXPECT_EQ(NodeRule::EXPRESSION, foo_parameter_1_node->rule());
    ASSERT_EQ(1, foo_parameter_1_node->child_count());

    Node* foo_parameter_1_root_node = foo_parameter_1_node->child(0);
    EXPECT_EQ(NodeRule::CALL, foo_parameter_1_root_node->rule());
    ASSERT_EQ(2, foo_parameter_1_root_node->child_count());

    Node* foo_parameter_1_name_node = foo_parameter_1_root_node->child(0);
    EXPECT_EQ(NodeRule::NAME, foo_parameter_1_name_node->rule());
    EXPECT_STREQ("Single", foo_parameter_1_name_node->string_value()->c_str());

    Node* foo_parameter_1_parameters_node = foo_parameter_1_root_node->child(1);
    EXPECT_EQ(NodeRule::CALL_PARAMETERS, foo_parameter_1_parameters_node->rule());
    ASSERT_EQ(1, foo_parameter_1_parameters_node->child_count());

    Node* foo_expression_right_node = foo_expression_root_node->child(1);
    EXPECT_EQ(NodeRule::FLOAT, foo_expression_right_node->rule());
    EXPECT_EQ(2.5, foo_expression_right_node->float_value());

    delete root;
}

TEST(Parser, ParseMemberExpression) {

    std::string input("package \"FooPackage\";"
        "public spec MySpec {"
        "  public bool Foo() {"
        "    return 1 + this.foo.x * (foo).print();"
        "  }"
        "}");

    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();
    Node* specs_node = root->child(2);
    Node* spec_node = specs_node->child(0);
    Node* functions_node = spec_node->child(2);
    Node* foo_node = functions_node->child(0);

    Node* foo_block_node = foo_node->child(4);
    EXPECT_EQ(1, foo_block_node->child_count());

    Node* foo_return_node = foo_block_node->child(0);
    EXPECT_EQ(NodeRule::RETURN, foo_return_node->rule());
    ASSERT_EQ(1, foo_return_node->child_count());

    Node* foo_return_expression_node = foo_return_node->child(0);
    EXPECT_EQ(NodeRule::EXPRESSION, foo_return_expression_node->rule());
    ASSERT_EQ(1, foo_return_expression_node->child_count());

    Node* foo_expression_root_node = foo_return_expression_node->child(0);
    EXPECT_EQ(NodeRule::ADD, foo_expression_root_node->rule());
    ASSERT_EQ(2, foo_expression_root_node->child_count());

    Node* foo_expression_one_node = foo_expression_root_node->child(0);
    EXPECT_EQ(NodeRule::INT, foo_expression_one_node->rule());
    ASSERT_EQ(1, foo_expression_one_node->int_value());

    Node* foo_expression_mul_node = foo_expression_root_node->child(1);
    EXPECT_EQ(NodeRule::MUL, foo_expression_mul_node->rule());
    ASSERT_EQ(2, foo_expression_mul_node->child_count());

    Node* foo_expression_xmem_node = foo_expression_mul_node->child(0);
    EXPECT_EQ(NodeRule::MEMBER, foo_expression_xmem_node->rule());
    ASSERT_EQ(2, foo_expression_xmem_node->child_count());

    Node* foo_expression_xthismem_node = foo_expression_xmem_node->child(0);
    EXPECT_EQ(NodeRule::MEMBER, foo_expression_xthismem_node->rule());
    ASSERT_EQ(2, foo_expression_xthismem_node->child_count());

    Node* foo_expression_xthis_node = foo_expression_xthismem_node->child(0);
    EXPECT_EQ(NodeRule::SYMBOL, foo_expression_xthis_node->rule());
    ASSERT_EQ(1, foo_expression_xthis_node->child_count());

    Node* foo_expression_xthisname_node = foo_expression_xthis_node->child(0);
    EXPECT_EQ(NodeRule::NAME, foo_expression_xthisname_node->rule());
    ASSERT_STREQ("this", foo_expression_xthisname_node->string_value()->c_str());

    Node* foo_expression_xfoo_node = foo_expression_xthismem_node->child(1);
    EXPECT_EQ(NodeRule::SYMBOL, foo_expression_xfoo_node->rule());
    ASSERT_EQ(1, foo_expression_xfoo_node->child_count());

    Node* foo_expression_xfooname_node = foo_expression_xfoo_node->child(0);
    EXPECT_EQ(NodeRule::NAME, foo_expression_xfooname_node->rule());
    ASSERT_STREQ("foo", foo_expression_xfooname_node->string_value()->c_str());

    Node* foo_expression_x_node = foo_expression_xmem_node->child(1);
    EXPECT_EQ(NodeRule::SYMBOL, foo_expression_x_node->rule());
    ASSERT_EQ(1, foo_expression_x_node->child_count());

    Node* foo_expression_xname_node = foo_expression_x_node->child(0);
    EXPECT_EQ(NodeRule::NAME, foo_expression_xname_node->rule());
    ASSERT_STREQ("x", foo_expression_xname_node->string_value()->c_str());

    Node* foo_expression_foomem_node = foo_expression_mul_node->child(1);
    EXPECT_EQ(NodeRule::MEMBER, foo_expression_foomem_node->rule());
    ASSERT_EQ(2, foo_expression_foomem_node->child_count());


    Node* foo_expression_foofoo_node = foo_expression_foomem_node->child(0);
    EXPECT_EQ(NodeRule::SYMBOL, foo_expression_foofoo_node->rule());
    ASSERT_EQ(1, foo_expression_foofoo_node->child_count());

    Node* foo_expression_foofooname_node = foo_expression_foofoo_node->child(0);
    EXPECT_EQ(NodeRule::NAME, foo_expression_foofooname_node->rule());
    ASSERT_STREQ("foo", foo_expression_foofooname_node->string_value()->c_str());

    Node* foo_expression_fooprint_node = foo_expression_foomem_node->child(1);
    EXPECT_EQ(NodeRule::CALL, foo_expression_fooprint_node->rule());
    ASSERT_EQ(2, foo_expression_fooprint_node->child_count());

    Node* foo_expression_fooprintname_node = foo_expression_fooprint_node->child(0);
    EXPECT_EQ(NodeRule::NAME, foo_expression_fooprintname_node->rule());
    ASSERT_STREQ("print", foo_expression_fooprintname_node->string_value()->c_str());

    delete root;
}

TEST(Parser, ParseMalformedMemberExpression) {

    // Case 1: Single operand reference.
    {
        std::string input("package \"FooPackage\";"
            "public spec MySpec {"
            "  public bool Foo() {"
            "    return this.;"
            "  }"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_EXPRESSION_INVALID_TOKEN);
    }

    // Case 2: Preceding DOT.
    {
        std::string input("package \"FooPackage\";"
            "public spec MySpec {"
            "  public bool Foo() {"
            "    return .foo;"
            "  }"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_EXPRESSION_INVALID_TOKEN);
    }
}

TEST(Parser, ParseExpressionMemberExpression) {

    // Ensure that parser doesn't throw exception when parsing expression members.
    std::string input("package \"FooPackage\";"
        "public spec MySpec {"
        "  public bool Foo() {"
        "    return ((3 + 5) * 2).ToString();"
        "  }"
        "}");

    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    delete parser.Parse();
}

TEST(Parser, ParseAssignStatement) {
    std::string input("package \"FooPackage\";"
        "public spec MySpec {"
        "  public bool Foo() {"
        "    x <- 3 = 4 + 5;"
        "  }"
        "}");

    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();
    Node* specs_node = root->child(2);
    Node* spec_node = specs_node->child(0);
    Node* functions_node = spec_node->child(2);
    Node* foo_node = functions_node->child(0);

    Node* foo_block_node = foo_node->child(4);
    EXPECT_EQ(1, foo_block_node->child_count());

    Node* foo_assign_statement_node = foo_block_node->child(0);
    EXPECT_EQ(NodeRule::ASSIGN, foo_assign_statement_node->rule());
    ASSERT_EQ(2, foo_assign_statement_node->child_count());

    Node* assign_target_node = foo_assign_statement_node->child(0);
    EXPECT_EQ(NodeRule::SYMBOL, assign_target_node->rule());
    ASSERT_EQ(1, assign_target_node->child_count());

    Node* assign_target_name_node = assign_target_node->child(0);
    EXPECT_EQ(NodeRule::NAME, assign_target_name_node->rule());
    ASSERT_STREQ("x", assign_target_name_node->string_value()->c_str());

    Node* assign_equals_node = foo_assign_statement_node->child(1);
    EXPECT_EQ(NodeRule::EQUALS, assign_equals_node->rule());
    ASSERT_EQ(2, assign_equals_node->child_count());

    Node* assign_equals_left_node = assign_equals_node->child(0);
    EXPECT_EQ(NodeRule::INT, assign_equals_left_node->rule());
    ASSERT_EQ(3, assign_equals_left_node->int_value());

    Node* assign_equals_right_node = assign_equals_node->child(1);
    EXPECT_EQ(NodeRule::ADD, assign_equals_right_node->rule());
    ASSERT_EQ(2, assign_equals_right_node->child_count());

    Node* assign_add_left_node = assign_equals_right_node->child(0);
    EXPECT_EQ(NodeRule::INT, assign_add_left_node->rule());
    ASSERT_EQ(4, assign_add_left_node->int_value());

    Node* assign_add_right_node = assign_equals_right_node->child(1);
    EXPECT_EQ(NodeRule::INT, assign_add_right_node->rule());
    ASSERT_EQ(5, assign_add_right_node->int_value());

    delete root;
}

TEST(Parser, ParseCallStatement) {

    std::string input("package \"FooPackage\";"
        "public spec MySpec {"
        "  public bool Foo() {"
        "    Print(\"Hello\");"
        "  }"
        "}");

    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();
    Node* specs_node = root->child(2);
    Node* spec_node = specs_node->child(0);
    Node* functions_node = spec_node->child(2);
    Node* foo_node = functions_node->child(0);

    Node* foo_block_node = foo_node->child(4);
    EXPECT_EQ(1, foo_block_node->child_count());

    Node* call_statement_node = foo_block_node->child(0);
    EXPECT_EQ(NodeRule::CALL, call_statement_node->rule());
    ASSERT_EQ(2, call_statement_node->child_count());

    Node* call_name_node = call_statement_node->child(0);
    EXPECT_EQ(NodeRule::NAME, call_name_node->rule());
    ASSERT_STREQ("Print", call_name_node->string_value()->c_str());

    Node* call_parameters_node = call_statement_node->child(1);
    EXPECT_EQ(NodeRule::CALL_PARAMETERS, call_parameters_node->rule());
    ASSERT_EQ(1, call_parameters_node->child_count());

    Node* parameter1_expression_node = call_parameters_node->child(0);
    EXPECT_EQ(NodeRule::EXPRESSION, parameter1_expression_node->rule());
    ASSERT_EQ(1, parameter1_expression_node->child_count());

    Node* expression_string_node = parameter1_expression_node->child(0);
    EXPECT_EQ(NodeRule::STRING, expression_string_node->rule());
    ASSERT_STREQ("Hello", expression_string_node->string_value()->c_str());

    delete root;
}

TEST(Parser, ParseComparisonExpression) {
    std::string input("package \"FooPackage\";"
        "public spec MySpec {"
        "  public bool Foo() {"
        "    tfVal <- 3 > 2 || 2 < 4 && 5 >= 4 || !(1.5 <= 4);"
        "  }"
        "}");

    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();
    Node* specs_node = root->child(2);
    Node* spec_node = specs_node->child(0);
    Node* functions_node = spec_node->child(2);
    Node* foo_node = functions_node->child(0);

    Node* foo_block_node = foo_node->child(4);
    EXPECT_EQ(1, foo_block_node->child_count());

    Node* assign_node = foo_block_node->child(0);
    EXPECT_EQ(NodeRule::ASSIGN, assign_node->rule());
    ASSERT_EQ(2, assign_node->child_count());

    Node* or1_node = assign_node->child(1);
    EXPECT_EQ(NodeRule::LOGOR, or1_node->rule());
    ASSERT_EQ(2, or1_node->child_count());

    Node* or2_node = or1_node->child(0);
    EXPECT_EQ(NodeRule::LOGOR, or2_node->rule());
    ASSERT_EQ(2, or2_node->child_count());

    Node* gt_node = or2_node->child(0);
    EXPECT_EQ(NodeRule::GREATER, gt_node->rule());
    ASSERT_EQ(2, gt_node->child_count());

    Node* gt_left_node = gt_node->child(0);
    EXPECT_EQ(NodeRule::INT, gt_left_node->rule());
    ASSERT_EQ(3, gt_left_node->int_value());

    Node* gt_right_node = gt_node->child(1);
    EXPECT_EQ(NodeRule::INT, gt_right_node->rule());
    ASSERT_EQ(2, gt_right_node->int_value());

    Node* and_node = or2_node->child(1);
    EXPECT_EQ(NodeRule::LOGAND, and_node->rule());
    ASSERT_EQ(2, and_node->child_count());

    Node* lt_node = and_node->child(0);
    EXPECT_EQ(NodeRule::LESS, lt_node->rule());
    ASSERT_EQ(2, lt_node->child_count());

    Node* lt_left_node = lt_node->child(0);
    EXPECT_EQ(NodeRule::INT, lt_left_node->rule());
    ASSERT_EQ(2, lt_left_node->int_value());

    Node* lt_right_node = lt_node->child(1);
    EXPECT_EQ(NodeRule::INT, lt_right_node->rule());
    ASSERT_EQ(4, lt_right_node->int_value());

    Node* gte_node = and_node->child(1);
    EXPECT_EQ(NodeRule::GREATER_EQUALS, gte_node->rule());
    ASSERT_EQ(2, gte_node->child_count());

    Node* gte_left_node = gte_node->child(0);
    EXPECT_EQ(NodeRule::INT, gte_left_node->rule());
    ASSERT_EQ(5, gte_left_node->int_value());

    Node* gte_right_node = gte_node->child(1);
    EXPECT_EQ(NodeRule::INT, gte_right_node->rule());
    ASSERT_EQ(4, gte_right_node->int_value());

    Node* not_node = or1_node->child(1);
    EXPECT_EQ(NodeRule::LOGNOT, not_node->rule());
    ASSERT_EQ(1, not_node->child_count());

    Node* lte_node = not_node->child(0);
    EXPECT_EQ(NodeRule::LESS_EQUALS, lte_node->rule());
    ASSERT_EQ(2, lte_node->child_count());

    Node* lte_left_node = lte_node->child(0);
    EXPECT_EQ(NodeRule::FLOAT, lte_left_node->rule());
    ASSERT_EQ(1.5, lte_left_node->float_value());

    Node* lte_right_node = lte_node->child(1);
    EXPECT_EQ(NodeRule::INT, lte_right_node->rule());
    ASSERT_EQ(4, lte_right_node->int_value());

    delete root;
}

TEST(Parser, ParseMalformedComparisonExpression) {

    // Case 1: missing left operand.
    {
        std::string input("package \"FooPackage\";"
            "public spec MySpec {"
            "  public bool Foo() {"
            "    return > 2;"
            "  }"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_EXPRESSION_INVALID_TOKEN);
    }

    // Case 2: missing right operand.
    {
        std::string input("package \"FooPackage\";"
            "public spec MySpec {"
            "  public bool Foo() {"
            "    return 3 =;"
            "  }"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_EXPRESSION_INVALID_TOKEN);
    }

    // Case 3: missing right and semicolon.
    {
        std::string input("package \"FooPackage\";"
            "public spec MySpec {"
            "  public bool Foo() {"
            "    return 3 ="
            "  }"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_EXPRESSION_INVALID_TOKEN);
    }

    // Case 4: missing semicolon.
    {
        std::string input("package \"FooPackage\";"
            "public spec MySpec {"
            "  public bool Foo() {"
            "    return 3 <= 4"
            "  }"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_EXPECTED_SEMICOLON);
    }

    // Case 5: mismatched LPAREN.
    {
        std::string input("package \"FooPackage\";"
            "public spec MySpec {"
            "  public bool Foo() {"
            "    return true && (3 <= 4;"
            "  }"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_EXPRESSION_MISSING_RPAREN);
    }

    // Case 6: mismatched RPAREN.
    {
        std::string input("package \"FooPackage\";"
            "public spec MySpec {"
            "  public bool Foo() {"
            "    return ((3 <= 4) && false;"
            "  }"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_EXPRESSION_MISSING_RPAREN);
    }

    // Case 7: mismatched RPAREN with NOT
    {
        std::string input("package \"FooPackage\";"
            "public spec MySpec {"
            "  public bool Foo() {"
            "    return !(3 > 3;"
            "  }"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_EXPRESSION_MISSING_RPAREN);
    }

    // Case 8: previously caused SEGFAULT for unknown reason,
    // test to prevent regression.
    {
        std::string input("package \"FooPackage\";"
            "public spec MySpec {"
            "  public bool Foo() {"
            "    tfVal <- 3 > 2 || 2 < 4 && 5 >= 4 || !(1.5 <= 4;"
            "  }"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_EXPRESSION_MISSING_RPAREN);
    }
}

// Checks that we can correctly parse a module with a specs and static functions.
// Although we don't check the entire structure of the tree, presumably the implementation
// uses the same subparsers everywhere so if our earlier function and spec tests pass, this
// one should imply correctness for these as well.
TEST(Parser, ParseModule) {

    std::string input("package \"FooPackage\";"
        "public spec MySpec { } "
        "public int32 main() { } "
        "public spec MySpec2{ } "
        "public int32 main2() { } "
        "public int32 main3() { } ");

    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();
    EXPECT_EQ(4, root->child_count());

    Node* specs_node = root->child(2);
    EXPECT_EQ(NodeRule::SPECS, specs_node->rule());
    EXPECT_EQ(2, specs_node->child_count());
    EXPECT_STREQ("MySpec", specs_node->child(0)->child(1)->string_value()->c_str());
    EXPECT_STREQ("MySpec2", specs_node->child(1)->child(1)->string_value()->c_str());

    Node* functions_node = root->child(3);
    EXPECT_EQ(NodeRule::FUNCTIONS, functions_node->rule());
    EXPECT_EQ(3, functions_node->child_count());
    EXPECT_STREQ("main", functions_node->child(0)->child(2)->string_value()->c_str());
    EXPECT_STREQ("main2", functions_node->child(1)->child(2)->string_value()->c_str());
    EXPECT_STREQ("main3", functions_node->child(2)->child(2)->string_value()->c_str());

    delete root;
}

// Addresses issue #105
TEST(Parser, IncompleteNameExpr) {
    std::string input("package \"FooPackage\";"
        "public spec MySpec {"
        "  public bool Foo() {"
        "    foo"
        "  }"
        "}");

    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    EXPECT_STATUS(parser.Parse(), STATUS_PARSER_INCOMPLETE_NAME_STATEMENT);
}

TEST(Parser, ParseMalformedIfStatement) {
    // Case 1: Missing LPAREN
    {
        std::string input("package \"FooPackage\";"
            "public int32 main() {"
            "    if"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_IF_MISSING_LPAREN);
    }

    // Case 2: Missing expression
    {
        std::string input("package \"FooPackage\";"
            "public int32 main() {"
            "    if ( ) "
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_EXPRESSION_INVALID_TOKEN);
    }

    // Case 3: Missing right parenthesis
    {
        std::string input("package \"FooPackage\";"
            "public int32 main() {"
            "    if ( true { } "
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_IF_MISSING_RPAREN);
    }

    // Case 4: Missing left brace.
    {
        std::string input("package \"FooPackage\";"
            "public int32 main() {"
            "    if ( true || false ) }"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_BLOCK_MISSING_LBRACE);
    }

    // Case 5: Missing right brace
    {
        std::string input("package \"FooPackage\";"
            "public int32 main() {"
            "    if ( true || false ) { "
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_EOF);
    }

    // Case 6: Missing left else brace
    {
        std::string input("package \"FooPackage\";"
            "public int32 main() {"
            "    if ( true || false ) { } else } "
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_BLOCK_MISSING_LBRACE);
    }

    // Case 7: Missing right else brace
    {
        std::string input("package \"FooPackage\";"
            "public int32 main() {"
            "    if ( true || false ) { } else { "
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_EOF);
    }

    // Case 8: Duplicate else
    {
        std::string input("package \"FooPackage\";"
            "public int32 main() {"
            "    if ( true || false ) { } else { } else { } "
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_SPEC_OR_FUNC_ACCESS_MODIFIER_MISSING);
    }
}

TEST(Parser, ParseCorrectIfStatement) {
    // Case 1: Only if
    {
        std::string input("package \"FooPackage\";"
            "public int32 main() {"
            "    if (true) { }"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_NO_THROW(parser.Parse());
    }

    // Case 2: If and else
    {
        std::string input("package \"FooPackage\";"
            "public int32 main() {"
            "    if (true) { } else { }"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_NO_THROW(parser.Parse());
    }


    // Case 3: If and else if
    {
        std::string input("package \"FooPackage\";"
            "public int32 main() {"
            "    if (true) { } else if (false) { }"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_NO_THROW(parser.Parse());
    }

    // Case 4: If and two else if
    {
        std::string input("package \"FooPackage\";"
            "public int32 main() {"
            "    if (true) { } else if (false) { } else if (false) { }"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_NO_THROW(parser.Parse());
    }

    // Case 5: If and two else if and an else
    {
        std::string input("package \"FooPackage\";"
            "public int32 main() {"
            "    if (true) { } else if (false) { } else if (false) { } else { }"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_NO_THROW(parser.Parse());
    }
}

TEST(Parser, ParseCorrectIfStatementTree) {
    std::string input("package \"FooPackage\";"
        "public int32 main() {"
        "    if (true) { } else if (false) { } else if (false) { } else { }"
        "}");

    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();
    Node* functions_node = root->child(3);
    Node* main_function_node = functions_node->child(0);
    Node* function_block_node = main_function_node->child(4);

    Node* if_node = function_block_node->child(0);
    EXPECT_EQ(NodeRule::IF, if_node->rule());
    EXPECT_EQ(3, if_node->child_count());

    Node* if_condition_node = if_node->child(0);
    EXPECT_EQ(NodeRule::EXPRESSION, if_condition_node->rule());

    Node* if_true_block_node = if_node->child(1);
    EXPECT_EQ(NodeRule::BLOCK, if_true_block_node->rule());

    Node* if_false_block_node = if_node->child(2);
    EXPECT_EQ(NodeRule::BLOCK, if_false_block_node->rule());
    EXPECT_EQ(1, if_false_block_node->child_count());

    Node* else_if_node = if_false_block_node->child(0);
    EXPECT_EQ(NodeRule::IF, else_if_node->rule());
    EXPECT_EQ(3, else_if_node->child_count());

    Node* else_if_condition_node = else_if_node->child(0);
    EXPECT_EQ(NodeRule::EXPRESSION, else_if_condition_node->rule());

    Node* else_if_true_block_node = else_if_node->child(1);
    EXPECT_EQ(NodeRule::BLOCK, else_if_true_block_node->rule());

    Node* else_if_false_block_node = else_if_node->child(2);
    EXPECT_EQ(NodeRule::BLOCK, if_false_block_node->rule());
    EXPECT_EQ(1, else_if_false_block_node->child_count());

    Node* else_if_else_if_node = else_if_false_block_node->child(0);
    EXPECT_EQ(NodeRule::IF, else_if_else_if_node->rule());
    EXPECT_EQ(3, else_if_node->child_count());

    Node* else_if_else_if_condition_node = else_if_else_if_node->child(0);
    EXPECT_EQ(NodeRule::EXPRESSION, else_if_else_if_condition_node->rule());

    Node* else_if_else_if_true_block_node = else_if_else_if_node->child(1);
    EXPECT_EQ(NodeRule::BLOCK, else_if_else_if_true_block_node->rule());

    Node* else_if_else_if_false_block_node = else_if_else_if_node->child(2);
    EXPECT_EQ(NodeRule::BLOCK, if_false_block_node->rule());
    EXPECT_EQ(0, else_if_else_if_false_block_node->child_count());
}

TEST(Parser, ParseMalformedForStatement) {
    // Case 1: Missing LPAREN
    {
        std::string input("package \"FooPackage\";"
            "public int32 main() {"
            "    for"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_FOR_MISSING_LPAREN);
    }

    // Case 2: Missing LPAREN
    {
        std::string input("package \"FooPackage\";"
            "public int32 main() {"
            "    for (;) { }"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_EXPECTED_SEMICOLON);
    }

    // Case 3: Missing RPAREN
    {
        std::string input("package \"FooPackage\";"
            "public int32 main() {"
            "    for (;; { }"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_FOR_MISSING_RPAREN);
    }

    // Case 4: Missing LBRACE
    {
        std::string input("package \"FooPackage\";"
            "public int32 main() {"
            "    for (;;) }"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_BLOCK_MISSING_LBRACE);
    }

    // Case 5: Missing RBRACE
    {
        std::string input("package \"FooPackage\";"
            "public int32 main() {"
            "    for (;;) { -"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_BLOCK_MISSING_RBRACE);
    }
}

TEST(Parser, ParseCorrectForStatement) {
    // Case 1: No params.
    {
        std::string input("package \"FooPackage\";"
            "public int32 main() {"
            "    for (;;) { }"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_NO_THROW(parser.Parse());
    }

    // Case 2: init only.
    {
        std::string input("package \"FooPackage\";"
            "public int32 main() {"
            "    for (x <- 1;;) { }"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_NO_THROW(parser.Parse());
    }

    // Case 3: condition only.
    {
        std::string input("package \"FooPackage\";"
            "public int32 main() {"
            "    for (;true;) { }"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_NO_THROW(parser.Parse());
    }

    // Case 4: update only.
    {
        std::string input("package \"FooPackage\";"
            "public int32 main() {"
            "    x <- 1;"
            "    for (;;x <- x + 1) { }"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_NO_THROW(parser.Parse());
    }

    // Case 5: condition and update only.
    {
        std::string input("package \"FooPackage\";"
            "public int32 main() {"
            "    x <- 1;"
            "    for (; x < 3; x <- x + 1) { }"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_NO_THROW(parser.Parse());
    }

    // Case 6: all params
    {
        std::string input("package \"FooPackage\";"
            "public int32 main() {"
            "    for (x <- 1; x < 3; x <- x + 1) { }"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_NO_THROW(parser.Parse());
    }
}

TEST(Parser, CorrectForStatementTreeNoParams) {
    std::string input("package \"FooPackage\";"
        "public int32 main() {"
        "    for (;;) { }"
        "}");

    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();
    Node* functions_node = root->child(3);
    Node* main_function_node = functions_node->child(0);
    Node* function_block_node = main_function_node->child(4);

    Node* for_node = function_block_node->child(0);
    EXPECT_EQ(NodeRule::FOR, for_node->rule());
    EXPECT_EQ(4, for_node->child_count());

    Node* init_node = for_node->child(0);
    EXPECT_EQ(NodeRule::LOOP_INITIALIZE, init_node->rule());
    EXPECT_EQ(0, init_node->child_count());

    Node* condition_node = for_node->child(1);
    EXPECT_EQ(NodeRule::LOOP_CONDITION, condition_node->rule());
    EXPECT_EQ(0, condition_node->child_count());

    Node* update_node = for_node->child(2);
    EXPECT_EQ(NodeRule::LOOP_UPDATE, update_node->rule());
    EXPECT_EQ(0, update_node->child_count());
    
    Node* for_node_block = for_node->child(3);
    EXPECT_EQ(NodeRule::BLOCK, for_node_block->rule());
    EXPECT_EQ(0, for_node_block->child_count());

    delete root;
}

TEST(Parser, CorrectForStatementTreeAllParams) {
    std::string input("package \"FooPackage\";"
        "public int32 main() {"
        "    for (x <- 1; x < 1; x <- x + 1) { }"
        "}");

    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();
    Node* functions_node = root->child(3);
    Node* main_function_node = functions_node->child(0);
    Node* function_block_node = main_function_node->child(4);

    Node* for_node = function_block_node->child(0);
    EXPECT_EQ(NodeRule::FOR, for_node->rule());
    EXPECT_EQ(4, for_node->child_count());

    Node* for_init_node = for_node->child(0);
    EXPECT_EQ(NodeRule::LOOP_INITIALIZE, for_init_node->rule());
    EXPECT_EQ(1, for_init_node->child_count());

    Node* init_expr_node = for_init_node->child(0);
    EXPECT_EQ(NodeRule::EXPRESSION, init_expr_node->rule());
    EXPECT_EQ(1, init_expr_node->child_count());

    Node* for_condition_node = for_node->child(1);
    EXPECT_EQ(NodeRule::LOOP_CONDITION, for_condition_node->rule());
    EXPECT_EQ(1, for_condition_node->child_count());

    Node* cond_expr_node = for_condition_node->child(0);
    EXPECT_EQ(NodeRule::EXPRESSION, cond_expr_node->rule());
    EXPECT_EQ(1, cond_expr_node->child_count());

    Node* for_update_node = for_node->child(2);
    EXPECT_EQ(NodeRule::LOOP_UPDATE, for_update_node->rule());
    EXPECT_EQ(1, for_update_node->child_count());

    Node* update_expr_node = for_update_node->child(0);
    EXPECT_EQ(NodeRule::EXPRESSION, update_expr_node->rule());
    EXPECT_EQ(1, update_expr_node->child_count());

    Node* for_node_block = for_node->child(3);
    EXPECT_EQ(NodeRule::BLOCK, for_node_block->rule());
    EXPECT_EQ(0, for_node_block->child_count());

    delete root;
}

TEST(Parser, ParseMalformedWhileStatement) {
    // Case 1: Missing LPAREN
    {
        std::string input("package \"FooPackage\";"
            "public int32 main() {"
            "    while"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_WHILE_MISSING_LPAREN);
    }

    // Case 2: Missing LPAREN
    {
        std::string input("package \"FooPackage\";"
            "public int32 main() {"
            "    while (true { }"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_WHILE_MISSING_RPAREN);
    }

    // Case 3: Invalid condition type
    {
        std::string input("package \"FooPackage\";"
            "public int32 main() {"
            "    while (3) { }"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_SEMANTIC_INVALID_LOOP_CONDITION_TYPE);
    }

    // Case 4: Invalid return type
    {
        std::string input("package \"FooPackage\";"
            "public int32 main() {"
            "    while (true) { return false; }"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_SEMANTIC_RETURN_TYPE_MISMATCH);
    }
}

TEST(Parser, CorrectWhileTree) {
    std::string input("package \"FooPackage\";"
        "public int32 main() {"
        "    while (true) { }"
        "}");

    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();
    Node* functions_node = root->child(3);
    Node* main_function_node = functions_node->child(0);
    Node* function_block_node = main_function_node->child(4);

    Node* for_node = function_block_node->child(0);
    EXPECT_EQ(NodeRule::FOR, for_node->rule());
    EXPECT_EQ(4, for_node->child_count());

    Node* for_init_node = for_node->child(0);
    EXPECT_EQ(NodeRule::LOOP_INITIALIZE, for_init_node->rule());
    EXPECT_EQ(0, for_init_node->child_count());

    Node* for_condition_node = for_node->child(1);
    EXPECT_EQ(NodeRule::LOOP_CONDITION, for_condition_node->rule());
    EXPECT_EQ(1, for_condition_node->child_count());

    Node* cond_expr_node = for_condition_node->child(0);
    EXPECT_EQ(NodeRule::EXPRESSION, cond_expr_node->rule());
    EXPECT_EQ(1, cond_expr_node->child_count());

    Node* for_update_node = for_node->child(2);
    EXPECT_EQ(NodeRule::LOOP_UPDATE, for_update_node->rule());
    EXPECT_EQ(0, for_update_node->child_count());

    Node* for_node_block = for_node->child(3);
    EXPECT_EQ(NodeRule::BLOCK, for_node_block->rule());
    EXPECT_EQ(0, for_node_block->child_count());

    delete root;
}

TEST(Parser, ParseMalformedTypeExpression) {
    // Case 1: Missing left most angle brace.
    {
        std::string input("package \"FooPackage\";"
            "public List List<List<int32, bool>, bool>, string> main() {"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_FUNCTION_MISSING_LPAREN);
    }

    // Case 2: Missing an inner angle brace.
    {
        std::string input("package \"FooPackage\";"
            "public List<List List<int32, bool>, bool>, string> main() {"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_FUNCTION_MISSING_LPAREN);
    }

    // Case 3: Missing an inner angle brace (a different one).
    {
        std::string input("package \"FooPackage\";"
            "public List<List<Listint32, bool>, bool>, string> main() {"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_WHILE_MISSING_LPAREN);
    }

    // Case 4: Missing an inner angle brace (a different one).
    {
        std::string input("package \"FooPackage\";"
            "public List<List<Listint32, bool>, bool>, string> main() {"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_WHILE_MISSING_LPAREN);
    }

    // Case 5: Missing an outer close angle brace.
    {
        std::string input("package \"FooPackage\";"
            "public List<List<List<int32, bool>, bool>, string main() {"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_TYPE_PARAM_MISSING_GREATER);
    }

    // Case 6: Missing an outer close angle brace (a different one).
    {
        std::string input("package \"FooPackage\";"
            "public List<List<List<int32, bool>, bool, string> main() {"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_TYPE_PARAM_MISSING_GREATER);
    }

    // Case 7: Missing a delimiter.
    {
        std::string input("package \"FooPackage\";"
            "public List<List<List<int32, bool> bool, string> main() {"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_TYPE_PARAM_MISSING_COMMA);
    }

    // Case 8: Missing a different delimiter.
    {
        std::string input("package \"FooPackage\";"
            "public List<List<List<int32, bool>, bool> string> main() {"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_TYPE_PARAM_MISSING_COMMA);
    }

    // Case 9: Missing name.
    {
        std::string input("package \"FooPackage\";"
            "public List<List<List<int32, >, bool>, string> main() {"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_TYPE_PARAM_MISSING_NAME);
    }

    // Case 10: Missing name.
    {
        std::string input("package \"FooPackage\";"
            "public List<List<List<int32, bool>, >, string> main() {"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_TYPE_PARAM_MISSING_NAME);
    }

    // Case 11: Missing name.
    {
        std::string input("package \"FooPackage\";"
            "public List<> main() {"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_TYPE_PARAM_MISSING_NAME);
    }
}

TEST(Parser, CorrectTypeExpression) {
    // Case 1: Nested type params.
    {
        std::string input("package \"FooPackage\";"
            "public List<List<List<int32, bool>, bool>, List<string, int32>> main() {"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_NO_THROW(parser.Parse());
    }

    // Case 2: No parameters.
    {
        std::string input("package \"FooPackage\";"
            "public List main() {"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_NO_THROW(parser.Parse());
    }

    // Case 3: One parameter.
    {
        std::string input("package \"FooPackage\";"
            "public List<int32> main() {"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_NO_THROW(parser.Parse());
    }

    // Case 4: Two parameters.
    {
        std::string input("package \"FooPackage\";"
            "public List<int32, bool> main() {"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_NO_THROW(parser.Parse());
    }
}

TEST(Parser, CorrectTypeExpressionTree) {
    std::string input("package \"FooPackage\";"
        "public List<List<bool>, int32> main() {"
        "}");

    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();
    Node* functions_node = root->child(3);
    Node* main_function_node = functions_node->child(0);

    Node* type_node = main_function_node->child(1);
    EXPECT_STREQ("List", type_node->string_value()->c_str());
    EXPECT_EQ(NodeRule::TYPE, type_node->rule());
    EXPECT_EQ(2, type_node->child_count());

    Node* left_type_param = type_node->child(0);
    EXPECT_STREQ("List", left_type_param->string_value()->c_str());
    EXPECT_EQ(NodeRule::TYPE, left_type_param->rule());
    EXPECT_EQ(1, left_type_param->child_count());

    Node* left_type_param_param = left_type_param->child(0);
    EXPECT_STREQ("bool", left_type_param_param->string_value()->c_str());
    EXPECT_EQ(NodeRule::TYPE, left_type_param_param->rule());
    EXPECT_EQ(0, left_type_param_param->child_count());

    Node* right_type_param = type_node->child(1);
    EXPECT_STREQ("int32", right_type_param->string_value()->c_str());
    EXPECT_EQ(NodeRule::TYPE, right_type_param->rule());
    EXPECT_EQ(0, right_type_param->child_count());

    delete root;
}

TEST(Parser, ParseMalformedNewExpression) {
    // Case 1: Missing name.
    {
        std::string input("package \"FooPackage\";"
            "public int32 main() {"
            "    x <- new ();"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_NEW_EXPRESSION_MISSING_NAME);
    }

    // Case 2: Missing left parenthesis.
    {
        std::string input("package \"FooPackage\";"
            "public int32 main() {"
            "    x <- new List);"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_NEW_EXPRESSION_MISSING_LPAREN);
    }

    // Case 3: Missing right parenthesis.
    {
        std::string input("package \"FooPackage\";"
            "public int32 main() {"
            "    x <- new List(;"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_NEW_EXPRESSION_MISSING_RPAREN);
    }
}

TEST(Parser, ParseCorrectNewExpression) {
    // Case 1: No params.
    {
        std::string input("package \"FooPackage\";"
            "public int32 main() {"
            "    x <- new List();"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_NO_THROW(parser.Parse());
    }

    // Case 2: One param.
    {
        std::string input("package \"FooPackage\";"
            "public int32 main() {"
            "    x <- new List(3);"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_NO_THROW(parser.Parse());
    }

    // Case 3: Two params.
    {
        std::string input("package \"FooPackage\";"
            "public int32 main() {"
            "    x <- new List(3, true);"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_NO_THROW(parser.Parse());
    }

    // Case 4: Two params and one generic
    {
        std::string input("package \"FooPackage\";"
            "public int32 main() {"
            "    x <- new List<int32>(3, true);"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_NO_THROW(parser.Parse());
    }

    // Case 5: Three params and two generics
    {
        std::string input("package \"FooPackage\";"
            "public int32 main() {"
            "    x <- new List<int32, bool>(3, true, \"hello\");"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_NO_THROW(parser.Parse());
    }
}

TEST(Parser, ParseCorrectNewExpressionTree) {
    std::string input("package \"FooPackage\";"
        "public int32 main() {"
        "    x <- new List<int32, bool>(3, true, \"hello\");"
        "}");

    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();
    Node* functions_node = root->child(3);
    Node* main_function_node = functions_node->child(0);
    Node* function_block_node = main_function_node->child(4);
    Node* assign_node = function_block_node->child(0);

    Node* new_node = assign_node->child(1);
    EXPECT_EQ(NodeRule::NEW, new_node->rule());
    EXPECT_EQ(2, new_node->child_count());

    Node* type_node = new_node->child(0);
    EXPECT_EQ(NodeRule::TYPE, type_node->rule());
    EXPECT_EQ(2, type_node->child_count());

    Node* call_params_node = new_node->child(1);
    EXPECT_EQ(NodeRule::CALL_PARAMETERS, call_params_node->rule());
    EXPECT_EQ(3, call_params_node->child_count());
}

TEST(Parser, ParseMalformedDefaultExpression) {
    // Case 1: No params or semicolon.
    {
        std::string input("package \"FooPackage\";"
            "public int32 main() {"
            "    x <- default"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_DEFAULT_EXPRESSION_MISSING_LPAREN);
    }

    // Case 2: Missing LPAREN.
    {
        std::string input("package \"FooPackage\";"
            "public int32 main() {"
            "    x <- defaultint32);"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_DEFAULT_EXPRESSION_MISSING_LPAREN);
    }

    // Case 3: Missing Type.
    {
        std::string input("package \"FooPackage\";"
            "public int32 main() {"
            "    x <- default();"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_DEFAULT_EXPRESSION_MISSING_TYPE);
    }

    // Case 4: Missing Closing Brace.
    {
        std::string input("package \"FooPackage\";"
            "public int32 main() {"
            "    x <- default(int32;"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_DEFAULT_EXPRESSION_MISSING_RPAREN);
    }
}

TEST(Parser, CorrectDefaultExpressionTree) {
    std::string input("package \"FooPackage\";"
        "public int32 main() {"
        "    x <- default(List<int32>);"
        "}");

    CompilerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* node = parser.Parse();



    delete node;
}

} // namespace compiler
} // namespace gunderscript
