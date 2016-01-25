#
# Gunderscript 2 Runtime Library Build File
# (C) 2016 Christian Gunderman
#

# Include the public and common includes.
include_directories(../include)
include_directories(../common)

# TODO: These paths are nasty. Refactor the nanojit library.
include_directories(
    ${nanojit_SOURCE_DIR}/nanojit
    ${nanojit_SOURCE_DIR}/lirasm/VMPI.nj
    ${nanojit_SOURCE_DIR})

add_library (
    gunderscript_runtime
    virtual_machine.cc)

target_link_libraries (gunderscript_runtime gunderscript_common)

# Generate tests if option enabled.
if (gunderscript_runtime_tests)
    include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
    add_executable (
        gunderscript_runtime_tests
        virtual_machine_unittest.cc)
    target_link_libraries(gunderscript_runtime_tests gunderscript_runtime)
    target_link_libraries(gunderscript_runtime_tests gtest gtest_main)
endif ()