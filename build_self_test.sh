#!/bin/bash

#
# Gunderscript-2 Build and Self Test CI Script
# (C) 2016 Christian Gunderman
#

git submodule init
git submodule update
cmake .
make
./compiler/gunderscript_compiler_tests

# Kill build if test fails.
if [[ $? != 0 ]]; then
   exit 1
fi

./runtime/gunderscript_runtime_tests

# Kill build if test fails.
if [[ $? != 0 ]]; then
   exit 1
fi