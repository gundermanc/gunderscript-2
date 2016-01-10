#!/bin/sh

#
# Gunderscript-2 Build and Self Test CI Script
# (C) 2016 Christian Gunderman
#

git submodule init
git submodule update
cmake .
make
./library/gunderscript_tests
