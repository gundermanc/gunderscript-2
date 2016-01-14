#
# Gunderscript 2 Compiler Library Build File
# (C) 2015-2016 Christian Gunderman
#

# Compile the Gunderscript Library.
include_directories(../include)

# TODO: These paths are nasty. Refactor the nanojit library.
include_directories(
    ${nanojit_SOURCE_DIR}/nanojit
    ${nanojit_SOURCE_DIR}/lirasm/VMPI.nj
    ${nanojit_SOURCE_DIR})

add_library (
    gunderscript_compiler
    lexer.cc
    compiler_source.cc
    symbol_table.cc
    node.cc
    parser.cc
    ast_walker.cc
    semantic_ast_walker.cc
    lirgen_ast_walker.cc
    compiler.cc)
target_link_libraries(gunderscript_compiler nanojit)

# Generate tests if option enabled.
if (gunderscript_compiler_tests)
    include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
    add_executable (
        gunderscript_compiler_tests
        lexer_unittest.cc
        symbol_table_unittest.cc
        parser_unittest.cc
        semantic_ast_walker_unittest.cc)
    target_link_libraries(gunderscript_compiler_tests gunderscript_compiler)
    target_link_libraries(gunderscript_compiler_tests gtest gtest_main)
endif ()