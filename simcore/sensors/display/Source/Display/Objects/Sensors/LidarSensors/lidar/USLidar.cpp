#include "USLidar.h"
#include <sstream>
#include <fstream>
#include <algorithm>
#include <ctime>

namespace ulidar
{

#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

USLidar::USLidar()
    : _range(200.f)
    , _frequency(10)
    , _hray_num(500)
    , _vray_num(16)
    , fov_low(15.f)
    , fov_up(20.f)
    , fov_left(60.f)
    , fov_right(60.f)
{
}

USLidar::~USLidar()
{
}

// frequency
float USLidar::getRotationFrequency() const
{
    return _frequency;
}

float USLidar::getRange() const
{
    return _range;
}

lidar::ReturnMode USLidar::getReturnMode() const
{
    return lidar::RT_Strongest;
}

lidar::LidarType USLidar::getType() const
{
    return lidar::LT_SUser;
}

std::pair<float, float> USLidar::getYawPitchAngle(uint32_t pos, uint32_t r) const
{
    return std::make_pair(_horizontal_angles.at(r), _vertical_angles.at(r));
}

uint32_t USLidar::package(const lidar_ptset& datas)
{
    return 0;
}

void USLidar::setRange(float r)
{
    _range = r;
}

void USLidar::setRotationFrequency(float rf)
{
    _frequency = rf;
}

void USLidar::setRaysNum(uint32_t horizontal, float vertical)
{
    _hray_num = horizontal;
    _vray_num = vertical;
}

void USLidar::setFOV(float low, float up, float left, float right)
{
    fov_low = low;
    fov_up = up;
    fov_left = left;
    fov_right = right;
}

bool USLidar::Init()
{
    if (_hray_num == 0 || _vray_num == 0)
    {
        return false;
    }
    _horizontal_angles.resize(_hray_num, (fov_left + fov_right) * 0.5f);
    _vertical_angles.resize(_vray_num, (fov_low + fov_up) * 0.5f);
    // 歙埱煦票
    if (_hray_num > 1)
    {
        for (uint32_t i = 0; i < _hray_num; i++)
        {
            _horizontal_angles.at(i) = -fov_left + i * (fov_right + fov_left) / (_hray_num - 1);
        }
    }
    if (_vray_num > 1)
    {
        for (uint32_t i = 0; i < _vray_num; i++)
        {
            _vertical_angles.at(i) = -fov_low + i * (fov_up + fov_low) / (_vray_num - 1);
        }
    }
    return true;
}

}    // namespace ulidar
