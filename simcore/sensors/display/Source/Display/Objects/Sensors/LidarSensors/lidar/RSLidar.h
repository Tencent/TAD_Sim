#pragma once
#include <string>
#include <vector>
#include <assert.h>
#include <memory>
#include <map>
#include "Lidar.h"
#include "Networking.h"
#include <ctime>

namespace rslidar
{

//
struct utc_time
{
    std::time_t t;
    int micro;
    void set(uint8_t* data) const;
};
struct utm_time
{
    utm_time(const utc_time& t);

    uint8_t year;     // 0~255 corresponds year 2000 to year 2255.
    uint8_t month;    // 1~12 month
    uint8_t day;      // 1~31 day
    uint8_t hour;     // 0~23 hour
    uint8_t min;      // 0~59 min
    uint8_t sec;      // 0~59 sec
    uint16_t ms;      // 0~999
    uint16_t us;      // 0~999

    void set(uint8_t* data) const;
};

enum dis_resolution_mode
{
    cm_05 = 0,    // 0.5cm
    cm_10 = 1     // 1cm
};

struct msop_data
{
    msop_data();
    virtual ~msop_data() = default;
    virtual const uint8_t* data() const = 0;
    virtual uint32_t length() const = 0;

    virtual void set_header_time(const utc_time& utime) = 0;
    virtual void set_temperature(float temp) = 0;
    virtual void set_block_azimuth(uint8_t bn, uint16_t azimuth) = 0;
    virtual void set_channel_data(uint8_t bn, uint8_t cn, uint16_t distance, uint8_t reflectivity) = 0;
};
struct msop_data16 : msop_data
{
    msop_data16();

    static const uint32_t BLOCK_HEADER_SIZE = 4;
    static const uint32_t CHANNEL_SIZE = 3;
    static const uint32_t CHANNELS_PER_BLOCK = 32;
    static const uint32_t BLOCK_CHANNEL_SIZE = (CHANNEL_SIZE * CHANNELS_PER_BLOCK);    // 96
    static const uint32_t BLOCK_SIZE = (BLOCK_CHANNEL_SIZE + BLOCK_HEADER_SIZE);       // 100
    static const uint32_t BLOCKS_PER_MSOP = 12;
    static const uint32_t MSOP_HEADER_SIZE = 42;
    static const uint32_t MSOP_DATA_SIZE = (BLOCKS_PER_MSOP * BLOCK_SIZE);    // 1200
    static const uint32_t MSOP_TAIL_SIZE = 6;
    static const uint32_t MSOP_SIZE = (MSOP_HEADER_SIZE + MSOP_DATA_SIZE + MSOP_TAIL_SIZE);

    void copy(void* d) const;
    virtual const uint8_t* data() const;
    virtual uint32_t length() const
    {
        return MSOP_SIZE;
    }
    virtual void set_header_time(const utc_time& utime);
    virtual void set_temperature(float temp);
    virtual void set_block_azimuth(uint8_t bn, uint16_t azimuth);
    virtual void set_channel_data(uint8_t bn, uint8_t cn, uint16_t distance, uint8_t reflectivity);

protected:
    uint8_t _data[MSOP_SIZE + 100] = {0};
    uint8_t* _channel_data_ptr[BLOCKS_PER_MSOP][CHANNELS_PER_BLOCK];
};
struct msop_data32 : msop_data16
{
    msop_data32();
};

struct msop_data_bpearl : msop_data16
{
    msop_data_bpearl();
};

struct msop_data_helios : msop_data_bpearl
{
    msop_data_helios();

    virtual void set_header_time(const utc_time& utime);
};

struct msop_data_ruby : msop_data
{
    static const uint32_t BLOCK_HEADER_SIZE = 4;
    static const uint32_t CHANNEL_SIZE = 3;
    static const uint32_t CHANNELS_PER_BLOCK = 128;
    static const uint32_t BLOCK_CHANNEL_SIZE = (CHANNEL_SIZE * CHANNELS_PER_BLOCK);    // 384
    static const uint32_t BLOCK_SIZE = (BLOCK_CHANNEL_SIZE + BLOCK_HEADER_SIZE);       // 388
    static const uint32_t BLOCKS_PER_MSOP = 3;
    static const uint32_t MSOP_HEADER_SIZE = 80;
    static const uint32_t MSOP_DATA_SIZE = (BLOCKS_PER_MSOP * BLOCK_SIZE);    // 1164
    static const uint32_t MSOP_TAIL_SIZE = 4;
    static const uint32_t MSOP_SIZE = (MSOP_HEADER_SIZE + MSOP_DATA_SIZE + MSOP_TAIL_SIZE);

    msop_data_ruby();
    void copy(void* d) const;
    virtual const uint8_t* data() const;
    virtual uint32_t length() const
    {
        return MSOP_SIZE;
    }
    virtual void set_header_time(const utc_time& utime);
    virtual void set_temperature(float temp);
    virtual void set_block_azimuth(uint8_t bn, uint16_t azimuth);
    virtual void set_channel_data(uint8_t bn, uint8_t cn, uint16_t distance, uint8_t reflectivity);

protected:
    uint8_t _data[MSOP_SIZE + 100] = {0};
    uint8_t* _channel_data_ptr[BLOCKS_PER_MSOP][CHANNELS_PER_BLOCK];
};

struct msop_data_ss
{
    msop_data_ss() = default;
    virtual ~msop_data_ss() = default;
    virtual const uint8_t* data() const = 0;
    virtual uint32_t length() const = 0;

    virtual void set_header_time(const utc_time& utime, uint32_t psn) = 0;
    virtual void set_temperature(float temp) = 0;
    virtual void set_block_azimuth(uint8_t bn, uint8_t toffset, uint8_t ret_seq) = 0;
    virtual void set_channel_data(
        uint8_t bn, uint8_t cn, uint16_t pitch, uint16_t yaw, uint16_t distance, uint8_t reflectivity) = 0;

    virtual void set_return_mode(uint8_t return_mode) = 0;
    virtual uint8_t get_return_mode() const = 0;
};
struct msop_data_m1 : msop_data_ss
{
    static const uint32_t BLOCK_HEADER_SIZE = 2;
    static const uint32_t CHANNEL_SIZE = 9;
    static const uint32_t CHANNELS_PER_BLOCK = 5;
    static const uint32_t BLOCK_CHANNEL_SIZE = (CHANNEL_SIZE * CHANNELS_PER_BLOCK);    //
    static const uint32_t BLOCK_SIZE = (BLOCK_CHANNEL_SIZE + BLOCK_HEADER_SIZE);       //
    static const uint32_t BLOCKS_PER_MSOP = 25;
    static const uint32_t MSOP_HEADER_SIZE = 32;
    static const uint32_t MSOP_DATA_SIZE = (BLOCKS_PER_MSOP * BLOCK_SIZE);    //
    static const uint32_t MSOP_TAIL_SIZE = 3;
    static const uint32_t MSOP_SIZE = (MSOP_HEADER_SIZE + MSOP_DATA_SIZE + MSOP_TAIL_SIZE);

    msop_data_m1();
    void copy(void* d) const;
    virtual const uint8_t* data() const;
    virtual uint32_t length() const
    {
        return MSOP_SIZE;
    }
    virtual void set_header_time(const utc_time& utime, uint32_t psn);
    virtual void set_return_mode(uint8_t return_mode);
    virtual uint8_t get_return_mode() const;
    virtual void set_temperature(float temp);
    virtual void set_block_azimuth(uint8_t bn, uint8_t toffset, uint8_t ret_seq);
    virtual void set_channel_data(
        uint8_t bn, uint8_t cn, uint16_t pitch, uint16_t yaw, uint16_t distance, uint8_t reflectivity);

protected:
    uint8_t _data[MSOP_SIZE + 100] = {0};
    uint8_t* _channel_data_ptr[BLOCKS_PER_MSOP][CHANNELS_PER_BLOCK];
};

struct difop_data
{
    static const uint32_t DIFOP_HEADER_SIZE = 8;
    static const uint32_t DIFOP_DATA_SIZE = 1238;
    static const uint32_t DIFOP_TAIL_SIZE = 2;
    static const uint32_t DIFOP_SIZE = (DIFOP_HEADER_SIZE + DIFOP_DATA_SIZE + DIFOP_TAIL_SIZE);

    difop_data();
    virtual ~difop_data() = default;

    void copy(void* d) const;
    const uint8_t* data() const;

    ////1200,600,300;clockwise rotation;
    void set_mot_spd(uint16_t mot_spd);
    uint16_t get_mot_spd() const;

    // eth
    void set_eth(
        uint8_t lidar_ip[4], uint8_t dest_pc_ip[4], uint8_t mac_addr[6], uint16_t msop_port, uint16_t difop_port);

    // du,*100
    void set_hor_fov(uint16_t fov_start, uint16_t fov_end);
    void set_mot_phase(uint16_t mot_phase);
    void set_frm(uint8_t top_frm[5], uint8_t bot_frm[5]);
    void set_intensity(uint8_t intensity_scale, uint8_t intensity_mode /*1,2,3*/);

    uint8_t get_intensity_mode() const;
    dis_resolution_mode get_dis_resolution_mode() const;

    void set_sn(uint8_t sn[6]);
    void set_zero_angle_offset(uint16_t zero_angle_offset);
    virtual void set_return_mode(uint8_t return_mode);
    virtual uint8_t get_return_mode() const;

    // Upper computer compatibility
    void set_upper_cc(uint16_t upper_cc);
    // Operation status 未完成
    // Fault diagnosis 未完成

    void set_gprmc(const char* gpr);
    virtual void set_time(const utc_time& utime);

    virtual void set_cor_vert_ang(const std::vector<float> angles)
    {
    }
    virtual void set_cor_hori_ang(const std::vector<float> angles)
    {
    }

    uint8_t _data[DIFOP_SIZE + 100] = {0};
};
struct difop_data16 : difop_data
{
    // Corrected intensity curves coefficient
    // Corrected static
    // Corrected vertical angle
    void set_cor_vert_ang(const std::vector<float> angles);
};
struct difop_data32 : difop_data
{
    void set_cor_vert_ang(const std::vector<float> angles);
    void set_cor_hori_ang(const std::vector<float> angles);
};

struct difop_data_bpearl : difop_data32
{
    void set_cor_vert_ang(const std::vector<float> angles);
    void set_cor_hori_ang(const std::vector<float> angles);
};

struct difop_data_helios : difop_data32
{
    virtual void set_return_mode(uint8_t return_mode);
    virtual uint8_t get_return_mode() const;
    void set_cor_vert_ang(const std::vector<float> angles);
    void set_cor_hori_ang(const std::vector<float> angles);

    virtual void set_time(const utc_time& utime);
};
struct difop_data_ruby : difop_data_helios
{
    void set_cor_vert_ang(const std::vector<float> angles);
    void set_cor_hori_ang(const std::vector<float> angles);

    virtual void set_time(const utc_time& utime);
};

struct difop_data_ss
{
    static const uint32_t DIFOP_HEADER_SIZE = 8;
    static const uint32_t DIFOP_DATA_SIZE = 247;
    static const uint32_t DIFOP_TAIL_SIZE = 1;
    static const uint32_t DIFOP_SIZE = (DIFOP_HEADER_SIZE + DIFOP_DATA_SIZE + DIFOP_TAIL_SIZE);

    difop_data_ss();
    virtual ~difop_data_ss() = default;

    void copy(void* d) const;
    const uint8_t* data() const;

    void set_frequency(uint8_t f);
    // du,*100
    void set_hor_fov(uint16_t start, uint16_t end);
    void set_ele_fov(uint16_t start, uint16_t end);

    // eth
    void set_eth(
        uint8_t lidar_ip[4], uint8_t dest_pc_ip[4], uint8_t mac_addr[6], uint16_t msop_port, uint16_t difop_port);
    void set_sn(uint8_t sn[6]);
    virtual void set_return_mode(uint8_t return_mode);
    virtual uint8_t get_return_mode() const;

    virtual void set_time(const utc_time& utime);

    uint8_t _data[DIFOP_SIZE + 100] = {0};
};

class RSLidar : public lidar::TraditionalLidar
{
public:
    RSLidar();
    ~RSLidar();

    virtual float getRotationFrequency() const;
    virtual float getRange() const;
    virtual lidar::ReturnMode getReturnMode() const;
    virtual float getHorizontalResolution() const;
    virtual std::pair<float, float> getYawPitchAngle(uint32_t pos, uint32_t r) const;

    virtual void setRange(float r);
    // 5hz 10hz 20hz
    virtual void setRotationFrequency(float rf);
    virtual void setReturnMode(lidar::ReturnMode rm);

    const uint8_t* getDifopBuf() const;
    virtual bool loadInterReference(const std::string& dir);
    virtual bool setAngleFromString(const FString& str);
    virtual bool setIP(const FString& ip, const FString& port);
    virtual bool Init();
    virtual const char* getGPRMC() const
    {
        return nullptr;
    };
    virtual float getLaserRadius() const
    {
        return 0;
    }
    virtual float getLaserHeight() const
    {
        return 0;
    }

    const std::string& error()
    {
        return _error;
    }

protected:
    std::shared_ptr<msop_data> _msop_data;
    std::shared_ptr<difop_data> _difop_data;
    std::vector<float> _horizontal_angles;
    std::vector<float> _vertical_angles;
    float _range;
    float _horizontal_resolution;
    std::string _udp_IP;
    uint32 _udp_msop_port;
    uint32 _udp_difop_port;

    std::string _error;

    // udp
    FSocket* SenderSocket = nullptr;
    TSharedPtr<FInternetAddr> RemoteAddr_msop;
    TSharedPtr<FInternetAddr> RemoteAddr_difop;
    std::time_t pretime;

    int _temperature = 68;
    const int TEMPERATURE_MIN = 31;

    void send_data(std::time_t t);
};

class RSLidar16 : public RSLidar
{
public:
    RSLidar16();
    ~RSLidar16() = default;

    virtual lidar::LidarType getType() const;
    virtual uint32_t getRaysNum() const
    {
        return 16u;
    }
    virtual uint32_t getHorizontalScanMinUnit() const;
    virtual float getLaserRadius() const;
    virtual float getLaserHeight() const;

    virtual bool loadInterReference(const std::string& dir);

    virtual void setRotationFrequency(float rf);

    virtual bool Init();
    virtual uint32_t package(const lidar_ptset& datas);

protected:
private:
    float _aIntensityCal[7][16] = {{0}};
    int _dChannelNum[16][41] = {{0}};

    uint8_t reflectivity_calibration(float instensity) const;
    virtual uint16_t distance2pixel(float distance, uint32_t passageway) const;
};

class RSLidar32 : public RSLidar
{
public:
    RSLidar32();
    ~RSLidar32() = default;

    virtual lidar::LidarType getType() const
    {
        return lidar::LT_RSLiDAR32;
    }
    virtual uint32_t getRaysNum() const
    {
        return 32u;
    }
    virtual uint32_t getHorizontalScanMinUnit() const;
    virtual float getLaserRadius() const;
    virtual float getLaserHeight() const;

    virtual bool loadInterReference(const std::string& dir);
    virtual void setRotationFrequency(float rf);

    virtual bool Init();
    virtual uint32_t package(const lidar_ptset& datas);

protected:
    uint8_t reflectivity_calibration(float instensity) const;
    virtual uint16_t distance2pixel(float distance, uint32_t passageway) const;

private:
    int _dChannelNum[32][51] = {{0}};
};

class RSBpearl : public RSLidar32
{
public:
    RSBpearl();
    virtual lidar::LidarType getType() const
    {
        return lidar::LT_RSBpearl;
    }
    virtual float getLaserRadius() const
    {
        return 0.01473;
    }
    virtual float getLaserHeight() const
    {
        return 0.09427;
    }
    virtual bool loadInterReference(const std::string& dir);

    virtual bool Init();
};

class RSHelios : public RSBpearl
{
public:
    RSHelios();
    virtual float getLaserRadius() const
    {
        return 0.03498;
    }
    virtual float getLaserHeight() const
    {
        return 0;
    }
    virtual lidar::LidarType getType() const
    {
        return lidar::LT_RSHelios;
    }

    virtual lidar::ReturnMode getReturnMode() const;
    virtual void setReturnMode(lidar::ReturnMode rm);
};

class RSRuby : public RSLidar
{
public:
    RSRuby();

    virtual lidar::LidarType getType() const
    {
        return lidar::LT_RSRuby;
    }
    virtual uint32_t getRaysNum() const
    {
        return 128u;
    }

    virtual uint32_t getHorizontalScanMinUnit() const;
    virtual float getLaserRadius() const
    {
        return 0.03615;
    }
    virtual float getLaserHeight() const
    {
        return 0;
    }

    virtual void setRotationFrequency(float rf);
    virtual bool loadInterReference(const std::string& dir);

    virtual bool Init();
    virtual uint32_t package(const lidar_ptset& datas);

    virtual lidar::ReturnMode getReturnMode() const;
    virtual void setReturnMode(lidar::ReturnMode rm);

protected:
    uint8_t reflectivity_calibration(float instensity) const;
    virtual uint16_t distance2pixel(float distance, uint32_t passageway) const;
};

class RSSLidar : public lidar::TraditionalLidar
{
public:
    virtual void setRange(float r)
    {
        _range = r;
    }
    virtual float getRange() const
    {
        return _range;
    }
    virtual void setReturnMode(lidar::ReturnMode rm);
    lidar::ReturnMode getReturnMode() const;

    virtual uint32_t getHorizontalScanCount() const;    // 水平扫描总数
    virtual float getHorizontalScanAngle(uint32_t pos) const;
    virtual std::pair<float, float> getYawPitchAngle(uint32_t pos, uint32_t r) const;
    const uint8_t* getDifopBuf() const;
    virtual bool loadInterReference(const std::string& dir);
    virtual bool setAngleFromString(const FString& str);
    virtual bool setIP(const FString& ip, const FString& port);
    virtual bool Init();
    virtual const char* getGPRMC() const
    {
        return nullptr;
    };

    const std::string& error()
    {
        return _error;
    }

protected:
    std::shared_ptr<msop_data_ss> _msop_data;
    std::shared_ptr<difop_data_ss> _difop_data;
    std::vector<float> _horizontal_angles;
    std::vector<float> _vertical_angles;
    float _range;
    std::string _udp_IP;
    uint32 _udp_msop_port;
    uint32 _udp_difop_port;

    std::string _error;

    // udp
    FSocket* SenderSocket = nullptr;
    TSharedPtr<FInternetAddr> RemoteAddr_msop;
    TSharedPtr<FInternetAddr> RemoteAddr_difop;
    std::time_t pretime;

    int _temperature = 23;
    const int TEMPERATURE_MIN = 70;

    void send_data(std::time_t t);
};

class RSM1 : public RSSLidar
{
public:
    RSM1();

    virtual lidar::LidarType getType() const
    {
        return lidar::LT_RSM1;
    }
    virtual uint32_t getRaysNum() const
    {
        return msop_data_m1::BLOCKS_PER_MSOP * msop_data_m1::CHANNELS_PER_BLOCK;
    }

    virtual uint32_t getHorizontalScanMinUnit() const
    {
        return 1;
    }
    virtual float getLaserRadius() const
    {
        return 0;
    }
    virtual float getLaserHeight() const
    {
        return 0;
    }
    virtual void setReturnMode(lidar::ReturnMode rm);
    lidar::ReturnMode getReturnMode() const;

    virtual float getRotationFrequency() const
    {
        return 10;
    }
    virtual bool loadInterReference(const std::string& dir);

    virtual uint32_t package(const lidar_ptset& datas);

protected:
    virtual float getHorizontalResolution() const
    {
        return 0.2;
    }
};
class RSM1P : public RSM1
{
public:
    virtual lidar::LidarType getType() const
    {
        return lidar::LT_RSM1;
    }
};

}    // namespace rslidar
