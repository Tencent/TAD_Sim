#include "Lidar.h"
#include <algorithm>
#include <fstream>
#include <sstream>

namespace lidar
{

// 激光设备的半径
float TraditionalLidar::getLaserRadius() const
{
    return 0.0f;
}
// 激光设备的高度
float TraditionalLidar::getLaserHeight() const
{
    return 0.0f;
}
// 获取水平扫描刻度数
uint32_t TraditionalLidar::getHorizontalScanCount() const
{
    return std::floor(360.0f / getHorizontalResolution());
}

// 水平扫描总数
float TraditionalLidar::getHorizontalScanAngle(uint32_t pos) const
{
    // check(pos < getHorizontalScanCount());
    return pos * getHorizontalResolution();
}

}    // namespace lidar
