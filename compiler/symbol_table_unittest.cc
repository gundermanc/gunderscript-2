// Gunderscript 2 Symbol Table Unit Test
// (C) 2014-2015 Christian Gunderman

#include <string>

#include "gtest/gtest.h"
#include "testing_macros.h"

#include "symbol_table.h"

using namespace gunderscript;
using gunderscript::compiler::SymbolTable;

// Checks to make sure that all values are at correct defaults
// upon initialization.
TEST(SymbolTable, DefaultConditions) {
    SymbolTable<std::string> table;

    EXPECT_STATUS(table.Pop(), STATUS_SYMBOLTABLE_BOTTOM_OF_STACK);
    ASSERT_EQ(1, table.depth());
}

// Puts and gets items in a single level of the SymbolTable and
// checks to make sure that the correct values are read.
TEST(SymbolTable, SingleLevelPutGet) {
    SymbolTable<std::string> table;

    EXPECT_STATUS(table.Get("Item1"), STATUS_SYMBOLTABLE_UNDEFINED_SYMBOL);
    EXPECT_STATUS(table.GetTopOnly("Item1"), STATUS_SYMBOLTABLE_UNDEFINED_SYMBOL);
    ASSERT_EQ(1, table.depth());

    table.Put("Item1", "value1");

    ASSERT_STREQ("value1", table.Get("Item1").c_str());
    ASSERT_STREQ("value1", table.GetTopOnly("Item1").c_str());
    EXPECT_STATUS(table.Get("Item2"), STATUS_SYMBOLTABLE_UNDEFINED_SYMBOL);
    EXPECT_STATUS(table.GetTopOnly("Item2"), STATUS_SYMBOLTABLE_UNDEFINED_SYMBOL);
    ASSERT_EQ(1, table.depth());

    table.Put("Item2", "value2");
    ASSERT_STREQ("value1", table.Get("Item1").c_str());
    ASSERT_STREQ("value1", table.GetTopOnly("Item1").c_str());
    ASSERT_STREQ("value2", table.Get("Item2").c_str());
    ASSERT_STREQ("value2", table.GetTopOnly("Item2").c_str());
    ASSERT_EQ(1, table.depth());
}

// Tries to put duplicate values in single level of SymbolTable
// and checks for failure and correct value.
TEST(SymbolTable, SingleLevelPutDuplicate) {
    SymbolTable<std::string> table;

    EXPECT_STATUS(table.Get("Item1"), STATUS_SYMBOLTABLE_UNDEFINED_SYMBOL);

    table.Put("Item1", "value1");
    ASSERT_STREQ("value1", table.Get("Item1").c_str());

    EXPECT_STATUS(table.Put("Item1", "value2"), STATUS_SYMBOLTABLE_DUPLICATE_SYMBOL);
    ASSERT_STREQ("value1", table.Get("Item1").c_str());
}

// Checks to make sure values put in lower levels in the SymbolTable
// can still be accessed through the Get method.
TEST(SymbolTable, MultiLevelPutGet) {
    SymbolTable<std::string> table;

    EXPECT_STATUS(table.Get("Item1"), STATUS_SYMBOLTABLE_UNDEFINED_SYMBOL);
    EXPECT_STATUS(table.GetTopOnly("Item1"), STATUS_SYMBOLTABLE_UNDEFINED_SYMBOL);
    EXPECT_STATUS(table.Get("Item2"), STATUS_SYMBOLTABLE_UNDEFINED_SYMBOL);
    EXPECT_STATUS(table.GetTopOnly("Item2"), STATUS_SYMBOLTABLE_UNDEFINED_SYMBOL);
    EXPECT_STATUS(table.Get("Item3"), STATUS_SYMBOLTABLE_UNDEFINED_SYMBOL);
    EXPECT_STATUS(table.GetTopOnly("Item3"), STATUS_SYMBOLTABLE_UNDEFINED_SYMBOL);

    // Put Item1 and Item2, leaving Item3 undefined.
    table.Put("Item1", "value1");
    table.Put("Item2", "value2");

    // Add a level to the SymbolTable.
    table.Push();

    // Make sure the values haven't changed, except for GetTopOnly
    // since they are no longer stored in top.
    ASSERT_STREQ("value1", table.Get("Item1").c_str());
    EXPECT_STATUS(table.GetTopOnly("Item1"), STATUS_SYMBOLTABLE_UNDEFINED_SYMBOL);
    ASSERT_STREQ("value2", table.Get("Item2").c_str());
    EXPECT_STATUS(table.GetTopOnly("Item2"), STATUS_SYMBOLTABLE_UNDEFINED_SYMBOL);
    EXPECT_STATUS(table.Get("Item3"), STATUS_SYMBOLTABLE_UNDEFINED_SYMBOL);
    EXPECT_STATUS(table.GetTopOnly("Item3"), STATUS_SYMBOLTABLE_UNDEFINED_SYMBOL);

    // Set new value for Item1 and add Item3 in new level.
    table.Put("Item1", "value3");
    table.Put("Item3", "value4");

    // Make sure only Item1 values have changed.
    ASSERT_STREQ("value3", table.Get("Item1").c_str());
    ASSERT_STREQ("value3", table.GetTopOnly("Item1").c_str());
    ASSERT_STREQ("value2", table.Get("Item2").c_str());
    EXPECT_STATUS(table.GetTopOnly("Item2"), STATUS_SYMBOLTABLE_UNDEFINED_SYMBOL);
    ASSERT_STREQ("value4", table.Get("Item3").c_str());
    ASSERT_STREQ("value4", table.GetTopOnly("Item3").c_str());

    // Remove second level.
    table.Pop();

    // Check for values restored to where they were before.
    ASSERT_STREQ("value1", table.Get("Item1").c_str());
    ASSERT_STREQ("value1", table.GetTopOnly("Item1").c_str());
    ASSERT_STREQ("value2", table.Get("Item2").c_str());
    ASSERT_STREQ("value2", table.GetTopOnly("Item2").c_str());
    EXPECT_STATUS(table.Get("Item3"), STATUS_SYMBOLTABLE_UNDEFINED_SYMBOL);
    EXPECT_STATUS(table.GetTopOnly("Item3"), STATUS_SYMBOLTABLE_UNDEFINED_SYMBOL);
}

// Checks to make sure we can put an item in the bottom most level
// of symbol table with PutBottom.
TEST(SymbolTable, PutBottom) {
    SymbolTable<std::string> table;

    // Item 1.
    table.Put("Item1", "Value1");

    // Push second table and item 2.
    table.Push();
    table.Put("Item2", "Value2");

    // Push third table and item 3.
    table.Push();
    table.Put("Item3", "Value3");
    table.PutBottom("Item4", "Value4");

    // Check all items are in.
    ASSERT_STREQ("Value1", table.Get("Item1").c_str());
    ASSERT_STREQ("Value2", table.Get("Item2").c_str());
    ASSERT_STREQ("Value3", table.Get("Item3").c_str());
    ASSERT_STREQ("Value4", table.Get("Item4").c_str());

    // Pop table 3 and validate.
    table.Pop();
    ASSERT_STREQ("Value1", table.Get("Item1").c_str());
    ASSERT_STREQ("Value2", table.Get("Item2").c_str());
    EXPECT_STATUS(table.Get("Item3").c_str(), STATUS_SYMBOLTABLE_UNDEFINED_SYMBOL);
    ASSERT_STREQ("Value4", table.Get("Item4").c_str());

    // Pop table 2 and validate.
    table.Pop();
    ASSERT_STREQ("Value1", table.Get("Item1").c_str());
    EXPECT_STATUS(table.Get("Item2").c_str(), STATUS_SYMBOLTABLE_UNDEFINED_SYMBOL);
    EXPECT_STATUS(table.Get("Item3").c_str(), STATUS_SYMBOLTABLE_UNDEFINED_SYMBOL);
    ASSERT_STREQ("Value4", table.Get("Item4").c_str());

    EXPECT_STATUS(table.Pop(), STATUS_SYMBOLTABLE_BOTTOM_OF_STACK);
}