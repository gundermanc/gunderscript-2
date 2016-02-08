#!/bin/sh

#
# Gunderscript-2 Build and Self Test CI Script
# (C) 2016 Christian Gunderman
#

git submodule init
git submodule update
cmake .
make
./compiler/gunderscript_compiler_tests
./runtime/gunderscript_runtime_tests