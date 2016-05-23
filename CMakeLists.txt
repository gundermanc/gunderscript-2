#
# Gunderscript 2 Root Build File
# (C) 2015 Christian Gunderman
#

option(gunderscript_cli "Gunderscript Command Line Application" ON)
option(gunderscript_compiler "Gunderscript Compiler Library" ON)
option(gunderscript_compiler_tests "Gunderscript Compiler Library Tests" ON)
option(gunderscript_runtime_tests "Gunderscript Runtime Library Tests" ON)
option(gunderscript_force_define_debug "Force define _DEBUG for working around CMAKE_BUILD_TYPE not defined on Windows" OFF)

cmake_minimum_required (VERSION 2.8.7)
project (GUNDERSCRIPT)

# Force static linkage of the runtime library so that we don't depend on
# MSVC DLLs on Windows. Also required for GoogleTest linkage.
include(cmake/msvc_macros.cmake)
configure_msvc_runtime ()

# Disable C asserts in non-debug mode.
if (${CMAKE_BUILD_TYPE} NOT MATCHES "Debug")
    add_definitions(-DNDEBUG)
endif ()

# Travis CI Compatible C++11 Support
# CXX_STANDARD property doesn't work on Travis CI because
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

# Forces definition of _DEBUG to force inclusion of DEBUG specific
# code. This is useful for two reasons:
# - RELEASE code with DEBUG symbols
# - CMAKE for Visual Studio does not define CMAKE_BUILD_TYPE.
#   We can use this option to define it manually.
if (gunderscript_force_define_debug)
    set (CMAKE_BUILD_TYPE "Debug")
endif ()

# Print build config.
if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
    message ("Gunderscript DEBUG Configuration")
else ()
    message ("Gunderscript RELEASE Configuration")
endif ()

# TODO: this is pretty jank, reconsider.
# Define platform constants for NanoJIT
include(nanojit/cmake_macros.cmake)
define_nanojit_platform()

# Import Google Test if option enabled.
if (gunderscript_compiler_tests)
    add_subdirectory (googletest)
endif ()

# Compile NanoJIT dependency.
add_subdirectory (nanojit)

# Compile GC dependency.
set(enable_cplusplus ON CACHE BOOL "Enable GC C++ Support")
add_subdirectory (bdwgc)

# Compile libgunderscript common library.
add_subdirectory (common)

# Compile libgunderscript compiler if option enabled.
if (gunderscript_compiler)
    add_subdirectory (compiler)
endif ()

# Compile libgunderscript runtime if option enabled.
add_subdirectory (runtime)

# Compile command line components if option enabled.
if (gunderscript_cli)
    add_subdirectory (cli)
endif ()
