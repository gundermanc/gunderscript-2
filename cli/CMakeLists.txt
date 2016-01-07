#
# Gunderscript 2 Command Line Build File
# (C) 2015 Christian Gunderman
#

include_directories(../library)
add_executable (gunderscript_cli cli.cc entry.cc)
target_link_libraries(gunderscript_cli libgunderscript)

# Require C++ 11 Standard
set_property (TARGET gunderscript_cli PROPERTY CXX_STANDARD 11)
set_property (TARGET gunderscript_cli PROPERTY CXX_STANDARD_REQUIRED ON)