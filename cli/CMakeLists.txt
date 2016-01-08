#
# Gunderscript 2 Command Line Build File
# (C) 2015-2016 Christian Gunderman
#

include_directories(../include)
add_executable (gunderscript_cli debug.cc cli.cc entry.cc)
target_link_libraries(gunderscript_cli libgunderscript)
