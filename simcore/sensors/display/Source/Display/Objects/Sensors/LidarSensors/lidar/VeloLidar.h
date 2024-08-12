#pragma once
#include <string>
#include <vector>
#include <assert.h>
#include <memory>
#include <map>
#include "Lidar.h"
#include "Networking.h"

namespace vllidar
{

//
struct utm_time
{
    uint8_t year;     // 20 = 2020 year
    uint8_t month;    // 1~12 month
    uint8_t day;      // 1~31 day
    uint8_t hour;     // 0~23 hour
    uint8_t min;      // 0~59 min
    uint8_t sec;      // 0~59 sec
    uint32_t us;      // 0~999999

    void set_utc(uint8_t* data) const;
    void set_us(uint8_t* data) const;
};

class point_data32
{
public:
    static const uint32_t BLOCK_HEADER_SIZE = 4;
    static const uint32_t CHANNEL_SIZE = 3;
    static const uint32_t CHANNELS_PER_BLOCK = 32;
    static const uint32_t BLOCK_CHANNEL_SIZE = (CHANNEL_SIZE * CHANNELS_PER_BLOCK);
    static const uint32_t BLOCK_SIZE = (BLOCK_CHANNEL_SIZE + BLOCK_HEADER_SIZE);    // 100
    static const uint32_t BLOCKS_PER_PACKAGE = 12;
    static const uint32_t PACKAGE_DATA_SIZE = (BLOCKS_PER_PACKAGE * BLOCK_SIZE);    // 1200
    static const uint32_t PTS_TAIL_SIZE = 6;
    static const uint32_t PTS_SIZE = (PACKAGE_DATA_SIZE + PTS_TAIL_SIZE);
    static const uint32_t PTS_TOTAL_SIZE = PTS_SIZE;    //

    static const uint32_t GPS_SIZE = 512;

    point_data32();
    virtual ~point_data32();
    const uint8_t* pt_data() const;
    const uint8_t* gps_data() const;

    void set_block_azimuth(uint8_t bn, uint16_t azimuth);
    void set_channel_data(uint8_t bn, uint8_t cn, uint16_t distance, uint8_t reflectivity);
    void set_return_mode(uint8_t return_mode);
    uint8_t get_return_mode() const;

    virtual void set_time(const utm_time& utime);

protected:
    uint8_t _data[PTS_TOTAL_SIZE];
    uint8_t _GPS[GPS_SIZE];
    uint8_t* _channel_data_ptr[BLOCKS_PER_PACKAGE][CHANNELS_PER_BLOCK];
    uint8_t* _tail_ptr;
};

class VeloLidar32 : public lidar::TraditionalLidar
{
public:
    VeloLidar32();
    ~VeloLidar32();

    int GetID()
    {
        return _id;
    }
    void SetID(int id);
    virtual lidar::LidarType getType() const
    {
        return lidar::LT_Velodyne32;
    }
    virtual uint32_t getRaysNum() const
    {
        return point_data32::CHANNELS_PER_BLOCK;
    }
    virtual float getRotationFrequency() const;
    virtual float getRange() const;
    virtual lidar::ReturnMode getReturnMode() const;
    virtual float getHorizontalResolution() const;
    virtual std::pair<float, float> getYawPitchAngle(uint32_t pos, uint32_t r) const;
    virtual float getLaserRadius() const;
    virtual float getLaserHeight() const;

    virtual void setRange(float r);
    // 5hz 10hz 20hz
    virtual void setRotationFrequency(float rf);
    virtual void setReturnMode(lidar::ReturnMode rm);
    virtual uint32_t getHorizontalScanMinUnit() const;

    virtual bool loadInterReference(const std::string& dir);
    virtual bool setAngleFromString(const FString& str);
    virtual bool setIP(const FString& ip, const FString& port);
    virtual bool Init();
    virtual uint32_t package(const lidar_ptset& datas);

    const std::string& error()
    {
        return _error;
    }

protected:
    int _id{0};
    std::shared_ptr<point_data32> _pt_data;
    std::vector<float> _horizontal_angles;
    std::vector<float> _vertical_angles;
    float _range;
    float _horizontal_resolution;
    std::string _udp_IP;
    uint32 _udp_pt_port = 0;
    uint32 _udp_gps_port = 0;

    std::string _error;

    // udp
    FSocket* SenderSocket = nullptr;
    TSharedPtr<FInternetAddr> RemoteAddr_msop;
    TSharedPtr<FInternetAddr> RemoteAddr_difop;
    std::time_t pretime;

    utm_time to_utime(std::time_t t) const;
    bool readini(const std::string& ini, std::map<std::string, std::map<std::string, std::string>>& data);
    virtual void send_data(std::time_t t);
    void split_string(const std::string& s, const std::string& delim, std::vector<std::string>& ret);
};

}    // namespace vllidar
