// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

// Define a macro to disallow copying a class
#define DISALLOW_COPY(TypeName) TypeName(const TypeName&) = delete

// Define a macro to disallow assigning a class
#define DISALLOW_ASSIGN(TypeName) TypeName& operator=(const TypeName&) = delete

// Define a macro to disallow copying and assigning a class
#ifndef DISALLOW_COPY_AND_ASSIGN
#  define DISALLOW_COPY_AND_ASSIGN(TypeName) \
    DISALLOW_COPY(TypeName);                 \
    DISALLOW_ASSIGN(TypeName)
#endif

// Define a macro to export or import a class from a DLL
#ifndef TXSIMMAP_API
#  define TXSIMMAP_API
#else
#  ifdef TXSIMMAP_EXPORTS
#    if defined(_WIN32) || defined(__CYGWIN__)
#      ifdef __GNUC__
#        define TXSIMMAP_API __attribute__((dllexport))
#      else
#        define TXSIMMAP_API __declspec(dllexport)
#        pragma warning(disable : 4251)
#      endif
#    else
#      if __GNUC__ >= 4
#        define TXSIMMAP_API __attribute__((visibility("default")))
#      else
#        define TXSIMMAP_API
#      endif
#    endif
#  else
#    if defined(_WIN32) || defined(__CYGWIN__)
#      ifdef __GNUC__
#        define TXSIMMAP_API __attribute__((dllimport))
#      else
#        define TXSIMMAP_API __declspec(dllimport)
#        pragma warning(disable : 4251)
#      endif
#    else
#      define TXSIMMAP_API
#    endif
#  endif
#endif
