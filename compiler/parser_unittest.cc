// Gunderscript 2 Parser Tests
// (C) 2014 Christian Gunderman
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
    EXPECT_EQ(NodeRule::NAME, spec_node_0_name->rule());
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
    EXPECT_EQ(NodeRule::NAME, spec_node_1_name->rule());
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
    ASSERT_EQ(6, foo_node->child_count());

    Node* foo_access_modifier_node = foo_node->child(0);
    EXPECT_EQ(NodeRule::ACCESS_MODIFIER, foo_access_modifier_node->rule());
    EXPECT_EQ(LexerSymbol::PUBLIC, foo_access_modifier_node->symbol_value());

    Node* foo_native_node = foo_node->child(1);
    EXPECT_EQ(NodeRule::NATIVE, foo_native_node->rule());
    EXPECT_FALSE(foo_native_node->bool_value());

    Node* foo_type_node = foo_node->child(2);
    EXPECT_EQ(NodeRule::TYPE, foo_type_node->rule());
    EXPECT_STREQ("int", foo_type_node->string_value()->c_str());

    Node* foo_name_node = foo_node->child(3);
    EXPECT_EQ(NodeRule::NAME, foo_name_node->rule());
    EXPECT_STREQ("Foo", foo_name_node->string_value()->c_str());

    Node* foo_parameters_node = foo_node->child(4);
    EXPECT_EQ(NodeRule::FUNCTION_PARAMETERS, foo_parameters_node->rule());
    EXPECT_EQ(0, foo_parameters_node->child_count());

    Node* foo_block_node = foo_node->child(5);
    EXPECT_EQ(0, foo_block_node->child_count());

    delete root;
}

TEST(Parser, ParseConstructorEmpty) {

    std::string input("package \"FooPackage\";"
        "public spec MySpec {"
        "  public construct(int32 x) {"
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
    ASSERT_EQ(6, foo_node->child_count());

    Node* foo_access_modifier_node = foo_node->child(0);
    EXPECT_EQ(NodeRule::ACCESS_MODIFIER, foo_access_modifier_node->rule());
    EXPECT_EQ(LexerSymbol::PUBLIC, foo_access_modifier_node->symbol_value());

    Node* foo_native_node = foo_node->child(1);
    EXPECT_EQ(NodeRule::NATIVE, foo_native_node->rule());
    EXPECT_FALSE(foo_native_node->bool_value());

    Node* foo_type_node = foo_node->child(2);
    EXPECT_EQ(NodeRule::TYPE, foo_type_node->rule());
    EXPECT_EQ(NULL, foo_type_node->string_value());

    Node* foo_name_node = foo_node->child(3);
    EXPECT_EQ(NodeRule::NAME, foo_name_node->rule());
    EXPECT_EQ(NULL, foo_name_node->string_value());

    Node* foo_parameters_node = foo_node->child(4);
    EXPECT_EQ(NodeRule::FUNCTION_PARAMETERS, foo_parameters_node->rule());
    EXPECT_EQ(1, foo_parameters_node->child_count());

    Node* foo_block_node = foo_node->child(5);
    EXPECT_EQ(0, foo_block_node->child_count());

    delete root;
}

TEST(Parser, ParseFunctionEmptyOneParameterNative) {

    std::string input("package \"FooPackage\";"
        "public spec MySpec {"
        "  concealed native string Foo2(int x);"
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
    EXPECT_EQ(LexerSymbol::CONCEALED, foo_access_modifier_node->symbol_value());

    Node* foo_native_node = foo_node->child(1);
    EXPECT_EQ(NodeRule::NATIVE, foo_native_node->rule());
    EXPECT_EQ(true, foo_native_node->bool_value());

    Node* foo_type_node = foo_node->child(2);
    EXPECT_EQ(NodeRule::TYPE, foo_type_node->rule());
    EXPECT_STREQ("string", foo_type_node->string_value()->c_str());

    Node* foo_name_node = foo_node->child(3);
    EXPECT_EQ(NodeRule::NAME, foo_name_node->rule());
    EXPECT_STREQ("Foo2", foo_name_node->string_value()->c_str());

    Node* foo_parameters_node = foo_node->child(4);
    EXPECT_EQ(NodeRule::FUNCTION_PARAMETERS, foo_parameters_node->rule());
    EXPECT_EQ(1, foo_parameters_node->child_count());

    Node* foo_x_node = foo_parameters_node->child(0);
    EXPECT_EQ(NodeRule::FUNCTION_PARAMETER, foo_x_node->rule());
    ASSERT_EQ(2, foo_x_node->child_count());

    Node* foo_x_type_node = foo_x_node->child(0);
    EXPECT_EQ(NodeRule::TYPE, foo_x_type_node->rule());
    EXPECT_STREQ("int", foo_x_type_node->string_value()->c_str());

    Node* foo_x_name_node = foo_x_node->child(1);
    EXPECT_EQ(NodeRule::NAME, foo_x_name_node->rule());
    EXPECT_STREQ("x", foo_x_name_node->string_value()->c_str());

    delete root;
}

TEST(Parser, ParseFunctionEmptyTwoParameterNative) {

    std::string input("package \"FooPackage\";"
        "public spec MySpec {"
        "  concealed native string Foo2(int x, string y);"
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

    Node* foo_parameters_node = foo_node->child(4);
    EXPECT_EQ(NodeRule::FUNCTION_PARAMETERS, foo_parameters_node->rule());
    EXPECT_EQ(2, foo_parameters_node->child_count());

    Node* foo_x_node = foo_parameters_node->child(0);
    EXPECT_EQ(NodeRule::FUNCTION_PARAMETER, foo_x_node->rule());
    ASSERT_EQ(2, foo_x_node->child_count());

    Node* foo_x_type_node = foo_x_node->child(0);
    EXPECT_EQ(NodeRule::TYPE, foo_x_type_node->rule());
    EXPECT_STREQ("int", foo_x_type_node->string_value()->c_str());

    Node* foo_x_name_node = foo_x_node->child(1);
    EXPECT_EQ(NodeRule::NAME, foo_x_name_node->rule());
    EXPECT_STREQ("x", foo_x_name_node->string_value()->c_str());

    Node* foo_y_node = foo_parameters_node->child(1);
    EXPECT_EQ(NodeRule::FUNCTION_PARAMETER, foo_y_node->rule());
    ASSERT_EQ(2, foo_y_node->child_count());

    Node* foo_y_type_node = foo_y_node->child(0);
    EXPECT_EQ(NodeRule::TYPE, foo_y_type_node->rule());
    EXPECT_STREQ("string", foo_y_type_node->string_value()->c_str());

    Node* foo_y_name_node = foo_y_node->child(1);
    EXPECT_EQ(NodeRule::NAME, foo_y_name_node->rule());
    EXPECT_STREQ("y", foo_y_name_node->string_value()->c_str());

    delete root;
}

TEST(Parser, ParseMultipleFunctions) {

    std::string input("package \"FooPackage\";"
        "public spec MySpec {"
        "  concealed native string Foo2(int x, string y);"
        "  public native int Add();"
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
            "  native string Foo2(int x, string y);"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_SPEC_UNKNOWN_MEMBER);
    }

    // Case 2: out of order function attributes.
    {
        std::string input("package \"FooPackage\";"
            "public spec MySpec {"
            "  native public string Foo(int x, string y);"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_SPEC_UNKNOWN_MEMBER);
    }

    // Case 3: incorrect token type for NAME.
    {
        std::string input("package \"FooPackage\";"
            "public spec MySpec {"
            "  public native string 45(int x, string y);"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_FUNCTION_MISSING_NAME);
    }

    // Case 4: extraneous comma.
    {
        std::string input("package \"FooPackage\";"
            "public spec MySpec {"
            "  public native string Foo(int x,);"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_FUNCTIONPARAMS_MISSING_TYPE);
    }

    // Case 5: incorrect argument.
    {
        std::string input("package \"FooPackage\";"
            "public spec MySpec {"
            "  public native string Foo(public int x);"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_FUNCTIONPARAMS_MISSING_TYPE);
    }

    // Case 6: missing comma.
    {
        std::string input("package \"FooPackage\";"
            "public spec MySpec {"
            "  public native string Foo(int x int y);"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_FUNCTIONPARAMS_MISSING_COMMA);
    }

    // Case 7: missing LPAREN.
    {
        std::string input("package \"FooPackage\";"
            "public spec MySpec {"
            "  public native string Foo int x);"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_FUNCTION_MISSING_LPAREN);
    }

    // Case 8: missing RPAREN.
    {
        std::string input("package \"FooPackage\";"
            "public spec MySpec {"
            "  public native string Foo(int x ;"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_FUNCTIONPARAMS_MISSING_COMMA);
    }

    // Case 9: missing SEMICOLON.
    {
        std::string input("package \"FooPackage\";"
            "public spec MySpec {"
            "  public native string Foo(int x)"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_BLOCK_MISSING_LBRACE);
    }

    // Case 11: missing brace.
    {
        std::string input("package \"FooPackage\";"
            "public spec MySpec {"
            "  public native string Foo(int x) }"
            "}");

        CompilerStringSource source(input);
        Lexer lexer(source);
        Parser parser(lexer);

        EXPECT_STATUS(parser.Parse(), STATUS_PARSER_MALFORMED_BLOCK_MISSING_LBRACE);
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

    Node* foo_block_node = foo_node->child(5);
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

    Node* foo_block_node = foo_node->child(5);
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

    Node* foo_block_node = foo_node->child(5);
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

    Node* foo_block_node = foo_node->child(5);
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

    Node* foo_block_node = foo_node->child(5);
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

    Node* foo_block_node = foo_node->child(5);
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

    Node* foo_block_node = foo_node->child(5);
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

    Node* foo_block_node = foo_node->child(5);
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

    Node* foo_block_node = foo_node->child(5);
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

    Node* foo_block_node = foo_node->child(5);
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

    Node* foo_block_node = foo_node->child(5);
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

    Node* foo_block_node = foo_node->child(5);
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

    Node* foo_block_node = foo_node->child(5);
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

    Node* foo_block_node = foo_node->child(5);
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

    Node* foo_block_node = foo_node->child(5);
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

    Node* foo_block_node = foo_node->child(5);
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
    EXPECT_STREQ("main", functions_node->child(0)->child(3)->string_value()->c_str());
    EXPECT_STREQ("main2", functions_node->child(1)->child(3)->string_value()->c_str());
    EXPECT_STREQ("main3", functions_node->child(2)->child(3)->string_value()->c_str());

    delete root;
}

} // namespace compiler
} // namespace gunderscript
