#
# Gunderscript 2 Root Build File
# (C) 2015 Christian Gunderman
#

option(libgunderscript_tests "Gunderscript Tests" ON)

cmake_minimum_required (VERSION 3.2.2)
project (GUNDERSCRIPT)

# Force static linkage of the runtime library so that we don't depend on
# MSVC DLLs on Windows. Also required for GoogleTest linkage.
include(cmake/msvc_macros.cmake)
configure_msvc_runtime ()

# Compile libgunderscript.
add_subdirectory (library)

# Import Google Test if option enabled.
if (libgunderscript_tests)
    add_subdirectory (googletest)
endif ()

# Compile command line components.
add_subdirectory (cli)
