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
#define MAX_RAYNUM 320
#define MAX_BYTE 1472

#pragma pack(push)
#pragma pack(1)

struct iop_data
{
    uint8_t header[8];
    uint16_t id;
    uint16_t version;    // = 1
    uint16_t frequency;
    uint16_t maxDistance;
    uint16_t laserN;
    uint16_t azimuthResolution;
    uint16_t fovUp;
    uint16_t fovDown;
    uint16_t fovLeft;
    uint16_t fovRight;
    uint16_t reflectionMode;
    uint16_t blockNum;
    uint16_t pkgNum;
    uint8_t reserve[66];
    uint16_t angle[MAX_RAYNUM * 2];    //

    uint32_t byteLen;
};
struct cell_data
{
    uint16_t distance;
    uint8_t reflection;
};
struct block_data_header
{
    uint16_t timeoffset;
    uint16_t azimuth;
    //
};
struct utm_time
{
    uint8_t year;     // 0~255 corresponds year 2000 to year 2255.
    uint8_t month;    // 1~12 month
    uint8_t day;      // 1~31 day
    uint8_t hour;     // 0~23 hour
    uint8_t min;      // 0~59 min
    uint8_t sec;      // 0~59 sec
    uint16_t ms;      // 0~999
    uint16_t us;      // 0~999

    void set(std::time_t t);
};
struct pop_data
{
    uint8_t header[4];
    uint16_t psn;
    utm_time time;
    double X;
    double Y;
    double Z;
    float roll;
    float pitch;
    float yaw;
    uint8_t block[MAX_BYTE];

    //
    uint32_t byteLen;
    uint32_t blockSize;
};
#pragma pack(pop)

class UTLidar : public lidar::TraditionalLidar
{
public:
    UTLidar();
    ~UTLidar();

    virtual lidar::LidarType getType() const;
    virtual uint32_t getRaysNum() const;
    virtual float getHorizontalResolution() const;        // 水平分辨率 °
    virtual uint32_t getHorizontalScanMinUnit() const;    // 水平最小单元的扫描个数
    virtual float getRotationFrequency() const;
    virtual float getRange() const;
    virtual uint32_t getHorizontalScanCount() const;
    virtual float getHorizontalScanAngle(uint32_t pos) const;
    virtual lidar::ReturnMode getReturnMode() const;
    virtual std::pair<float, float> getYawPitchAngle(uint32_t pos, uint32_t r) const;
    virtual uint32_t package(const lidar_ptset& datas);

    virtual void setReturnMode(lidar::ReturnMode);
    virtual void setRange(float r);
    virtual void setRotationFrequency(float rf);
    virtual void setRaysNum(uint32_t rn);
    virtual void setreflectionMode(uint32_t mode);
    virtual void setFOV(float left, float right, float low, float up);
    virtual void setHorizontalResolution(float r);
    virtual bool loadInterReference(const std::string& dir);
    virtual bool setAngleFromString(const FString& str);
    virtual bool setIP(const FString& ip, const FString& port);
    virtual bool Init();

protected:
    iop_data _iop_data;
    pop_data _pop_data;
    std::vector<float> _horizontal_angles;
    std::vector<float> _vertical_angles;
    float _range = 200.f;
    float _frequency = 10.f;
    float _horizontal_resolution = 0.2f;
    uint32_t _ray_num = 128;
    float fov_left = 0, fov_right = 360;
    float fov_low = -30.f, fov_up = 20.f;
    int _reflection_mode = 0;

    FString _udp_IP;
    uint32 _udp_iop_port = 0;
    uint32 _udp_pop_port = 0;
    FSocket* SenderSocket = nullptr;
    TSharedPtr<FInternetAddr> RemoteAddr_iop;
    TSharedPtr<FInternetAddr> RemoteAddr_pop;
    std::time_t pretime = 0;

    struct Point3I
    {
        float x, y, z;
        float instensity;
    };
    FString savePath;
    TArray<Point3I> pcdPoints;
    bool SavePcd();
    void initPackage();
    void send_data(std::time_t t);
};

}    // namespace ulidar
