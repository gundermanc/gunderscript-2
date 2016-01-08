#
# Gunderscript 2 Root Build File
# (C) 2015 Christian Gunderman
#

option(gunderscript_tests "Gunderscript Tests" ON)

cmake_minimum_required (VERSION 2.8.7)
project (GUNDERSCRIPT)

# Force static linkage of the runtime library so that we don't depend on
# MSVC DLLs on Windows. Also required for GoogleTest linkage.
include(cmake/msvc_macros.cmake)
configure_msvc_runtime ()

# Travis CI Compatible C++11 Support
# CXX_STANDARD proeprty doesn't work on Travis CI because
# Travis has horribly out of date CMAKE *grumble grumble*
include (CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG ("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG ("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if (COMPILER_SUPPORTS_CXX11)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif (COMPILER_SUPPORTS_CXX0X)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else ()
    message (STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif ()

# Import Google Test if option enabled.
if (gunderscript_tests)
    add_subdirectory (googletest)
endif ()
  
# Compile libgunderscript.
add_subdirectory (library)

# Compile command line components.
add_subdirectory (cli)
