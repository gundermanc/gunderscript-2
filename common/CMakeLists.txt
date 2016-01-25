#
# Gunderscript 2 Common Library Build File
# (C) 2016 Christian Gunderman
#

# Include public includes.
include_directories(../include)

# TODO: These paths are nasty. Refactor the nanojit library.
include_directories(
    ${nanojit_SOURCE_DIR}/nanojit
    ${nanojit_SOURCE_DIR}/lirasm/VMPI.nj
    ${nanojit_SOURCE_DIR})

add_library (
    gunderscript_common
    moduleimpl.cc
    module.cc)
target_link_libraries(gunderscript_common nanojit)