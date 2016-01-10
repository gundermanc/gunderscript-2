#
# Gunderscript 2 Library Build File
# (C) 2015-2016 Christian Gunderman
#

# Compile the Gunderscript Library.
include_directories(../include)
add_library (
    gunderscript
    lexer.cc
    compiler_source.cc
    symbol_table.cc
    node.cc
    parser.cc
    ast_walker.cc
    semantic_ast_walker.cc
    compiler.cc)

# Generate tests if option enabled.
if (gunderscript_tests)
    include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
    add_executable (
        gunderscript_tests
        lexer_unittest.cc
        symbol_table_unittest.cc
        parser_unittest.cc
        semantic_ast_walker_unittest.cc)
    target_link_libraries(gunderscript_tests gunderscript)
    target_link_libraries(gunderscript_tests gtest gtest_main)
endif ()