// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

// Check if the compiler is Microsoft Visual C++
#ifdef _MSC_VER

// Define the library name option
#  define MAPCOMMON_LIBNAME_OPT "libmapcommon"

// Define the platform option based on the architecture
#  if defined(_WIN64)
#    define MAPCOMMON_PLATFORM_OPT "x64"
#  else
#    define MAPCOMMON_PLATFORM_OPT "win32"
#  endif

// Define the compiler option based on the compiler version
#  if _MSC_VER < 1200
#    error "cannot support the version of compiler!"
#  elif _MSC_VER == 1200
#    define MAPCOMMON_COMPILER_OPT "vc60"
#  elif _MSC_VER == 1300
#    define MAPCOMMON_COMPILER_OPT "vc70"
#  elif _MSC_VER == 1310
#    define MAPCOMMON_COMPILER_OPT "vc71"
#  elif _MSC_VER == 1400
#    define MAPCOMMON_COMPILER_OPT "vc80"
#  elif _MSC_VER == 1500
#    define MAPCOMMON_COMPILER_OPT "vc90"
#  elif _MSC_VER == 1600
#    define MAPCOMMON_COMPILER_OPT "vc100"
#  elif _MSC_VER == 1700
#    define MAPCOMMON_COMPILER_OPT "vc110"
#  elif _MSC_VER == 1800
#    define MAPCOMMON_COMPILER_OPT "vc120"
#  elif _MSC_VER == 1900
#    define MAPCOMMON_COMPILER_OPT "vc130"
#  else
#    error "unknown compiler version!"
#  endif

// Define the runtime option based on the runtime library setting
#  if !defined(_DLL)
#    define MAPCOMMON_RUNTIME_OPT "s"
#  else
#    define MAPCOMMON_RUNTIME_OPT "r"
#  endif

// Define the debug option based on the debug setting
#  if defined(_DEBUG) || defined(DEBUG)
#    define MAPCOMMON_DEBUG_OPT "d"
#  else
#    define MAPCOMMON_DEBUG_OPT ""
#  endif

// Define the full library name based on the options defined above
#  define MAPCOMMON_LIB_FULLNAME                                                \
    MAPCOMMON_LIBNAME_OPT "_" MAPCOMMON_PLATFORM_OPT "_" MAPCOMMON_COMPILER_OPT \
                          "_" MAPCOMMON_RUNTIME_OPT MAPCOMMON_DEBUG_OPT ".lib"

// If the library name is defined, print a message and link the library
#  if defined(MAPCOMMON_LIB_FULLNAME)
#    pragma message("autolink mapcommon library file : " MAPCOMMON_LIB_FULLNAME)
#    pragma comment(lib, MAPCOMMON_LIB_FULLNAME)
#  else
#    error "cannot autolink mapcommon library file!"
#  endif

// If the compiler is not Microsoft Visual C++, print a message
#else
#  pragma message("---------------mapcommon lib : linux version---------------")
#endif  // _MSC_VER
