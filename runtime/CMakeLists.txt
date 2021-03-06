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

include_directories(${gc_SOURCE_DIR}/include)

add_library (
    gunderscript_runtime
    garbage_collector.cc
    virtual_machine.cc)

target_link_libraries (gunderscript_runtime gunderscript_common gc-lib)

# Generate tests if option enabled.
if (gunderscript_runtime_tests)
    include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
    add_executable (
        gunderscript_runtime_tests
        control_flow_integrationtest.cc
        primitive_types_integrationtest.cc
        primitive_typecasts_integrationtest.cc
        spec_types_integrationtest.cc)
    target_link_libraries(gunderscript_runtime_tests
                          gunderscript_compiler
                          gunderscript_runtime
                          gtest
                          gtest_main)
endif ()