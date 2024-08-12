#ifndef DEF_H
#define DEF_H

#ifdef TXSIM_EXPORTS
#if defined(_WIN32) || defined(__CYGWIN__)
#ifdef __GNUC__
#define TXSIM_API __attribute__ ((dllexport))
#else
#define TXSIM_API __declspec(dllexport)
#endif
#else
#if __GNUC__ >= 4
#define TXSIM_API __attribute__ ((visibility ("default")))
#else
#define TXSIM_API
#endif
#endif
#else
#define TXSIM_API
#endif

#if defined(__CUDACC__) // NVCC
#define CUDA_ALIGN(n) __align__(n)
#elif defined(__GNUC__) // GCC
#define CUDA_ALIGN(n) __attribute__((aligned(n)))
#elif defined(_MSC_VER) // MSVC
#define CUDA_ALIGN(n) __declspec(align(n))
#else
#error "Please provide a definition for MY_ALIGN macro for your host compiler!"
#endif

#endif
