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
  ASSERT_EQ(2, root->child_count());

  Node* name_node = root->GetChild(0);
  EXPECT_EQ(NodeRule::NAME, name_node->rule());
  EXPECT_STREQ("FooPackage", name_node->string_value()->c_str());

  Node* depends_node = root->GetChild(1);
  EXPECT_EQ(NodeRule::DEPENDS, depends_node->rule());
  EXPECT_EQ(0, depends_node->child_count());
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
  ASSERT_EQ(2, root->child_count());

  Node* name_node = root->GetChild(0);
  EXPECT_EQ(NodeRule::NAME, name_node->rule());
  EXPECT_STREQ("Foo", name_node->string_value()->c_str());

  Node* depends_node = root->GetChild(1);
  EXPECT_EQ(NodeRule::DEPENDS, depends_node->rule());
  EXPECT_EQ(2, depends_node->child_count());

  Node* dependency_node_0 = depends_node->GetChild(0);
  EXPECT_EQ(NodeRule::NAME, dependency_node_0->rule());
  EXPECT_EQ(0, dependency_node_0->child_count());
  EXPECT_STREQ("Foo2", dependency_node_0->string_value()->c_str());

  Node* dependency_node_1 = depends_node->GetChild(1);
  EXPECT_EQ(NodeRule::NAME, dependency_node_1->rule());
  EXPECT_EQ(0, dependency_node_1->child_count());
  EXPECT_STREQ("Foo3", dependency_node_1->string_value()->c_str());
}

}
}
