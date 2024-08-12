/**
 * @file cuComplexEx.h
 * @author kekesong (kekesong@tencent.com)
 * @brief 
 * @version 0.1
 * @date 2024-07-12
 * 
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 * 
 */
#ifndef CU_COMPLEXEX_H_
#define CU_COMPLEXEX_H_
#include <cuComplex.h>
#include <cuda.h>


#ifndef PI
#  define PI 3.1415926F
#endif
#ifndef E
#  define E 2.71828183F
#endif

__host__ __device__ static __inline__ cuComplex operator+(cuComplex a, cuComplex b) { return cuCaddf(a, b); }
__host__ __device__ static __inline__ cuComplex operator-(cuComplex a, cuComplex b) { return cuCsubf(a, b); }
__host__ __device__ static __inline__ cuComplex operator*(cuComplex a, cuComplex b) { return cuCmulf(a, b); }
__host__ __device__ static __inline__ cuComplex operator*=(cuComplex& a, cuComplex b) { return a = cuCmulf(a, b); }

__host__ __device__ static __inline__ cuComplex cuCexpf(cuComplex c) {
  float e = exp(c.x);
  return make_cuComplex(e * cosf(c.y), e * sinf(c.y));
}

__host__ __device__ static __inline__ cuComplex cuCpolf(cuComplex c) {
  float a = c.x;
  float b = c.y;
  float z = cuCabsf(c);
  float f = atan2(b, a);
  return make_cuComplex(z, f);
}
__host__ __device__ static __inline__ cuComplex cuCrecf(cuComplex c) {
  float z = fabsf(c.x);
  float f = c.y;
  float a = z * cosf(f);
  float b = z * sinf(f);
  return make_cuComplex(a, b);
}

__host__ __device__ static __inline__ cuComplex cuCpowf(cuComplex base, cuComplex exp) {
  float2 b = cuCpolf(base);
  float r = b.x;
  float f = b.y;
  float c = exp.x;
  float d = exp.y;
  float z = powf(r, c) * powf(E, -d * f);
  float fi = d * logf(r) + c * f;
  float2 rpol = make_cuComplex(z, fi);
  return cuCrecf(rpol);
}

__host__ __device__ static __inline__ float cuCabsf2(cuFloatComplex x) {
  float a = x.x * x.x + x.y * x.y;
  if (a > 0.0000001f) {
    return sqrtf(a);
  }
  return 0;
}

struct __device_builtin__ __builtin_align__(16) cuComplexVec3 {
  cuComplex x, y, z;

  __host__ __device__ cuComplexVec3() : x{0}, y{0}, z{0} {}
  __host__ __device__ cuComplexVec3(const float3& rhs) {
    x = make_cuComplex(rhs.x, 0);
    y = make_cuComplex(rhs.y, 0);
    z = make_cuComplex(rhs.z, 0);
  }
  __host__ __device__ cuComplexVec3 operator+(const cuComplexVec3& rhs) {
    cuComplexVec3 m;
    m.x = cuCaddf(x, rhs.x);
    m.y = cuCaddf(y, rhs.y);
    m.z = cuCaddf(z, rhs.z);
    return m;
  }
  __host__ __device__ cuComplexVec3 operator-() {
    cuComplexVec3 m;
    m.x.x = -x.x;
    m.x.y = -x.y;
    m.y.x = -y.x;
    m.y.y = -y.y;
    m.z.x = -z.x;
    m.z.y = -z.y;
    return m;
  }
  __host__ __device__ cuComplexVec3 operator*(const cuComplex& rhs) {
    cuComplexVec3 m;
    m.x = cuCmulf(x, rhs);
    m.y = cuCmulf(y, rhs);
    m.z = cuCmulf(z, rhs);
    return m;
  }
  __host__ __device__ cuComplexVec3 cross(const cuComplexVec3& rhs) {
    cuComplexVec3 m;
    m.x = cuCsubf(cuCmulf(y, rhs.z), cuCmulf(z, rhs.y));
    m.y = cuCsubf(cuCmulf(z, rhs.x), cuCmulf(x, rhs.z));
    m.z = cuCsubf(cuCmulf(x, rhs.y), cuCmulf(y, rhs.x));
    return m;
  }
  __host__ __device__ cuComplexVec3 cross(const float3& rhs) { return cross(cuComplexVec3(rhs)); }
  __host__ __device__ cuComplex dot(const cuComplexVec3 rhs) {
    return cuCaddf(cuCaddf(cuCmulf(x, rhs.x), cuCmulf(y, rhs.y)), cuCmulf(z, rhs.z));
  }
  __host__ __device__ cuComplex dot(const float3 rhs) { return dot(cuComplexVec3(rhs)); }
  __host__ __device__ cuComplex operator*(const cuComplexVec3 rhs) { return dot(rhs); }
  __host__ __device__ cuComplex operator*(const float3 rhs) { return dot(rhs); }
};

inline __host__ __device__ float3 operator-(float3& a) { return make_float3(-a.x, -a.y, -a.z); }

inline __host__ __device__ float3 operator+(float3 a, float b) { return make_float3(a.x + b, a.y + b, a.z + b); }

inline __host__ __device__ float3 operator+(float b, float3 a) { return make_float3(a.x + b, a.y + b, a.z + b); }

inline __host__ __device__ float3 operator-(float3 a, float3 b) { return make_float3(a.x - b.x, a.y - b.y, a.z - b.z); }
inline __host__ __device__ void operator-=(float3& a, float3 b) {
  a.x -= b.x;
  a.y -= b.y;
  a.z -= b.z;
}
inline __host__ __device__ float3 operator-(float3 a, float b) { return make_float3(a.x - b, a.y - b, a.z - b); }
inline __host__ __device__ float3 operator-(float b, float3 a) { return make_float3(b - a.x, b - a.y, b - a.z); }
inline __host__ __device__ void operator-=(float3& a, float b) {
  a.x -= b;
  a.y -= b;
  a.z -= b;
}

inline __host__ __device__ float operator*(float3 a, float3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
inline __host__ __device__ float3 operator*(float3 a, float b) { return make_float3(a.x * b, a.y * b, a.z * b); }
inline __host__ __device__ float3 operator*(float b, float3 a) { return make_float3(b * a.x, b * a.y, b * a.z); }
inline __host__ __device__ void operator*=(float3& a, float b) {
  a.x *= b;
  a.y *= b;
  a.z *= b;
}

inline __host__ __device__ void operator/=(float3& a, float3 b) {
  a.x /= b.x;
  a.y /= b.y;
  a.z /= b.z;
}
inline __host__ __device__ float3 operator/(float3 a, float b) { return make_float3(a.x / b, a.y / b, a.z / b); }
inline __host__ __device__ void operator/=(float3& a, float b) {
  a.x /= b;
  a.y /= b;
  a.z /= b;
}
inline __host__ __device__ float3 operator/(float b, float3 a) { return make_float3(b / a.x, b / a.y, b / a.z); }
//
//__host__ __device__ static __inline__  cuComplexVec3 cuCcrossf(const cuComplexVec3 lhs, const cuComplexVec3 rhs)
//{
//	cuComplexVec3 m;
//	m.x = cuCsubf(cuCmulf(lhs.y, rhs.z), cuCmulf(lhs.z, rhs.y));
//	m.y = cuCsubf(cuCmulf(lhs.z, rhs.x), cuCmulf(lhs.x, rhs.z));
//	m.z = cuCsubf(cuCmulf(lhs.x, rhs.y), cuCmulf(lhs.y, rhs.x));
//    return m;
//}
//
//__host__ __device__ static __inline__  cuComplex cuCdotf(const cuComplexVec3 lhs, const cuComplexVec3 rhs)
//{
//    return cuCaddf(cuCaddf(cuCmulf(lhs.x, rhs.x), cuCmulf(lhs.y, rhs.y)), cuCmulf(lhs.z, rhs.z));
//}

#endif
