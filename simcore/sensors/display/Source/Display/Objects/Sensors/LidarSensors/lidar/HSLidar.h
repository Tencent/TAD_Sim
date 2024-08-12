#pragma once
#include <string>
#include <vector>
#include <assert.h>
#include <memory>
#include <map>
#include <utility>
#include "Lidar.h"
#include "Networking.h"
#include <functional>

namespace hslidar
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
    void set_utc6(uint8_t* data) const;
    void set_us(uint8_t* data) const;
};

class HSLidar : public lidar::TraditionalLidar
{
public:
    HSLidar() = default;
    ~HSLidar() = default;

    int GetID()
    {
        return _id;
    }
    void SetID(int id)
    {
        _id = id;
    }
    virtual float getRange() const
    {
        return _range;
    }
    virtual float getHorizontalResolution() const
    {
        return _horizontal_resolution;
    }
    virtual std::pair<float, float> getYawPitchAngle(uint32_t pos, uint32_t r) const;
    virtual float getLaserRadius() const;
    virtual float getLaserHeight() const;

    virtual void setRange(float r)
    {
        _range = r;
    }

    virtual bool loadInterReference(const std::string& dir)
    {
        return false;
    }
    virtual bool setAngleFromString(const FString& str);
    virtual bool setIP(const FString& ip, const FString& port);

    const std::string& error()
    {
        return _error;
    }

protected:
    int _id{0};
    std::vector<float> _horizontal_angles;
    std::vector<float> _vertical_angles;
    float _range = 120;
    float _horizontal_resolution = 0.18f;
    std::string _udp_IP;
    uint32 _udp_pt_port = 0;
    uint32 _udp_gps_port = 0;

    std::string _error;

    // udp
    FSocket* SenderSocket = nullptr;
    TSharedPtr<FInternetAddr> RemoteAddr_msop;
    TSharedPtr<FInternetAddr> RemoteAddr_difop;
    std::time_t pretime = 0;

    virtual utm_time to_utime(std::time_t t) const;
    virtual void send_data(std::time_t t) = 0;
    bool readini(const std::string& ini, std::map<std::string, std::map<std::string, std::string>>& data);
    void split_string(const std::string& s, const std::string& delim, std::vector<std::string>& ret);
};

class FTcpReceiverThread : public FRunnable
{
public:
    FString tcpDesc;
    int tcpBufferSize = 1024;
    int tcpPort = 0;
    float tcpUpdateRate = 0.1f;
    std::function<void(const TArray<uint8>& in, TArray<uint8>& out)> callBack;

public:
    FTcpReceiverThread()
    {
    }
    ~FTcpReceiverThread()
    {
        stopping = true;
        m_Thread->WaitForCompletion();
        delete m_Thread;
    }
    void CreateThread()
    {
        m_Thread = FRunnableThread::Create(this, TEXT("TcpReceiverThread"), 0, EThreadPriority::TPri_Normal);
    }

    virtual bool Init() override
    {
        stopping = false;
        return true;
    }

    virtual uint32 Run() override
    {
        FIPv4Endpoint Endpoint(FIPv4Address(127, 0, 0, 1), tcpPort);
        FSocket* ListenSocket = FTcpSocketBuilder(tcpDesc)
                                    //.AsReusable()
                                    .BoundToEndpoint(Endpoint)
                                    .Listening(128);

        if (!ListenSocket)
        {
            UE_LOG(LogTemp, Log, TEXT("Lidar cannot create socket: %s."), *tcpDesc);
            return 0;
        }
        UE_LOG(LogTemp, Log, TEXT("Lidar create socket suc %d: %s."), tcpPort, *tcpDesc);
        // Set Buffer Size
        int32 NewSize = 0;
        ListenSocket->SetReceiveBufferSize(tcpBufferSize, NewSize);

        TArray<uint8> ReceiveData;
        uint32 size = 1024u;

        TSharedRef<FInternetAddr> RemoteAddress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
        while (!stopping)
        {
            bool Pending = false;

            // handle incoming connections
            if (ListenSocket->HasPendingConnection(Pending) && Pending)
            {
                // New Connection receive!
                FSocket* ConnectionSocket = ListenSocket->Accept(*RemoteAddress, *tcpDesc);

                if (ConnectionSocket != NULL)
                {
                    // Binary Array!

                    TArray<uint8> InData;
                    uint32 Size = 0;
                    while (ConnectionSocket->HasPendingData(Size) && !stopping)
                    {
                        TArray<uint8> ReceivedData;
                        ReceivedData.Init(0, FMath::Min(Size, 65507u));

                        int32 Read = 0;
                        if (ConnectionSocket->Recv(ReceivedData.GetData(), ReceivedData.Num(), Read))
                        {
                            if (Read > 0)
                            {
                                ReceivedData.SetNum(Read);
                                InData.Append(ReceivedData);
                            }
                        }
                        else
                        {
                            UE_LOG(LogTemp, Log, TEXT("Recv Failed!"));
                        }
                        UE_LOG(LogTemp, Log, TEXT("%s Data Bytes Read : %s Byte."), *tcpDesc, *FString::FromInt(Read));
                    }
                    if (InData.Num() > 0)
                    {
                        TArray<uint8> OutData;
                        callBack(InData, OutData);
                        if (OutData.Num() > 0)
                        {
                            int32 Write = 0;
                            if (ConnectionSocket->Send(OutData.GetData(), OutData.Num(), Write))
                            {
                            }
                            else
                            {
                                UE_LOG(LogTemp, Log, TEXT("Send Failed!"));
                            }
                            UE_LOG(LogTemp, Log, TEXT("%s Data Bytes Write : %s Byte."), *tcpDesc,
                                *FString::FromInt(Write));
                        }
                    }

                    ConnectionSocket->Close();
                    ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ConnectionSocket);
                }
            }

            FPlatformProcess::Sleep(tcpUpdateRate);
        }

        ListenSocket->Close();
        ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ListenSocket);
        return 0;
    }

    virtual void Stop() override
    {
        stopping = true;
    }

protected:
    FRunnableThread* m_Thread = 0;

private:
    bool stopping = false;
};

class point_data32
{
public:
    static const uint32_t BLOCK_HEADER_SIZE = 2;
    static const uint32_t CHANNEL_SIZE = 4;
    static const uint32_t CHANNELS_PER_BLOCK = 32;
    static const uint32_t BLOCK_CHANNEL_SIZE = (CHANNEL_SIZE * CHANNELS_PER_BLOCK);    // 120
    static const uint32_t BLOCK_SIZE = (BLOCK_CHANNEL_SIZE + BLOCK_HEADER_SIZE);       // 130
    static const uint32_t BLOCKS_PER_PACKAGE = 8;
    static const uint32_t PACKAGE_DATA_SIZE = (BLOCKS_PER_PACKAGE * BLOCK_SIZE);    // 1240
    static const uint32_t PACKAGE_HEADER_SIZE = 6;
    static const uint32_t PTS_HEADER_SIZE = 6;
    static const uint32_t PTS_TAIL_SIZE = 28;
    static const uint32_t PTS_SIZE = (PTS_HEADER_SIZE + PACKAGE_HEADER_SIZE + PACKAGE_DATA_SIZE + PTS_TAIL_SIZE);
    static const uint32_t PTS_RESERVE = 1024;
    static const uint32_t PTS_TOTAL_SIZE = (PTS_SIZE + PTS_RESERVE);
    static const uint32_t GPS_SIZE = 512;

    point_data32();
    virtual ~point_data32();
    const uint8_t* pt_data() const;
    const uint8_t* gps_data() const;

    void set_block_azimuth(uint8_t bn, uint16_t azimuth);
    void set_channel_data(uint8_t bn, uint8_t cn, uint16_t distance, uint8_t reflectivity);
    void set_return_mode(uint8_t return_mode);
    uint8_t get_return_mode() const;
    void set_frequency(uint8_t f);
    void set_sequence(uint32_t seq);

    virtual void set_time(const utm_time& utime);

protected:
    uint8_t _data[PTS_TOTAL_SIZE];
    uint8_t _GPS[GPS_SIZE];
    uint8_t* _channel_data_ptr[BLOCKS_PER_PACKAGE][CHANNELS_PER_BLOCK];
    uint8_t* _tail_ptr;
};

class HSLidar32 : public HSLidar
{
public:
    HSLidar32();
    ~HSLidar32();

    virtual lidar::LidarType getType() const
    {
        return lidar::LT_HS32;
    }
    virtual uint32_t getRaysNum() const
    {
        return point_data32::CHANNELS_PER_BLOCK;
    }
    virtual float getRotationFrequency() const;
    virtual lidar::ReturnMode getReturnMode() const;

    // 5hz 10hz 20hz
    virtual void setRotationFrequency(float rf);
    virtual void setReturnMode(lidar::ReturnMode rm);
    virtual uint32_t getHorizontalScanMinUnit() const;

    virtual bool loadInterReference(const std::string& dir);
    virtual bool Init();
    virtual uint32_t package(const lidar_ptset& datas);

protected:
    uint32_t udp_seq = 0;
    std::shared_ptr<point_data32> _pt_data;
    virtual void send_data(std::time_t t);

    TArray<uint8> m_CalibrationBuffer;
    TSharedPtr<FTcpReceiverThread> m_TcpPtcThread;
    void PtcFunc(const TArray<uint8>& in, TArray<uint8>& out);
};

class point_data40
{
public:
    static const uint32_t BLOCK_HEADER_SIZE = 4;
    static const uint32_t CHANNEL_SIZE = 3;
    static const uint32_t CHANNELS_PER_BLOCK = 40;
    static const uint32_t BLOCK_CHANNEL_SIZE = (CHANNEL_SIZE * CHANNELS_PER_BLOCK);    // 120
    static const uint32_t BLOCK_SIZE = (BLOCK_CHANNEL_SIZE + BLOCK_HEADER_SIZE);       // 124
    static const uint32_t BLOCKS_PER_PACKAGE = 10;
    static const uint32_t PACKAGE_DATA_SIZE = (BLOCKS_PER_PACKAGE * BLOCK_SIZE);    // 1240
    static const uint32_t PTS_TAIL_SIZE = 16;
    static const uint32_t PTS_SIZE = (PACKAGE_DATA_SIZE + PTS_TAIL_SIZE);
    static const uint32_t PTS_RESERVE = 1024;
    static const uint32_t PTS_TOTAL_SIZE = (PTS_SIZE + PTS_RESERVE);
    static const uint32_t GPS_SIZE = 512;

    point_data40();
    virtual ~point_data40();
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

class point_data40p : public point_data40
{
public:
    static const uint32_t PTS_TAIL_SIZE2 = 22;
    static const uint32_t PTS_SIZE2 = (PACKAGE_DATA_SIZE + PTS_TAIL_SIZE2);
};

class HSLidar40 : public HSLidar
{
public:
    HSLidar40();
    ~HSLidar40();

    virtual lidar::LidarType getType() const
    {
        return lidar::LT_HS40;
    }
    virtual uint32_t getRaysNum() const
    {
        return point_data40::CHANNELS_PER_BLOCK;
    }
    virtual float getRotationFrequency() const;
    virtual lidar::ReturnMode getReturnMode() const;

    // 5hz 10hz 20hz
    virtual void setRotationFrequency(float rf);
    virtual void setReturnMode(lidar::ReturnMode rm);
    virtual uint32_t getHorizontalScanMinUnit() const;

    virtual bool loadInterReference(const std::string& dir);
    virtual bool Init();
    virtual uint32_t package(const lidar_ptset& datas);

protected:
    std::shared_ptr<point_data40> _pt_data;
    virtual void send_data(std::time_t t);
};

class point_data64
{
public:
    static const uint32_t BLOCK_HEADER_SIZE = 2;
    static const uint32_t CHANNEL_SIZE = 3;
    static const uint32_t CHANNELS_PER_BLOCK = 64;
    static const uint32_t BLOCK_CHANNEL_SIZE = (CHANNEL_SIZE * CHANNELS_PER_BLOCK);    // 192
    static const uint32_t BLOCK_SIZE = (BLOCK_CHANNEL_SIZE + BLOCK_HEADER_SIZE);       // 194
    static const uint32_t BLOCKS_PER_PACKAGE = 6;
    static const uint32_t PACKAGE_DATA_SIZE = (BLOCKS_PER_PACKAGE * BLOCK_SIZE);    // 1164

    static const uint32_t PACKAGE_HEADER_SIZE = 8;
    static const uint32_t PACKAGE_TAIL_SIZE = 26;
    static const uint32_t PTS_SIZE = (PACKAGE_DATA_SIZE + PACKAGE_HEADER_SIZE + PACKAGE_TAIL_SIZE);
    static const uint32_t PTS_RESERVE = 1024;
    static const uint32_t PTS_TOTAL_SIZE = (PTS_SIZE + PTS_RESERVE);
    static const uint32_t GPS_SIZE = 512;

    point_data64();
    virtual ~point_data64();
    const uint8_t* pt_data() const;
    const uint8_t* gps_data() const;

    void set_block_azimuth(uint8_t bn, uint16_t azimuth);
    void set_channel_data(uint8_t bn, uint8_t cn, uint16_t distance, uint8_t reflectivity);
    void set_return_mode(uint8_t return_mode);
    void set_frequency(uint8_t f);
    uint8_t get_return_mode() const;

    virtual void set_time(const utm_time& utime);

protected:
    uint8_t _data[PTS_TOTAL_SIZE];
    uint8_t _GPS[GPS_SIZE];
    uint8_t* _channel_data_ptr[BLOCKS_PER_PACKAGE][CHANNELS_PER_BLOCK];
    uint8_t* _tail_ptr;
};

class HSLidar64 : public HSLidar
{
public:
    HSLidar64();
    ~HSLidar64();

    virtual lidar::LidarType getType() const
    {
        return lidar::LT_HS64;
    }
    virtual uint32_t getRaysNum() const
    {
        return point_data64::CHANNELS_PER_BLOCK;
    }
    virtual float getRotationFrequency() const;
    virtual lidar::ReturnMode getReturnMode() const;

    // 5hz 10hz 20hz
    virtual void setRotationFrequency(float rf);
    virtual void setReturnMode(lidar::ReturnMode rm);
    virtual uint32_t getHorizontalScanMinUnit() const;

    virtual bool loadInterReference(const std::string& dir);
    virtual bool Init();
    virtual uint32_t package(const lidar_ptset& datas);

protected:
    std::shared_ptr<point_data64> _pt_data;
    virtual void send_data(std::time_t t);
};

class point_data64qt
{
public:
    static const uint32_t BLOCK_HEADER_SIZE = 2;
    static const uint32_t CHANNEL_SIZE = 4;
    static const uint32_t CHANNELS_PER_BLOCK = 64;
    static const uint32_t BLOCK_CHANNEL_SIZE = (CHANNEL_SIZE * CHANNELS_PER_BLOCK);    // 256
    static const uint32_t BLOCK_SIZE = (BLOCK_CHANNEL_SIZE + BLOCK_HEADER_SIZE);       // 258
    static const uint32_t BLOCKS_PER_PACKAGE = 4;
    static const uint32_t PACKAGE_DATA_SIZE = (BLOCKS_PER_PACKAGE * BLOCK_SIZE);    // 1032

    static const uint32_t DATA_HEADER_SIZE = 6;
    static const uint32_t PACKAGE_HEADER_SIZE = 6;
    static const uint32_t PACKAGE_TAIL_SIZE = 28;
    static const uint32_t PTS_SIZE = (PACKAGE_DATA_SIZE + DATA_HEADER_SIZE + PACKAGE_HEADER_SIZE + PACKAGE_TAIL_SIZE);
    static const uint32_t PTS_RESERVE = 1024;
    static const uint32_t PTS_TOTAL_SIZE = (PTS_SIZE + PTS_RESERVE);

    point_data64qt();
    virtual ~point_data64qt();
    const uint8_t* pt_data() const;

    void set_block_azimuth(uint8_t bn, uint16_t azimuth);
    void set_channel_data(uint8_t bn, uint8_t cn, uint16_t distance, uint8_t reflectivity);
    void set_return_mode(uint8_t return_mode);
    void set_frequency(uint8_t f);
    uint8_t get_return_mode() const;
    void set_udp_sequence(uint32_t sqc);
    virtual void set_time(const utm_time& utime);

protected:
    uint8_t _data[PTS_TOTAL_SIZE];
    uint8_t* _channel_data_ptr[BLOCKS_PER_PACKAGE][CHANNELS_PER_BLOCK];
    uint8_t* _tail_ptr;
};

class HSLidar64QT : public HSLidar
{
public:
    HSLidar64QT();
    ~HSLidar64QT();

    virtual lidar::LidarType getType() const
    {
        return lidar::LT_HS64QT;
    }
    virtual uint32_t getRaysNum() const
    {
        return point_data64qt::CHANNELS_PER_BLOCK;
    }
    virtual float getRotationFrequency() const;
    virtual lidar::ReturnMode getReturnMode() const;

    // 5hz 10hz 20hz
    virtual void setRotationFrequency(float rf);
    virtual void setReturnMode(lidar::ReturnMode rm);
    virtual uint32_t getHorizontalScanMinUnit() const;

    virtual bool loadInterReference(const std::string& dir);
    virtual bool Init();
    virtual uint32_t package(const lidar_ptset& datas);

protected:
    uint32_t udp_seq = 0;
    std::shared_ptr<point_data64qt> _pt_data;
    virtual void send_data(std::time_t t);
    virtual utm_time to_utime(std::time_t t) const;
};

class point_data128
{
public:
    static const uint32_t BLOCK_HEADER_SIZE = 2;
    static const uint32_t CHANNEL_SIZE = 3;
    static const uint32_t CHANNELS_PER_BLOCK = 128;
    static const uint32_t BLOCK_CHANNEL_SIZE = (CHANNEL_SIZE * CHANNELS_PER_BLOCK);    // 384
    static const uint32_t BLOCK_SIZE = (BLOCK_CHANNEL_SIZE + BLOCK_HEADER_SIZE);       // 386
    static const uint32_t BLOCKS_CRC = 4;
    static const uint32_t BLOCKS_PER_PACKAGE = 2;
    static const uint32_t PACKAGE_DATA_SIZE = (BLOCKS_PER_PACKAGE * BLOCK_SIZE + BLOCKS_CRC);    // 776

    static const uint32_t PACKAGE_HEADER_SIZE = 6;
    static const uint32_t DATA_HEADER_SIZE = 6;
    static const uint32_t PACKAGE_SEC_SIZE = 17;    // 17;
    static const uint32_t PACKAGE_TAIL_SIZE = 30;
    static const uint32_t PACKAGE_TAIL_IMU_SIZE = 22;    // NOT USE
    static const uint32_t PACKAGE_TAIL_CRC_SIZE = 4;
    static const uint32_t PACKAGE_TAIL_SEC_SIZE = 32;    // NOT USE
    static const uint32_t PTS_SIZE = (PACKAGE_DATA_SIZE + PACKAGE_HEADER_SIZE + DATA_HEADER_SIZE + PACKAGE_SEC_SIZE +
                                      PACKAGE_TAIL_SIZE + PACKAGE_TAIL_IMU_SIZE + PACKAGE_TAIL_CRC_SIZE);
    static const uint32_t PTS_RESERVE = 1024;
    static const uint32_t PTS_TOTAL_SIZE = (PTS_SIZE + PTS_RESERVE);
    static const uint32_t GPS_SIZE = 512;

    point_data128();
    virtual ~point_data128();
    const uint8_t* pt_data() const;
    const uint8_t* gps_data() const;

    void set_block_azimuth(uint8_t bn, uint16_t azimuth);
    void set_channel_data(uint8_t bn, uint8_t cn, uint16_t distance, uint8_t reflectivity);
    void set_data_crc();
    void set_tail_crc();
    void set_return_mode(uint8_t return_mode);
    void set_frequency(uint8_t f);
    uint8_t get_return_mode() const;
    void set_udp_sequence(uint32_t sqc);

    virtual void set_time(const utm_time& utime);

protected:
    uint8_t _data[PTS_TOTAL_SIZE];
    uint8_t _GPS[GPS_SIZE];
    uint8_t* _channel_data_ptr[BLOCKS_PER_PACKAGE][CHANNELS_PER_BLOCK];
    uint8_t* _tail_ptr;
};

class HSLidar128 : public HSLidar
{
public:
    HSLidar128();
    ~HSLidar128();

    virtual lidar::LidarType getType() const
    {
        return lidar::LT_HS128;
    }
    virtual uint32_t getRaysNum() const
    {
        return point_data128::CHANNELS_PER_BLOCK;
    }
    virtual float getRotationFrequency() const;
    virtual lidar::ReturnMode getReturnMode() const;

    // 5hz 10hz 20hz
    virtual void setRotationFrequency(float rf);
    virtual void setReturnMode(lidar::ReturnMode rm);
    virtual uint32_t getHorizontalScanMinUnit() const;

    virtual bool loadInterReference(const std::string& dir);
    virtual bool Init();
    virtual uint32_t package(const lidar_ptset& datas);

protected:
    uint32_t udp_seq = 0;
    std::shared_ptr<point_data128> _pt_data;

    virtual utm_time to_utime(std::time_t t) const;
    virtual void send_data(std::time_t t);
};

class point_data128at
{
public:
    static const uint32_t BLOCK_HEADER_SIZE = 3;
    static const uint32_t CHANNEL_SIZE = 4;
    static const uint32_t CHANNELS_PER_BLOCK = 128;
    static const uint32_t BLOCK_CHANNEL_SIZE = (CHANNEL_SIZE * CHANNELS_PER_BLOCK);    // 512
    static const uint32_t BLOCK_SIZE = (BLOCK_CHANNEL_SIZE + BLOCK_HEADER_SIZE);       // 515
    static const uint32_t BLOCKS_CRC = 4;
    static const uint32_t BLOCKS_PER_PACKAGE = 2;
    static const uint32_t PACKAGE_DATA_SIZE = (BLOCKS_PER_PACKAGE * BLOCK_SIZE + BLOCKS_CRC);    // 1034

    static const uint32_t PACKAGE_HEADER_SIZE = 6;
    static const uint32_t DATA_HEADER_SIZE = 6;
    static const uint32_t PACKAGE_TAIL_SIZE = 40;
    static const uint32_t PACKAGE_TAIL_SEC_SIZE = 32;    // NOT USE
    static const uint32_t PTS_SIZE =
        (PACKAGE_DATA_SIZE + PACKAGE_HEADER_SIZE + DATA_HEADER_SIZE + PACKAGE_TAIL_SIZE + PACKAGE_TAIL_SEC_SIZE);
    static const uint32_t PTS_RESERVE = 1024;
    static const uint32_t PTS_TOTAL_SIZE = (PTS_SIZE + PTS_RESERVE);

    point_data128at();
    virtual ~point_data128at();
    const uint8_t* pt_data() const;

    void set_block_azimuth(uint8_t bn, uint16_t azimuth1, uint8_t azimuth2);
    void set_channel_data(uint8_t bn, uint8_t cn, uint16_t distance, uint8_t reflectivity);
    void set_data_crc();
    void set_tail_crc();
    void set_return_mode(uint8_t return_mode);
    void set_frequency(uint8_t f);
    uint8_t get_return_mode() const;
    void set_udp_sequence(uint32_t sqc);

    virtual void set_time(const utm_time& utime);

protected:
    uint8_t _data[PTS_TOTAL_SIZE];
    uint8_t* _channel_data_ptr[BLOCKS_PER_PACKAGE][CHANNELS_PER_BLOCK];
    uint8_t* _tail_ptr;
};

class HSLidar128AT : public HSLidar
{
public:
    HSLidar128AT();
    ~HSLidar128AT();

    virtual lidar::LidarType getType() const
    {
        return lidar::LT_HS128AT;
    }
    virtual uint32_t getRaysNum() const
    {
        return point_data128at::CHANNELS_PER_BLOCK;
    }
    virtual float getRotationFrequency() const;
    virtual lidar::ReturnMode getReturnMode() const;

    // 5hz 10hz 20hz
    virtual void setRotationFrequency(float rf);
    virtual void setReturnMode(lidar::ReturnMode rm);
    virtual uint32_t getHorizontalScanMinUnit() const;
    virtual uint32_t getHorizontalScanCount() const;
    virtual float getHorizontalScanAngle(uint32_t pos) const;
    virtual std::pair<float, float> getYawPitchAngle(uint32_t pos, uint32_t r) const;

    virtual bool loadInterReference(const std::string& dir);
    virtual bool Init();
    virtual uint32_t package(const lidar_ptset& datas);

protected:
    int framefix = 0;
    uint32_t udp_seq = 0;
    std::shared_ptr<point_data128at> _pt_data;

    virtual utm_time to_utime(std::time_t t) const;
    virtual void send_data(std::time_t t);

    struct PandarATCorrectionsHeader
    {
        uint8_t delimiter[2];
        uint8_t version[2];
        uint8_t channel_number;
        uint8_t mirror_number;
        uint8_t frame_number;
        uint8_t frame_config[8];
        uint8_t resolution;
    };

    struct PandarATFrameInfo15
    {
        uint32_t start_frame[8] = {0};
        uint32_t end_frame[8] = {0};
        int32_t azimuth[128] = {0};
        int32_t elevation[128] = {0};
        int8_t azimuth_offset[36000] = {0};
        int8_t elevation_offset[36000] = {0};
    };
    struct PandarATFrameInfo13
    {
        uint16_t start_frame[8] = {0};
        uint16_t end_frame[8] = {0};
        int16_t azimuth[128] = {0};
        int16_t elevation[128] = {0};
        int8_t azimuth_offset[36000] = {0};
        int8_t elevation_offset[36000] = {0};
    };

    struct PandarATCorrections
    {
        PandarATCorrectionsHeader header;
        PandarATFrameInfo13 frame3;
        PandarATFrameInfo15 frame5;
        uint8_t SHA256[32] = {0};
        double start_frame[3] = {0};
        double azimuth[128] = {0};
        double elevation[128] = {0};
        double azimuth_offset[36000] = {0};
        double elevation_offset[36000] = {0};
    };

    PandarATCorrections m_PandarAT_corrections;

    TArray<uint8> m_CalibrationBuffer;
    TSharedPtr<FTcpReceiverThread> m_TcpPtcThread;
    void PtcFunc(const TArray<uint8>& in, TArray<uint8>& out);
};

}    // namespace hslidar
