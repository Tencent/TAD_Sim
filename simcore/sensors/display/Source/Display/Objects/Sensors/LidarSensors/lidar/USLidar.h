#pragma once
#include <string>
#include <vector>
#include <assert.h>
#include <memory>
#include <map>
#include "Lidar.h"
#include "Networking.h"

namespace ulidar
{

class USLidar : public lidar::SolidStateLidar
{
public:
    USLidar();
    ~USLidar();

    virtual lidar::LidarType getType() const;
    virtual float getRotationFrequency() const;
    virtual float getRange() const;
    virtual lidar::ReturnMode getReturnMode() const;
    virtual std::pair<float, float> getYawPitchAngle(uint32_t pos, uint32_t r) const;
    virtual uint32_t package(const lidar_ptset& datas);

    virtual void setRange(float r);
    virtual void setRotationFrequency(float rf);
    virtual void setRaysNum(uint32_t horizontal, float vertical);
    virtual void setFOV(float low, float up, float left, float right);

    virtual bool Init();

    const std::string& error()
    {
        return _error;
    }

protected:
    std::vector<float> _horizontal_angles;
    std::vector<float> _vertical_angles;
    float _range;
    float _frequency;
    uint32_t _hray_num;
    uint32_t _vray_num;
    float fov_low, fov_up, fov_left, fov_right;

    std::string _error;
};

}    // namespace ulidar
