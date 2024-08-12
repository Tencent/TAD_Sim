#pragma once

#ifndef TXSIM_NO_EXPORTS
#  if defined(_WIN32) || defined(__CYGWIN__)
#    ifdef __GNUC__
#      define TXSIM_API __attribute__((dllexport))
#    else
#      define TXSIM_API __declspec(dllexport)
#    endif
#  else
#    if __GNUC__ >= 4
#      define TXSIM_API __attribute__((visibility("default")))
#    else
#      define TXSIM_API
#    endif
#  endif
#else
#  define TXSIM_API
#endif
