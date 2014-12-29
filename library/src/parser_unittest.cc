// Gunderscript 2 Parser Tests
// (C) 2014 Christian Gunderman
// Technically more "functional" than unit.
#include "gtest/gtest.h"

#include "lexer.h"
#include "parser.h"

namespace gunderscript {
namespace library {

TEST(Parser, Empty) {
  std::string input("");

  LexerStringSource source(input);
  Lexer lexer(source);
  Parser parser(lexer);

  ASSERT_THROW(parser.Parse(), ParserEndOfFileException);
}

TEST(Parser, PackageOnly) {
  // Default success case with package only.
  std::string input("package \"FooPackage\";");

  LexerStringSource source(input);
  Lexer lexer(source);
  Parser parser(lexer);

  Node* root = parser.Parse();
  EXPECT_EQ(NodeRule::MODULE, root->rule());
  ASSERT_EQ(3, root->child_count());

  Node* name_node = root->GetChild(0);
  EXPECT_EQ(NodeRule::NAME, name_node->rule());
  EXPECT_STREQ("FooPackage", name_node->string_value()->c_str());

  Node* depends_node = root->GetChild(1);
  EXPECT_EQ(NodeRule::DEPENDS, depends_node->rule());
  EXPECT_EQ(0, depends_node->child_count());

  Node* specs_node = root->GetChild(2);
  EXPECT_EQ(NodeRule::SPECS, specs_node->rule());
  EXPECT_EQ(0, specs_node->child_count());

  delete root;
}

TEST(Parser, MalformedPackage) {

  // Case 1: partial package keyword.
  {
    std::string input("packa");

    LexerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    EXPECT_THROW(parser.Parse(), ParserMalformedPackageException);
  }

  // Case 2: incorrect package name type.
  {
    std::string input("package 34");

    LexerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    EXPECT_THROW(parser.Parse(), ParserMalformedPackageException);
  }

  // Case 3: missing semicolon.
  {
    std::string input("package \"package_name\" depends \"foo\";");

    LexerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    // TODO: FIX: This test causes a memory leak in lexer.cc
    EXPECT_THROW(parser.Parse(), ParserUnexpectedTokenException);
  }
}

TEST(Parser, MalformedDepends) {

  // Case 1: partial depends.
  {
    std::string input("package \"food\"; depends");

    LexerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    EXPECT_THROW(parser.Parse(), ParserEndOfFileException);
  }

  // Case 2: incorrect depends type.
  {
    std::string input("package \"food\"; depends 34;");

    LexerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    EXPECT_THROW(parser.Parse(), ParserMalformedDependsException);
  }

  // Case 3: missing semicolon.
  {
    std::string input("package \"Foo\"; depends \"Foo2Package\"");

    LexerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    EXPECT_THROW(parser.Parse(), ParserEndOfFileException);
  }
}

TEST(Parser, PackageDependsOnly) {

  // Case 1: partial depends.
  std::string input("package \"Foo\"; depends \"Foo2\"; depends \"Foo3\";");

  LexerStringSource source(input);
  Lexer lexer(source);
  Parser parser(lexer);

  Node* root = parser.Parse();
  EXPECT_EQ(NodeRule::MODULE, root->rule());
  ASSERT_EQ(3, root->child_count());

  Node* name_node = root->GetChild(0);
  EXPECT_EQ(NodeRule::NAME, name_node->rule());
  EXPECT_STREQ("Foo", name_node->string_value()->c_str());

  Node* depends_node = root->GetChild(1);
  EXPECT_EQ(NodeRule::DEPENDS, depends_node->rule());
  EXPECT_EQ(2, depends_node->child_count());

  Node* specs_node = root->GetChild(2);
  EXPECT_EQ(NodeRule::SPECS, specs_node->rule());
  EXPECT_EQ(0, specs_node->child_count());

  Node* dependency_node_0 = depends_node->GetChild(0);
  EXPECT_EQ(NodeRule::NAME, dependency_node_0->rule());
  EXPECT_EQ(0, dependency_node_0->child_count());
  EXPECT_STREQ("Foo2", dependency_node_0->string_value()->c_str());

  Node* dependency_node_1 = depends_node->GetChild(1);
  EXPECT_EQ(NodeRule::NAME, dependency_node_1->rule());
  EXPECT_EQ(0, dependency_node_1->child_count());
  EXPECT_STREQ("Foo3", dependency_node_1->string_value()->c_str());

  delete root;
}

TEST(Parser, MalformedSpec) {

  // Case 1: missing access modifier.
  {
    std::string input("package \"FooPackage\"; spec MySpec { }");

    LexerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    // TODO: Fix: This test causes a memory leak in lexer.cc
    ASSERT_THROW(parser.Parse(), ParserMalformedSpecException);
  }

  // Case 2: missing spec keyword.
  {
    std::string input("package \"FooPackage\"; concealed MySpec { }");

    LexerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    ASSERT_THROW(parser.Parse(), ParserMalformedSpecException);
  }

  // Case 4: incorrect spec name format.
  {
    std::string input("package \"FooPackage\"; concealed spec \"MySpec\" { }");

    LexerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    ASSERT_THROW(parser.Parse(), ParserMalformedSpecException);
  }

  // Case 4: missing opening brace.
  {
    std::string input("package \"FooPackage\"; concealed spec \"MySpec\"  }");

    LexerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    ASSERT_THROW(parser.Parse(), ParserMalformedSpecException);
  }

  // Case 5: missing closing brace.
  {
    std::string input("package \"FooPackage\"; concealed spec \"MySpec\" { ");

    LexerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    ASSERT_THROW(parser.Parse(), ParserMalformedSpecException);
  }
}

TEST(Parser, EmptySpec) {

  std::string input("package \"FooPackage\";"
                    "public spec MySpec { }"
                    "concealed spec Foo { }");

  LexerStringSource source(input);
  Lexer lexer(source);
  Parser parser(lexer);

  Node* root = parser.Parse();
  EXPECT_EQ(NodeRule::MODULE, root->rule());
  ASSERT_EQ(3, root->child_count());

  Node* name_node = root->GetChild(0);
  EXPECT_EQ(NodeRule::NAME, name_node->rule());
  EXPECT_STREQ("FooPackage", name_node->string_value()->c_str());

  Node* depends_node = root->GetChild(1);
  EXPECT_EQ(NodeRule::DEPENDS, depends_node->rule());
  EXPECT_EQ(0, depends_node->child_count());

  Node* specs_node = root->GetChild(2);
  EXPECT_EQ(NodeRule::SPECS, specs_node->rule());
  EXPECT_EQ(2, specs_node->child_count());

  Node* spec_node_0 = specs_node->GetChild(0);
  EXPECT_EQ(NodeRule::SPEC, spec_node_0->rule());
  EXPECT_EQ(3, spec_node_0->child_count());

  Node* spec_node_1 = specs_node->GetChild(1);
  EXPECT_EQ(NodeRule::SPEC, spec_node_1->rule());
  EXPECT_EQ(3, spec_node_1->child_count());

  Node* spec_node_0_name = spec_node_0->GetChild(0);
  EXPECT_EQ(NodeRule::NAME, spec_node_0_name->rule());
  EXPECT_EQ(0, spec_node_0_name->child_count());
  EXPECT_STREQ("MySpec", spec_node_0_name->string_value()->c_str());

  Node* spec_node_0_functions = spec_node_0->GetChild(1);
  EXPECT_EQ(NodeRule::FUNCTIONS, spec_node_0_functions->rule());
  EXPECT_EQ(0, spec_node_0_functions->child_count());

  Node* spec_node_0_properties = spec_node_0->GetChild(2);
  EXPECT_EQ(NodeRule::PROPERTIES, spec_node_0_properties->rule());
  EXPECT_EQ(0, spec_node_0_properties->child_count());

  Node* spec_node_1_name = spec_node_1->GetChild(0);
  EXPECT_EQ(NodeRule::NAME, spec_node_1_name->rule());
  EXPECT_EQ(0, spec_node_1_name->child_count());
  EXPECT_STREQ("Foo", spec_node_1_name->string_value()->c_str());

  Node* spec_node_1_functions = spec_node_1->GetChild(1);
  EXPECT_EQ(NodeRule::FUNCTIONS, spec_node_1_functions->rule());
  EXPECT_EQ(0, spec_node_1_functions->child_count());

  Node* spec_node_1_properties = spec_node_1->GetChild(2);
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

    LexerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    EXPECT_THROW(parser.Parse(), ParserMalformedFunctionException);
  }

  // Case 2: incorrect symbol type property name.
  {
    std::string input("package \"FooPackage\";"
                      "public spec MySpec { "
                      "  float 34 { public get; concealed set; }"
                      "}");

    LexerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    EXPECT_THROW(parser.Parse(), ParserMalformedPropertyException);
  }

  // Case 3: missing opening brace.
  {
    std::string input("package \"FooPackage\";"
                      "public spec MySpec { "
                      "  float X  public get; concealed set; }"
                      "}");

    LexerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    EXPECT_THROW(parser.Parse(), ParserMalformedPropertyException);
  }

  // Case 4: missing closing brace.
  {
    std::string input("package \"FooPackage\";"
                      "public spec MySpec { "
                      "  float X { public get; concealed set; "
                      "  float Y { }"
                      "}");

    LexerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    EXPECT_THROW(parser.Parse(), ParserMalformedPropertyException);
  }

  // Case 5: missing property accessor/mutator access modifier.
  {
    std::string input("package \"FooPackage\";"
                      "public spec MySpec { "
                      "  float X { get; concealed set; }"
                      "}");

    LexerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    EXPECT_THROW(parser.Parse(), ParserMalformedPropertyException);
  }

  // Case 6: multiple get/set.
  {
    std::string input("package \"FooPackage\";"
                      "public spec MySpec { "
                      "  float X { public set; concealed set; }"
                      "}");

    LexerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    EXPECT_THROW(parser.Parse(), ParserMalformedPropertyException);
  }

  // Case 7: semicolon following property accessor body
  {
    std::string input("package \"FooPackage\";"
                      "public spec MySpec { "
                      "  float X { public get { }; concealed set; }"
                      "}");

    LexerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    // TODO: this exception may be incorrect.
    EXPECT_THROW(parser.Parse(), ParserMalformedPropertyException);
  }
}

TEST(Parser, ParsePropertyEmpty) {

  std::string input("package \"FooPackage\";"
                    "public spec MySpec {"
                    "  int X { }"
                    "  string Y { }"
                    "}");

  LexerStringSource source(input);
  Lexer lexer(source);
  Parser parser(lexer);

  Node* root = parser.Parse();
  ASSERT_EQ(3, root->child_count());

  Node* specs_node = root->GetChild(2);
  ASSERT_EQ(1, specs_node->child_count());

  Node* spec_node = specs_node->GetChild(0);
  ASSERT_EQ(3, spec_node->child_count());

  Node* properties_node = spec_node->GetChild(2);
  ASSERT_EQ(2, properties_node->child_count());

  Node* x_node = properties_node->GetChild(0);
  EXPECT_EQ(NodeRule::PROPERTY, x_node->rule());
  ASSERT_EQ(4, x_node->child_count());

  Node* x_type_node = x_node->GetChild(0);
  EXPECT_EQ(NodeRule::TYPE, x_type_node->rule());
  EXPECT_EQ(LexerSymbol::INT, x_type_node->symbol_value());

  Node* x_name_node = x_node->GetChild(1);
  EXPECT_EQ(NodeRule::NAME, x_name_node->rule());
  EXPECT_STREQ("X", x_name_node->string_value()->c_str());

  Node*  x_getter_node = x_node->GetChild(2);
  EXPECT_EQ(NodeRule::PROPERTY_FUNCTION, x_getter_node->rule());
  ASSERT_EQ(0, x_getter_node->child_count());

  Node* x_setter_node = x_node->GetChild(3);
  EXPECT_EQ(NodeRule::PROPERTY_FUNCTION, x_setter_node->rule());
  ASSERT_EQ(0, x_setter_node->child_count());

  Node* y_node = properties_node->GetChild(1);
  EXPECT_EQ(NodeRule::PROPERTY, y_node->rule());
  ASSERT_EQ(4, y_node->child_count());

  Node* y_type_node = y_node->GetChild(0);
  EXPECT_EQ(NodeRule::TYPE, y_type_node->rule());
  EXPECT_EQ(LexerSymbol::STRING, y_type_node->symbol_value());

  Node* y_name_node = y_node->GetChild(1);
  EXPECT_EQ(NodeRule::NAME, y_name_node->rule());
  EXPECT_STREQ("Y", y_name_node->string_value()->c_str());

  Node* y_getter_node = y_node->GetChild(2);
  EXPECT_EQ(NodeRule::PROPERTY_FUNCTION, y_getter_node->rule());
  ASSERT_EQ(0, y_getter_node->child_count());

  Node* y_setter_node = y_node->GetChild(3);
  EXPECT_EQ(NodeRule::PROPERTY_FUNCTION, y_setter_node->rule());
  ASSERT_EQ(0, y_setter_node->child_count());

  delete root;
}

TEST(Parser, ParsePropertyAuto) {

  // Case 1: get first, set second.
  {
    std::string input("package \"FooPackage\";"
                      "public spec MySpec { "
                      "  float X { public get; concealed set; }"
                      "}");

    LexerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();
    ASSERT_EQ(3, root->child_count());

    Node* specs_node = root->GetChild(2);
    ASSERT_EQ(1, specs_node->child_count());

    Node* spec_node = specs_node->GetChild(0);
    ASSERT_EQ(3, spec_node->child_count());

    Node* properties_node = spec_node->GetChild(2);
    ASSERT_EQ(1, properties_node->child_count());

    Node* x_node = properties_node->GetChild(0);
    EXPECT_EQ(NodeRule::PROPERTY, x_node->rule());
    ASSERT_EQ(4, x_node->child_count());

    Node* x_type_node = x_node->GetChild(0);
    EXPECT_EQ(NodeRule::TYPE, x_type_node->rule());
    EXPECT_EQ(LexerSymbol::FLOAT, x_type_node->symbol_value());

    Node* x_name_node = x_node->GetChild(1);
    EXPECT_EQ(NodeRule::NAME, x_name_node->rule());
    EXPECT_STREQ("X", x_name_node->string_value()->c_str());

    Node*  x_getter_node = x_node->GetChild(2);
    EXPECT_EQ(NodeRule::PROPERTY_FUNCTION, x_getter_node->rule());
    ASSERT_EQ(1, x_getter_node->child_count());

    Node* x_getter_access_modifier_node = x_getter_node->GetChild(0);
    EXPECT_EQ(NodeRule::ACCESS_MODIFIER, x_getter_access_modifier_node->rule());
    EXPECT_EQ(LexerSymbol::PUBLIC, x_getter_access_modifier_node->symbol_value());

    Node* x_setter_node = x_node->GetChild(3);
    EXPECT_EQ(NodeRule::PROPERTY_FUNCTION, x_setter_node->rule());
    ASSERT_EQ(1, x_setter_node->child_count());

    Node* x_setter_access_modifier_node = x_setter_node->GetChild(0);
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

    LexerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    Node* root = parser.Parse();
    Node* specs_node = root->GetChild(2);
    Node* spec_node = specs_node->GetChild(0);
    Node* properties_node = spec_node->GetChild(2);
    Node* x_node = properties_node->GetChild(0);

    Node*  x_getter_node = x_node->GetChild(2);
    Node* x_getter_access_modifier_node = x_getter_node->GetChild(0);
    EXPECT_EQ(NodeRule::ACCESS_MODIFIER, x_getter_access_modifier_node->rule());
    EXPECT_EQ(LexerSymbol::INTERNAL, x_getter_access_modifier_node->symbol_value());

    Node* x_setter_node = x_node->GetChild(3);
    Node* x_setter_access_modifier_node = x_setter_node->GetChild(0);
    EXPECT_EQ(NodeRule::ACCESS_MODIFIER, x_setter_access_modifier_node->rule());
    EXPECT_EQ(LexerSymbol::CONCEALED, x_setter_access_modifier_node->symbol_value());

    delete root;
  }
}

TEST(Parser, ParsePropertyWithFunctionBody) {
  // At the moment, ParseFunctionBody() is incomplete, so there isn't a great
  // way to test this yet.
  FAIL();
}

TEST(Parser, ParseFunctionEmpty) {

  std::string input("package \"FooPackage\";"
                    "public spec MySpec {"
                    "  public int Foo() {"
                    "  }"
                    "}");

  LexerStringSource source(input);
  Lexer lexer(source);
  Parser parser(lexer);

  Node* root = parser.Parse();
  Node* specs_node = root->GetChild(2);
  Node* spec_node = specs_node->GetChild(0);
  Node* functions_node = spec_node->GetChild(1);
  Node* foo_node = functions_node->GetChild(0);
  EXPECT_EQ(NodeRule::FUNCTION, foo_node->rule());
  ASSERT_EQ(6, foo_node->child_count());

  Node* foo_access_modifier_node = foo_node->GetChild(0);
  EXPECT_EQ(NodeRule::ACCESS_MODIFIER, foo_access_modifier_node->rule());
  EXPECT_EQ(LexerSymbol::PUBLIC, foo_access_modifier_node->symbol_value());

  Node* foo_native_node = foo_node->GetChild(1);
  EXPECT_EQ(NodeRule::NATIVE, foo_native_node->rule());
  EXPECT_EQ(false, foo_native_node->bool_value());

  Node* foo_type_node = foo_node->GetChild(2);
  EXPECT_EQ(NodeRule::TYPE, foo_type_node->rule());
  EXPECT_EQ(LexerSymbol::INT, foo_type_node->symbol_value());

  Node* foo_name_node = foo_node->GetChild(3);
  EXPECT_EQ(NodeRule::NAME, foo_name_node->rule());
  EXPECT_STREQ("Foo", foo_name_node->string_value()->c_str());

  Node* foo_parameters_node = foo_node->GetChild(4);
  EXPECT_EQ(NodeRule::PARAMETERS, foo_parameters_node->rule());
  EXPECT_EQ(0, foo_parameters_node->child_count());

  Node* foo_block_node = foo_node->GetChild(5);
  EXPECT_EQ(0, foo_block_node->child_count());

  delete root;
}

TEST(Parser, ParseFunctionEmptyOneParameterNative) {

  std::string input("package \"FooPackage\";"
                    "public spec MySpec {"
                    "  concealed native string Foo2(int x);"
                    "}");

  LexerStringSource source(input);
  Lexer lexer(source);
  Parser parser(lexer);

  Node* root = parser.Parse();
  Node* specs_node = root->GetChild(2);
  Node* spec_node = specs_node->GetChild(0);
  Node* functions_node = spec_node->GetChild(1);
  Node* foo_node = functions_node->GetChild(0);
  EXPECT_EQ(NodeRule::FUNCTION, foo_node->rule());
  ASSERT_EQ(5, foo_node->child_count());

  Node* foo_access_modifier_node = foo_node->GetChild(0);
  EXPECT_EQ(NodeRule::ACCESS_MODIFIER, foo_access_modifier_node->rule());
  EXPECT_EQ(LexerSymbol::CONCEALED, foo_access_modifier_node->symbol_value());

  Node* foo_native_node = foo_node->GetChild(1);
  EXPECT_EQ(NodeRule::NATIVE, foo_native_node->rule());
  EXPECT_EQ(true, foo_native_node->bool_value());

  Node* foo_type_node = foo_node->GetChild(2);
  EXPECT_EQ(NodeRule::TYPE, foo_type_node->rule());
  EXPECT_EQ(LexerSymbol::STRING, foo_type_node->symbol_value());

  Node* foo_name_node = foo_node->GetChild(3);
  EXPECT_EQ(NodeRule::NAME, foo_name_node->rule());
  EXPECT_STREQ("Foo2", foo_name_node->string_value()->c_str());

  Node* foo_parameters_node = foo_node->GetChild(4);
  EXPECT_EQ(NodeRule::PARAMETERS, foo_parameters_node->rule());
  EXPECT_EQ(1, foo_parameters_node->child_count());

  Node* foo_x_node = foo_parameters_node->GetChild(0);
  EXPECT_EQ(NodeRule::PARAMETER, foo_x_node->rule());
  ASSERT_EQ(2, foo_x_node->child_count());

  Node* foo_x_type_node = foo_x_node->GetChild(0);
  EXPECT_EQ(NodeRule::TYPE, foo_x_type_node->rule());
  EXPECT_EQ(LexerSymbol::INT, foo_x_type_node->symbol_value());

  Node* foo_x_name_node = foo_x_node->GetChild(1);
  EXPECT_EQ(NodeRule::NAME, foo_x_name_node->rule());
  EXPECT_STREQ("x", foo_x_name_node->string_value()->c_str());

  delete root;
}

TEST(Parser, ParseFunctionEmptyTwoParameterNative) {

  std::string input("package \"FooPackage\";"
                    "public spec MySpec {"
                    "  concealed native string Foo2(int x, string y);"
                    "}");

  LexerStringSource source(input);
  Lexer lexer(source);
  Parser parser(lexer);

  Node* root = parser.Parse();
  Node* specs_node = root->GetChild(2);
  Node* spec_node = specs_node->GetChild(0);
  Node* functions_node = spec_node->GetChild(1);
  Node* foo_node = functions_node->GetChild(0);
  EXPECT_EQ(NodeRule::FUNCTION, foo_node->rule());
  ASSERT_EQ(5, foo_node->child_count());

  Node* foo_parameters_node = foo_node->GetChild(4);
  EXPECT_EQ(NodeRule::PARAMETERS, foo_parameters_node->rule());
  EXPECT_EQ(2, foo_parameters_node->child_count());

  Node* foo_x_node = foo_parameters_node->GetChild(0);
  EXPECT_EQ(NodeRule::PARAMETER, foo_x_node->rule());
  ASSERT_EQ(2, foo_x_node->child_count());

  Node* foo_x_type_node = foo_x_node->GetChild(0);
  EXPECT_EQ(NodeRule::TYPE, foo_x_type_node->rule());
  EXPECT_EQ(LexerSymbol::INT, foo_x_type_node->symbol_value());

  Node* foo_x_name_node = foo_x_node->GetChild(1);
  EXPECT_EQ(NodeRule::NAME, foo_x_name_node->rule());
  EXPECT_STREQ("x", foo_x_name_node->string_value()->c_str());

  Node* foo_y_node = foo_parameters_node->GetChild(1);
  EXPECT_EQ(NodeRule::PARAMETER, foo_y_node->rule());
  ASSERT_EQ(2, foo_y_node->child_count());

  Node* foo_y_type_node = foo_y_node->GetChild(0);
  EXPECT_EQ(NodeRule::TYPE, foo_y_type_node->rule());
  EXPECT_EQ(LexerSymbol::STRING, foo_y_type_node->symbol_value());

  Node* foo_y_name_node = foo_y_node->GetChild(1);
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
                    "  string FooStringProperty { public get; }"
                    "}");

  LexerStringSource source(input);
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

    LexerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    // TODO: fix to throw ParserMalformedFunctionException instead.
    EXPECT_THROW(parser.Parse(), ParserMalformedSpecException);
  }

  // Case 2: out of order function attributes.
  {
    std::string input("package \"FooPackage\";"
                      "public spec MySpec {"
                      "  native public string Foo(int x, string y);"
                      "}");

    LexerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    // TODO: fix to throw ParserMalformedFunctionException instead.
    EXPECT_THROW(parser.Parse(), ParserMalformedSpecException);
  }

  // Case 3: incorrect token type for NAME.
  {
    std::string input("package \"FooPackage\";"
                      "public spec MySpec {"
                      "  public native string 45(int x, string y);"
                      "}");

    LexerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    EXPECT_THROW(parser.Parse(), ParserMalformedFunctionException);
  }

  // Case 4: extraneous comma.
  {
    std::string input("package \"FooPackage\";"
                      "public spec MySpec {"
                      "  public native string Foo(int x,);"
                      "}");

    LexerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    EXPECT_THROW(parser.Parse(), ParserMalformedFunctionException);
  }

  // Case 5: incorrect argument.
  {
    std::string input("package \"FooPackage\";"
                      "public spec MySpec {"
                      "  public native string Foo(public int x);"
                      "}");

    LexerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    EXPECT_THROW(parser.Parse(), ParserMalformedFunctionException);
  }

  // Case 6: missing comma.
  {
    std::string input("package \"FooPackage\";"
                      "public spec MySpec {"
                      "  public native string Foo(int x int y);"
                      "}");

    LexerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    EXPECT_THROW(parser.Parse(), ParserMalformedFunctionException);
  }

  // Case 7: missing LPAREN.
  {
    std::string input("package \"FooPackage\";"
                      "public spec MySpec {"
                      "  public native string Foo int x);"
                      "}");

    LexerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    EXPECT_THROW(parser.Parse(), ParserMalformedFunctionException);
  }

  // Case 8: missing RPAREN.
  {
    std::string input("package \"FooPackage\";"
                      "public spec MySpec {"
                      "  public native string Foo(int x ;"
                      "}");

    LexerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    EXPECT_THROW(parser.Parse(), ParserMalformedFunctionException);
  }

  // Case 8: missing SEMICOLON.
  {
    std::string input("package \"FooPackage\";"
                      "public spec MySpec {"
                      "  public native string Foo(int x)"
                      "}");

    LexerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    EXPECT_THROW(parser.Parse(), ParserMalformedBodyException);
  }

  // Case 8: missing body.
  {
    std::string input("package \"FooPackage\";"
                      "public spec MySpec {"
                      "  public native string Foo(int x)"
                      "}");

    LexerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    EXPECT_THROW(parser.Parse(), ParserMalformedBodyException);
  }

  // Case 9: missing brace.
  {
    std::string input("package \"FooPackage\";"
                      "public spec MySpec {"
                      "  public native string Foo(int x) }"
                      "}");

    LexerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    EXPECT_THROW(parser.Parse(), ParserMalformedBodyException);
  }
}

TEST(Parser, ParseEmptyReturn) {

  std::string input("package \"FooPackage\";"
                    "public spec MySpec {"
                    "  public string Foo() {"
                    "    return;"
                    "  }"
                    "}");

  LexerStringSource source(input);
  Lexer lexer(source);
  Parser parser(lexer);

  Node* root = parser.Parse();
  Node* specs_node = root->GetChild(2);
  Node* spec_node = specs_node->GetChild(0);
  Node* functions_node = spec_node->GetChild(1);
  Node* foo_node = functions_node->GetChild(0);

  Node* foo_block_node = foo_node->GetChild(5);
  EXPECT_EQ(1, foo_block_node->child_count());

  Node* foo_return_node = foo_block_node->GetChild(0);
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

  LexerStringSource source(input);
  Lexer lexer(source);
  Parser parser(lexer);

  Node* root = parser.Parse();
  Node* specs_node = root->GetChild(2);
  Node* spec_node = specs_node->GetChild(0);
  Node* functions_node = spec_node->GetChild(1);
  Node* foo_node = functions_node->GetChild(0);

  Node* foo_block_node = foo_node->GetChild(5);
  EXPECT_EQ(1, foo_block_node->child_count());

  Node* foo_return_node = foo_block_node->GetChild(0);
  EXPECT_EQ(NodeRule::RETURN, foo_return_node->rule());
  ASSERT_EQ(1, foo_return_node->child_count());

  Node* foo_return_expression_node = foo_return_node->GetChild(0);
  EXPECT_EQ(NodeRule::EXPRESSION, foo_return_expression_node->rule());
  EXPECT_EQ(1, foo_return_expression_node->child_count());

  Node* foo_int_node = foo_return_expression_node->GetChild(0);
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

    LexerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    EXPECT_THROW(parser.Parse(), ParserUnexpectedTokenException);
  }

  // Case 2: no expression but missing semicolon.
  {
    std::string input("package \"FooPackage\";"
                      "public spec MySpec {"
                      "  public string Foo() {"
                      "    return "
                      "  }"
                      "}");

    LexerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    EXPECT_THROW(parser.Parse(), ParserMalformedExpressionException);
  }
}

TEST(Parser, ParseOrderOfOperations) {

  std::string input("package \"FooPackage\";"
                    "public spec MySpec {"
                    "  public string Foo() {"
                    "    return (((15.55 + 5.0) * 3.0) - -1.0 % 2.0) / 'c';"
                    "  }"
                    "}");

  LexerStringSource source(input);
  Lexer lexer(source);
  Parser parser(lexer);

  Node* root = parser.Parse();
  Node* specs_node = root->GetChild(2);
  Node* spec_node = specs_node->GetChild(0);
  Node* functions_node = spec_node->GetChild(1);
  Node* foo_node = functions_node->GetChild(0);

  Node* foo_block_node = foo_node->GetChild(5);
  EXPECT_EQ(1, foo_block_node->child_count());

  Node* foo_return_node = foo_block_node->GetChild(0);
  EXPECT_EQ(NodeRule::RETURN, foo_return_node->rule());
  ASSERT_EQ(1, foo_return_node->child_count());

  Node* foo_return_expression_node = foo_return_node->GetChild(0);
  EXPECT_EQ(NodeRule::EXPRESSION, foo_return_expression_node->rule());
  EXPECT_EQ(1, foo_return_expression_node->child_count());

  Node* foo_expression_root_node = foo_return_expression_node->GetChild(0);
  EXPECT_EQ(NodeRule::DIV, foo_expression_root_node->rule());
  EXPECT_EQ(2, foo_expression_root_node->child_count());

  Node* foo_numerator_node = foo_expression_root_node->GetChild(0);
  EXPECT_EQ(NodeRule::SUB, foo_numerator_node->rule());
  EXPECT_EQ(2, foo_numerator_node->child_count());

  Node* foo_left_multiply_node = foo_numerator_node->GetChild(0);
  EXPECT_EQ(NodeRule::MUL, foo_left_multiply_node->rule());
  EXPECT_EQ(2, foo_left_multiply_node->child_count());

  Node* foo_add_node = foo_left_multiply_node->GetChild(0);
  EXPECT_EQ(NodeRule::ADD, foo_add_node->rule());
  EXPECT_EQ(2, foo_add_node->child_count());

  Node* foo_add_left_operand_node = foo_add_node->GetChild(0);
  EXPECT_EQ(NodeRule::FLOAT, foo_add_left_operand_node->rule());
  EXPECT_EQ(15.55, foo_add_left_operand_node->float_value());

  Node* foo_add_right_operand_node = foo_add_node->GetChild(1);
  EXPECT_EQ(NodeRule::FLOAT, foo_add_right_operand_node->rule());
  EXPECT_EQ(5.0, foo_add_right_operand_node->float_value());

  Node* foo_lm_right_operand_node = foo_left_multiply_node->GetChild(1);
  EXPECT_EQ(NodeRule::FLOAT, foo_lm_right_operand_node->rule());
  EXPECT_EQ(3.0, foo_lm_right_operand_node->float_value());

  Node* foo_right_multiply_node = foo_numerator_node->GetChild(1);
  EXPECT_EQ(NodeRule::MOD, foo_right_multiply_node->rule());
  EXPECT_EQ(2, foo_right_multiply_node->child_count());

  Node* foo_negate_node = foo_right_multiply_node->GetChild(0);
  EXPECT_EQ(NodeRule::SUB, foo_negate_node->rule());
  EXPECT_EQ(2, foo_negate_node->child_count());

  Node* foo_negate_left_operand_node = foo_negate_node->GetChild(0);
  EXPECT_EQ(NodeRule::CHAR, foo_negate_left_operand_node->rule());
  EXPECT_EQ(0, foo_negate_left_operand_node->int_value());

  Node* foo_negate_right_operand_node = foo_negate_node->GetChild(1);
  EXPECT_EQ(NodeRule::FLOAT, foo_negate_right_operand_node->rule());
  EXPECT_EQ(1.0, foo_negate_right_operand_node->float_value());

  Node* foo_rm_right_operand_node = foo_right_multiply_node->GetChild(1);
  EXPECT_EQ(NodeRule::FLOAT, foo_lm_right_operand_node->rule());
  EXPECT_EQ(2.0, foo_rm_right_operand_node->float_value());

  Node* foo_denominator_node = foo_expression_root_node->GetChild(1);
  EXPECT_EQ(NodeRule::CHAR, foo_denominator_node->rule());
  EXPECT_EQ('c', foo_denominator_node->int_value());

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

    LexerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    EXPECT_THROW(parser.Parse(), ParserMalformedExpressionException);
  }

  // Case 2: missing left operand add.
  {
    std::string input("package \"FooPackage\";"
                      "public spec MySpec {"
                      "  public string Foo() {"
                      "    return + 3;"
                      "  }"
                      "}");

    LexerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    EXPECT_THROW(parser.Parse(), ParserMalformedExpressionException);
  }

  // Case 3: missing right operand multiply.
  {
    std::string input("package \"FooPackage\";"
                      "public spec MySpec {"
                      "  public string Foo() {"
                      "    return 3 * ;"
                      "  }"
                      "}");

    LexerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    EXPECT_THROW(parser.Parse(), ParserMalformedExpressionException);
  }

  // Case 4: missing left operand divide.
  {
    std::string input("package \"FooPackage\";"
                      "public spec MySpec {"
                      "  public string Foo() {"
                      "    return / 3;"
                      "  }"
                      "}");

    LexerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    EXPECT_THROW(parser.Parse(), ParserMalformedExpressionException);
  }

  // Case 5: keyword in expression
  {
    std::string input("package \"FooPackage\";"
                      "public spec MySpec {"
                      "  public string Foo() {"
                      "    return 3 * return;"
                      "  }"
                      "}");

    LexerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    EXPECT_THROW(parser.Parse(), ParserMalformedExpressionException);
  }

  // Case 6: extraneous RPAREN.
  {
    std::string input("package \"FooPackage\";"
                      "public spec MySpec {"
                      "  public string Foo() {"
                      "    return -3);"
                      "  }"
                      "}");

    LexerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    EXPECT_THROW(parser.Parse(), ParserUnexpectedTokenException);
  }

  // Case 7: Unclosed parenthesis.
  {
    std::string input("package \"FooPackage\";"
                      "public spec MySpec {"
                      "  public string Foo() {"
                      "    return (-(3 + 34);"
                      "  }"
                      "}");

    LexerStringSource source(input);
    Lexer lexer(source);
    Parser parser(lexer);

    EXPECT_THROW(parser.Parse(), ParserMalformedExpressionException);
  }
}

} // namespace library
} // namespace gunderscript
