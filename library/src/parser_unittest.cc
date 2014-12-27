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

  LexerStringSource* source = new  LexerStringSource(input);
  Lexer lexer(*source);
  Parser parser(lexer);

  ASSERT_THROW(parser.Parse(), ParserEndOfFileException);
}

TEST(Parser, PackageOnly) {
  // Default success case with package only.
  std::string input("package \"FooPackage\";");

  LexerStringSource* source = new  LexerStringSource(input);
  Lexer lexer(*source);
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
}

TEST(Parser, MalformedPackage) {

  // Case 1: partial package keyword.
  {
    std::string input("packa");

    LexerStringSource* source = new  LexerStringSource(input);
    Lexer lexer(*source);
    Parser parser(lexer);

    EXPECT_THROW(parser.Parse(), ParserMalformedPackageException);
  }

  // Case 2: incorrect package name type.
  {
    std::string input("package 34");

    LexerStringSource* source = new  LexerStringSource(input);
    Lexer lexer(*source);
    Parser parser(lexer);

    EXPECT_THROW(parser.Parse(), ParserMalformedPackageException);
  }

  // Case 3: missing semicolon.
  {
    std::string input("package \"package_name\" depends \"foo\";");

    LexerStringSource* source = new  LexerStringSource(input);
    Lexer lexer(*source);
    Parser parser(lexer);

    EXPECT_THROW(parser.Parse(), ParserUnexpectedTokenException);
  }
}

TEST(Parser, MalformedDepends) {

  // Case 1: partial depends.
  {
    std::string input("package \"food\"; depends");

    LexerStringSource* source = new  LexerStringSource(input);
    Lexer lexer(*source);
    Parser parser(lexer);

    EXPECT_THROW(parser.Parse(), ParserEndOfFileException);
  }

  // Case 2: incorrect depends type.
  {
    std::string input("package \"food\"; depends 34;");

    LexerStringSource* source = new  LexerStringSource(input);
    Lexer lexer(*source);
    Parser parser(lexer);

    EXPECT_THROW(parser.Parse(), ParserMalformedDependsException);
  }

  // Case 3: missing semicolon.
  {
    std::string input("package \"Foo\"; depends \"Foo2Package\"");

    LexerStringSource* source = new  LexerStringSource(input);
    Lexer lexer(*source);
    Parser parser(lexer);

    EXPECT_THROW(parser.Parse(), ParserEndOfFileException);
  }
}

TEST(Parser, PackageDependsOnly) {

  // Case 1: partial depends.
  std::string input("package \"Foo\"; depends \"Foo2\"; depends \"Foo3\";");

  LexerStringSource* source = new  LexerStringSource(input);
  Lexer lexer(*source);
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
}

TEST(Parser, MalformedSpec) {

  // Case 1: missing access modifier.
  {
    std::string input("package \"FooPackage\"; spec MySpec { }");

    LexerStringSource* source = new  LexerStringSource(input);
    Lexer lexer(*source);
    Parser parser(lexer);

    ASSERT_THROW(parser.Parse(), ParserMalformedSpecException);
  }

  // Case 2: missing spec keyword.
  {
    std::string input("package \"FooPackage\"; concealed MySpec { }");

    LexerStringSource* source = new  LexerStringSource(input);
    Lexer lexer(*source);
    Parser parser(lexer);

    ASSERT_THROW(parser.Parse(), ParserMalformedSpecException);
  }

  // Case 4: incorrect spec name format.
  {
    std::string input("package \"FooPackage\"; concealed spec \"MySpec\" { }");

    LexerStringSource* source = new  LexerStringSource(input);
    Lexer lexer(*source);
    Parser parser(lexer);

    ASSERT_THROW(parser.Parse(), ParserMalformedSpecException);
  }

  // Case 4: missing opening brace.
  {
    std::string input("package \"FooPackage\"; concealed spec \"MySpec\"  }");

    LexerStringSource* source = new  LexerStringSource(input);
    Lexer lexer(*source);
    Parser parser(lexer);

    ASSERT_THROW(parser.Parse(), ParserMalformedSpecException);
  }

  // Case 5: missing closing brace.
  {
    std::string input("package \"FooPackage\"; concealed spec \"MySpec\" { ");

    LexerStringSource* source = new  LexerStringSource(input);
    Lexer lexer(*source);
    Parser parser(lexer);

    ASSERT_THROW(parser.Parse(), ParserMalformedSpecException);
  }
}

TEST(Parser, EmptySpec) {

  std::string input("package \"FooPackage\";"
                    "public spec MySpec { }"
                    "concealed spec Foo { }");

  LexerStringSource* source = new  LexerStringSource(input);
  Lexer lexer(*source);
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
}

TEST(Parser, ParseMalformedProperty) {

  // Case 1: leading access modifier.
  {
    std::string input("package \"FooPackage\";"
                      "public spec MySpec { "
                      "  concealed float X { public get; concealed set; }"
                      "}");

    LexerStringSource* source = new  LexerStringSource(input);
    Lexer lexer(*source);
    Parser parser(lexer);

    EXPECT_THROW(parser.Parse(), ParserMalformedPropertyException);
  }

  // Case 2: incorrect symbol type property name.
  {
    std::string input("package \"FooPackage\";"
                      "public spec MySpec { "
                      "  float 34 { public get; concealed set; }"
                      "}");

    LexerStringSource* source = new  LexerStringSource(input);
    Lexer lexer(*source);
    Parser parser(lexer);

    EXPECT_THROW(parser.Parse(), ParserMalformedPropertyException);
  }

  // Case 3: missing opening brace.
  {
    std::string input("package \"FooPackage\";"
                      "public spec MySpec { "
                      "  float X  public get; concealed set; }"
                      "}");

    LexerStringSource* source = new  LexerStringSource(input);
    Lexer lexer(*source);
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

    LexerStringSource* source = new  LexerStringSource(input);
    Lexer lexer(*source);
    Parser parser(lexer);

    EXPECT_THROW(parser.Parse(), ParserMalformedPropertyException);
  }

  // Case 5: missing property accessor/mutator access modifier.
  {
    std::string input("package \"FooPackage\";"
                      "public spec MySpec { "
                      "  float X { get; concealed set; }"
                      "}");

    LexerStringSource* source = new  LexerStringSource(input);
    Lexer lexer(*source);
    Parser parser(lexer);

    EXPECT_THROW(parser.Parse(), ParserMalformedPropertyException);
  }

  // Case 6: multiple get/set.
  {
    std::string input("package \"FooPackage\";"
                      "public spec MySpec { "
                      "  float X { public set; concealed set; }"
                      "}");

    LexerStringSource* source = new  LexerStringSource(input);
    Lexer lexer(*source);
    Parser parser(lexer);

    EXPECT_THROW(parser.Parse(), ParserMalformedPropertyException);
  }

  // Case 7: semicolon following property accessor body
  {
    std::string input("package \"FooPackage\";"
                      "public spec MySpec { "
                      "  float X { public get { }; concealed set; }"
                      "}");

    LexerStringSource* source = new  LexerStringSource(input);
    Lexer lexer(*source);
    Parser parser(lexer);

    // TODO: this exception may be incorrect.
    EXPECT_THROW(parser.Parse(), ParserUnexpectedTokenException);
  }
}

TEST(Parser, ParsePropertyEmpty) {

  std::string input("package \"FooPackage\";"
                    "public spec MySpec {"
                    "  int X { }"
                    "  string Y { }"
                    "}");

  LexerStringSource* source = new  LexerStringSource(input);
  Lexer lexer(*source);
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
}

TEST(Parser, ParsePropertyAuto) {

  // Case 1: get first, set second.
  {
    std::string input("package \"FooPackage\";"
                      "public spec MySpec { "
                      "  float X { public get; concealed set; }"
                      "}");

    LexerStringSource* source = new  LexerStringSource(input);
    Lexer lexer(*source);
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
  }

  // Case 2: get second, set first.
  {
    std::string input("package \"FooPackage\";"
                      "public spec MySpec { "
                      "  float X { concealed set; internal get; }"
                      "}");

    LexerStringSource* source = new  LexerStringSource(input);
    Lexer lexer(*source);
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

  LexerStringSource* source = new  LexerStringSource(input);
  Lexer lexer(*source);
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
}

TEST(Parser, ParseFunctionEmptyOneParameterNative) {

  std::string input("package \"FooPackage\";"
                    "public spec MySpec {"
                    "  concealed native string Foo2(int x);"
                    "}");

  LexerStringSource* source = new  LexerStringSource(input);
  Lexer lexer(*source);
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
}

TEST(Parser, ParseFunctionEmptyTwoParameterNative) {

  std::string input("package \"FooPackage\";"
                    "public spec MySpec {"
                    "  concealed native string Foo2(int x, string y);"
                    "}");

  LexerStringSource* source = new  LexerStringSource(input);
  Lexer lexer(*source);
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

  LexerStringSource* source = new  LexerStringSource(input);
  Lexer lexer(*source);
  Parser parser(lexer);

  parser.Parse();
}

TEST(Parser, ParseMalformedFunctions) {

  // Case 1: missing access modifier.
  {
    std::string input("package \"FooPackage\";"
                      "public spec MySpec {"
                      "  native string Foo2(int x, string y);"
                      "}");

    LexerStringSource* source = new  LexerStringSource(input);
    Lexer lexer(*source);
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

    LexerStringSource* source = new  LexerStringSource(input);
    Lexer lexer(*source);
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

    LexerStringSource* source = new  LexerStringSource(input);
    Lexer lexer(*source);
    Parser parser(lexer);

    EXPECT_THROW(parser.Parse(), ParserMalformedFunctionException);
  }

  // Case 4: extraneous comma.
  {
    std::string input("package \"FooPackage\";"
                      "public spec MySpec {"
                      "  public native string Foo(int x,);"
                      "}");

    LexerStringSource* source = new  LexerStringSource(input);
    Lexer lexer(*source);
    Parser parser(lexer);

    EXPECT_THROW(parser.Parse(), ParserMalformedFunctionException);
  }

  // Case 5: incorrect argument.
  {
    std::string input("package \"FooPackage\";"
                      "public spec MySpec {"
                      "  public native string Foo(public int x);"
                      "}");

    LexerStringSource* source = new  LexerStringSource(input);
    Lexer lexer(*source);
    Parser parser(lexer);

    EXPECT_THROW(parser.Parse(), ParserMalformedFunctionException);
  }

  // Case 6: missing comma.
  {
    std::string input("package \"FooPackage\";"
                      "public spec MySpec {"
                      "  public native string Foo(int x int y);"
                      "}");

    LexerStringSource* source = new  LexerStringSource(input);
    Lexer lexer(*source);
    Parser parser(lexer);

    EXPECT_THROW(parser.Parse(), ParserMalformedFunctionException);
  }

  // Case 7: missing LPAREN.
  {
    std::string input("package \"FooPackage\";"
                      "public spec MySpec {"
                      "  public native string Foo int x);"
                      "}");

    LexerStringSource* source = new  LexerStringSource(input);
    Lexer lexer(*source);
    Parser parser(lexer);

    EXPECT_THROW(parser.Parse(), ParserMalformedFunctionException);
  }

  // Case 8: missing RPAREN.
  {
    std::string input("package \"FooPackage\";"
                      "public spec MySpec {"
                      "  public native string Foo(int x ;"
                      "}");

    LexerStringSource* source = new  LexerStringSource(input);
    Lexer lexer(*source);
    Parser parser(lexer);

    EXPECT_THROW(parser.Parse(), ParserMalformedFunctionException);
  }

  // Case 8: missing SEMICOLON.
  {
    std::string input("package \"FooPackage\";"
                      "public spec MySpec {"
                      "  public native string Foo(int x)"
                      "}");

    LexerStringSource* source = new  LexerStringSource(input);
    Lexer lexer(*source);
    Parser parser(lexer);

    EXPECT_THROW(parser.Parse(), ParserMalformedBodyException);
  }

  // Case 8: missing body.
  {
    std::string input("package \"FooPackage\";"
                      "public spec MySpec {"
                      "  public native string Foo(int x)"
                      "}");

    LexerStringSource* source = new  LexerStringSource(input);
    Lexer lexer(*source);
    Parser parser(lexer);

    EXPECT_THROW(parser.Parse(), ParserMalformedBodyException);
  }

  // Case 9: missing brace.
  {
    std::string input("package \"FooPackage\";"
                      "public spec MySpec {"
                      "  public native string Foo(int x) }"
                      "}");

    LexerStringSource* source = new  LexerStringSource(input);
    Lexer lexer(*source);
    Parser parser(lexer);

    EXPECT_THROW(parser.Parse(), ParserMalformedBodyException);
  }
}

TEST(Parser, ParseFunctionWithBody) {
  // TODO: body parsing code and this test.
  FAIL();
}

} // namespace library
} // namespace gunderscript
