#pragma once
#include <vector>
#include <string>
#include <chrono>
#include <map>
#include <algorithm>
#include <ctime>
#include <array>

namespace lidar
{
using microsecond_time = std::chrono::time_point<std::chrono::system_clock, std::chrono::microseconds>;

enum LidarType
{
    LT_RSLiDAR16,
    LT_RSLiDAR32,
    LT_RSBpearl,
    LT_RSHelios,
    LT_RSRuby,
    LT_RSM1,
    LT_RSM1P,
    LT_HS32,
    LT_HS40,
    LT_HS64,
    LT_HS64QT,
    LT_HS120FT,
    LT_HS128,
    LT_HS128AT,
    LT_Velodyne32,
    LT_TUser,
    LT_SUser
};

enum ReturnMode
{
    RT_Dual = 0,
    RT_Strongest = 0x01,
    RT_Last = 0x02
};

class Lidar
{
public:
    Lidar() = default;
    virtual ~Lidar() = default;
    virtual LidarType getType() const = 0;
};

class TraditionalLidar : public Lidar
{
public:
    TraditionalLidar() = default;
    virtual ~TraditionalLidar() = default;

    int GetID()
    {
        return _id;
    }
    void SetID(int id)
    {
        _id = id;
    }

    virtual uint32_t getRaysNum() const = 0;
    virtual uint32_t getHorizontalScanMinUnit() const = 0;    // 水平最小单元的扫描个数
    virtual float getRotationFrequency() const = 0;
    virtual float getLaserRadius() const;
    virtual float getLaserHeight() const;
    virtual float getRange() const = 0;
    virtual ReturnMode getReturnMode() const = 0;
    virtual uint32_t getHorizontalScanCount() const;    // 水平扫描总数
    virtual float getHorizontalScanAngle(uint32_t pos) const;
    virtual uint32_t getReturnNum() const
    {
        return getReturnMode() == ReturnMode::RT_Dual ? 2 : 1;
    }
    virtual void setReturnMode(lidar::ReturnMode rm) = 0;

    virtual std::pair<float, float> getYawPitchAngle(uint32_t pos, uint32_t r) const = 0;

    struct lidar_point
    {
        float x = 0, y = 0, z = 0;    // 左手系，米
        float distance = 0;
        float instensity = 0;
        unsigned int tag_c = 0u;
        unsigned int tag_t = 0u;
        float norinter = 0;
    };

    struct channel_data
    {
        uint32_t hor_pos = 0;
        float r = 0, p = 0, y = 0;     //  todo
        uint32_t pn = 0;               //= getRaysNum * ( dual ? 2 : 1 )
        std::time_t utime = 0;         // 微秒
        double X = 0, Y = 0, Z = 0;    // todo
        float roll = 0, pitch = 0, yaw = 0;
        lidar_point* points = 0;
    };
    struct lidar_ptset
    {
        std::vector<channel_data> channels;
        std::vector<lidar_point> points;
    };

    virtual uint32_t package(const lidar_ptset& datas) = 0;

protected:
    virtual float getHorizontalResolution() const = 0;    // 水平分辨率 °

    int _id{0};
};

class SolidStateLidar : public Lidar
{
public:
    SolidStateLidar() = default;
    virtual ~SolidStateLidar() = default;

    virtual float getLaserRadius() const
    {
        return 0;
    }
    virtual float getLaserHeight() const
    {
        return 0;
    }
    virtual float getRange() const = 0;
    virtual ReturnMode getReturnMode() const = 0;

    struct lidar_point
    {
        float yaw;
        float pitch;
        float distance;
        float instensity;
    };

    struct channel_data
    {
        std::time_t utime;                  // 微秒
        std::vector<lidar_point> points;    // size = getRaysNum * ( dual ? 2 : 1 )
    };
    struct lidar_ptset
    {
        std::vector<channel_data> channels;
        std::vector<lidar_point> points;
    };

    virtual uint32_t package(const lidar_ptset& datas) = 0;
};

}    // namespace lidar
