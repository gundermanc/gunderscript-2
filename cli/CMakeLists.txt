#
# Gunderscript 2 Command Line Build File
# (C) 2015 Christian Gunderman
#

include_directories(../include)
add_executable (gunderscript_cli cli.cc entry.cc)
target_link_libraries(gunderscript_cli libgunderscript)
