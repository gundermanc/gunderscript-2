#
# Gunderscript 2 Runtime Library Build File
# (C) 2016 Christian Gunderman
#

# Compile the Gunderscript Library.
include_directories(../include)

add_library (
    gunderscript_runtime
    virtual_machine.cc)

# Generate tests if option enabled.
if (gunderscript_runtime_tests)
    include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
    add_executable (
        gunderscript_runtime_tests
        virtual_machine_unittest.cc)
    target_link_libraries(gunderscript_runtime_tests gunderscript_runtime)
    target_link_libraries(gunderscript_runtime_tests gtest gtest_main)
endif ()