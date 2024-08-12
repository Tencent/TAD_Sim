#pragma once

#include "CoreMinimal.h"

// #include "SharedMemoryWriter.generated.h"

#if defined(_WIN32) || defined(__CYGWIN__)
#ifdef __GNUC__
#define DLL_EXPORT __attribute__ ((dllexport))
#else
#define DLL_EXPORT __declspec(dllexport)
#endif
#else
#if __GNUC__ >= 4
#define DLL_EXPORT __attribute__ ((visibility ("default")))
#else
#define DLL_EXPORT
#endif
#endif

namespace ueboost
{

float DLL_EXPORT bessel_j(int, float);
unsigned int DLL_EXPORT crc(const void*, int len);
}
