#ifndef _TERRAIN_PRE_DEFINE_H_
#define _TERRAIN_PRE_DEFINE_H_

#include <memory>

#ifdef _WIN32
#  ifdef BUILD_TERRAIN_DLL
#    define MODULE_API __declspec(dllexport)
#  else
#    define MODULE_API __declspec(dllimport)
#  endif  // BUILD_TERRAIN_DLL
#else
#  define MODULE_API
#endif

#ifndef _M_PI_
#  define _M_PI_ 3.141592657
#endif

#ifndef real_t
#  define real_t double
#endif

namespace terrain {
// vector3d
struct MODULE_API Vector3d {
  real_t x;
  real_t y;
  real_t z;
};

// terrain hadmap
class CTerrainHDMap;
using TerrainMapPtr = CTerrainHDMap*;

// async task
class CAsyncTask;
using CAsyncTaskPtr = std::unique_ptr<CAsyncTask>;
}  // namespace terrain

#endif
