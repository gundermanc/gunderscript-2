// Gunderscript 2 Symbol Table Unit Test
// (C) 2014 Christian Gunderman

#include <string>

#include "gtest/gtest.h"

#include "symbol_table.h"

using gunderscript::library::SymbolTable;

// Checks to make sure that all values are at correct defaults
// upon initialization.
TEST(SymbolTable, DefaultConditions) {
  SymbolTable<std::string> table;

  ASSERT_FALSE(table.Pop());
  ASSERT_EQ(1, table.depth());
}

// Puts and gets items in a single level of the SymbolTable and
// checks to make sure that the correct values are read.
TEST(SymbolTable, SingleLevelPutGet) {
  SymbolTable<std::string> table;

  ASSERT_EQ(NULL, table.Get("Item1"));
  ASSERT_EQ(NULL, table.GetTopOnly("Item1"));
  ASSERT_EQ(1, table.depth());

  ASSERT_TRUE(table.Put("Item1", "value1"));

  ASSERT_STREQ("value1", table.Get("Item1")->c_str());
  ASSERT_STREQ("value1", table.GetTopOnly("Item1")->c_str());
  ASSERT_EQ(NULL, table.Get("Item2"));
  ASSERT_EQ(NULL, table.GetTopOnly("Item2"));
  ASSERT_EQ(1, table.depth());

  ASSERT_TRUE(table.Put("Item2", "value2"));
  ASSERT_STREQ("value1", table.Get("Item1")->c_str());
  ASSERT_STREQ("value1", table.GetTopOnly("Item1")->c_str());
  ASSERT_STREQ("value2", table.Get("Item2")->c_str());
  ASSERT_STREQ("value2", table.GetTopOnly("Item2")->c_str());
  ASSERT_EQ(1, table.depth());
}

// Tries to put duplicate values in single level of SymbolTable
// and checks for failure and correct value.
TEST(SymbolTable, SingleLevelPutDuplicate) {
  SymbolTable<std::string> table;

  ASSERT_EQ(NULL, table.Get("Item1"));

  ASSERT_TRUE(table.Put("Item1", "value1"));
  ASSERT_STREQ("value1", table.Get("Item1")->c_str());

  ASSERT_FALSE(table.Put("Item1", "value2"));
  ASSERT_STREQ("value1", table.Get("Item1")->c_str());
}

// Checks to make sure values put in lower levels in the SymbolTable
// can still be accessed through the Get method.
TEST(SymbolTable, MultiLevelPutGet) {
  SymbolTable<std::string> table;

  ASSERT_EQ(NULL, table.Get("Item1"));
  ASSERT_EQ(NULL, table.GetTopOnly("Item1"));
  ASSERT_EQ(NULL, table.Get("Item2"));
  ASSERT_EQ(NULL, table.GetTopOnly("Item2"));
  ASSERT_EQ(NULL, table.Get("Item3"));
  ASSERT_EQ(NULL, table.GetTopOnly("Item3"));

  // Put Item1 and Item2, leaving Item3 undefined.
  ASSERT_TRUE(table.Put("Item1", "value1"));
  ASSERT_TRUE(table.Put("Item2", "value2"));

  // Add a level to the SymbolTable.
  table.Push();

  // Make sure the values haven't changed, except for GetTopOnly
  // since they are no longer stored in top.
  ASSERT_STREQ("value1", table.Get("Item1")->c_str());
  ASSERT_EQ(NULL, table.GetTopOnly("Item1"));
  ASSERT_STREQ("value2", table.Get("Item2")->c_str());
  ASSERT_EQ(NULL, table.GetTopOnly("Item2"));
  ASSERT_EQ(NULL, table.Get("Item3"));
  ASSERT_EQ(NULL, table.GetTopOnly("Item3"));
  
  // Set new value for Item1 and add Item3 in new level.
  ASSERT_TRUE(table.Put("Item1", "value3"));
  ASSERT_TRUE(table.Put("Item3", "value4"));

  // Make sure only Item1 values have changed.
  ASSERT_STREQ("value3", table.Get("Item1")->c_str());
  ASSERT_STREQ("value3", table.GetTopOnly("Item1")->c_str());
  ASSERT_STREQ("value2", table.Get("Item2")->c_str());
  ASSERT_EQ(NULL, table.GetTopOnly("Item2"));
  ASSERT_STREQ("value4", table.Get("Item3")->c_str());
  ASSERT_STREQ("value4", table.GetTopOnly("Item3")->c_str());

  // Remove second level.
  ASSERT_TRUE(table.Pop());

  // Check for values restored to where they were before.
  ASSERT_STREQ("value1", table.Get("Item1")->c_str());
  ASSERT_STREQ("value1", table.GetTopOnly("Item1")->c_str());
  ASSERT_STREQ("value2", table.Get("Item2")->c_str());
  ASSERT_STREQ("value2", table.GetTopOnly("Item2")->c_str());
  ASSERT_EQ(NULL, table.Get("Item3"));
  ASSERT_EQ(NULL, table.GetTopOnly("Item3"));
}
