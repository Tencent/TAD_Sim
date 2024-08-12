/**
 *
 */

#include "UTLidar.h"
#include <sstream>
#include <fstream>
#include <algorithm>
#include <ctime>
#include <cmath>
#include "Kismet/KismetMathLibrary.h"

namespace ulidar
{

#define ULIDAR_VERSION 1
#define FLOAT2INT(f) ((int) ((f) *100))

UTLidar::UTLidar()
{
    memset(&_iop_data, 0, sizeof(_iop_data));
    memset(&_pop_data, 0, sizeof(_pop_data));
}

UTLidar::~UTLidar()
{
    SavePcd();
}

float UTLidar::getRotationFrequency() const
{
    return _frequency;
}

float UTLidar::getRange() const
{
    return _range;
}

uint32_t UTLidar::getHorizontalScanCount() const
{
    return std::floor((fov_right - fov_left) / getHorizontalResolution());
}
float UTLidar::getHorizontalScanAngle(uint32_t pos) const
{
    return fov_left + pos * _horizontal_resolution;
}

lidar::ReturnMode UTLidar::getReturnMode() const
{
    return lidar::RT_Strongest;
}

lidar::LidarType UTLidar::getType() const
{
    return lidar::LT_TUser;
}

uint32_t UTLidar::getRaysNum() const
{
    return _ray_num;
}

float UTLidar::getHorizontalResolution() const
{
    return _horizontal_resolution;
}

uint32_t UTLidar::getHorizontalScanMinUnit() const
{
    return _iop_data.blockNum;
}

std::pair<float, float> UTLidar::getYawPitchAngle(uint32_t pos, uint32_t r) const
{
    return std::make_pair(getHorizontalScanAngle(pos) + _horizontal_angles.at(r), _vertical_angles.at(r));
}

void UTLidar::send_data(std::time_t t)
{
    if (SenderSocket)
    {
        int32 BytesSent = 0;
        int32 faildnum = 0;

        while (!SenderSocket->SendTo((uint8*) &_pop_data, _pop_data.byteLen, BytesSent, *RemoteAddr_pop))
        {
            BytesSent = 0;
            FPlatformProcess::Sleep(0.00001);
            faildnum++;
            if (faildnum == 100)
            {
                UE_LOG(LogTemp, Warning, TEXT("TUlidar send pop error: only sent %d bytes. errcode is %d"), BytesSent,
                    ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->TranslateErrorCode(BytesSent));
                break;
            }
        }

        auto dt = (t - pretime) / 1000;
        if (dt >= 100)
        {
            BytesSent = 0;
            if (!SenderSocket->SendTo((uint8*) &_iop_data, _iop_data.byteLen, BytesSent, *RemoteAddr_iop))
            {
                UE_LOG(LogTemp, Warning, TEXT("TUlidar send iop error: only sent %d bytes. errcode is %d"), BytesSent,
                    ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->TranslateErrorCode(BytesSent));
            }
            pretime = t;
        }
    }
}
uint32_t UTLidar::package(const lidar_ptset& datas)
{
    uint32_t channels_per_packet = getHorizontalScanMinUnit();
    uint32_t buf_num = datas.channels.size() / channels_per_packet;

    // A group ahead: process each block in the dataset
    for (uint32_t i = 0; i < buf_num; i++)
    {
        const channel_data& cd0 = datas.channels.at(i * channels_per_packet);
        _pop_data.psn = cd0.hor_pos / channels_per_packet;
        _pop_data.time.set(cd0.utime);
        _pop_data.X = cd0.X;
        _pop_data.Y = cd0.Y;
        _pop_data.Z = cd0.Z;
        _pop_data.roll = cd0.r;
        _pop_data.pitch = cd0.p;
        _pop_data.yaw = cd0.y;
        // Process all points within one horizontal scan unit
        for (uint32_t j = 0; j < channels_per_packet; j++)
        {
            const channel_data& cd = datas.channels.at(i * channels_per_packet + j);
            float azimuth = getHorizontalScanAngle(cd.hor_pos);

            block_data_header* block = (block_data_header*) (_pop_data.block + _pop_data.blockSize * j);
            block->azimuth = FLOAT2INT(azimuth);
            block->timeoffset = cd.utime - cd0.utime;
            cell_data* cell = (cell_data*) ((uint8_t*) block + sizeof(block_data_header));
            assert(cd.pn == getRaysNum());
            for (uint32_t k = 0; k < _ray_num; k++)
            {
                cell[k].distance = FLOAT2INT(cd.points[k].distance);
                cell[k].reflection = std::round(cd.points[k].instensity);
            }
        }
        // Send processed data to the host at appropriate time
        send_data(cd0.utime);
    }
    return buf_num;

    // for a frame
    for (const auto& cd : datas.channels)
    {
        if (cd.hor_pos < 1)
            SavePcd();
        pcdPoints.Empty();
        for (auto c = 0u; c < cd.pn; ++c)
        {
            const auto cdp = cd.points[c];
            auto yawpitch = getYawPitchAngle(cd.hor_pos, c);
            FRotator LaserRot(yawpitch.second, yawpitch.first, 0);    // float InPitch, float InYaw, float InRoll
            // calculate the point in local coordinate system
            FVector p = cdp.distance * UKismetMathLibrary::GetForwardVector(LaserRot);
            Point3I pi;
            pi.x = p.X;
            pi.y = p.Y;
            pi.z = p.Z;
            pi.instensity = cdp.instensity;
            pcdPoints.Add(pi);
        }
    }
    return 0;
}

void UTLidar::setReturnMode(lidar::ReturnMode)
{
    UE_LOG(LogTemp, Warning, TEXT("lidar %d: not support set return mode."), GetID());
}

void UTLidar::setRange(float r)
{
    _range = r;
}

void UTLidar::setRotationFrequency(float rf)
{
    _frequency = rf;
}

void UTLidar::setRaysNum(uint32_t rn)
{
    _ray_num = rn;
}

void UTLidar::setreflectionMode(uint32_t mode)
{
    _reflection_mode = mode;
}

void UTLidar::setFOV(float left, float right, float low, float up)
{
    fov_left = left;
    fov_right = right;
    fov_up = up;
    fov_low = low;
}

void UTLidar::setHorizontalResolution(float r)
{
    _horizontal_resolution = r;
}

bool UTLidar::loadInterReference(const std::string& dir)
{
    std::string cfgfile = dir + "/cfg" + std::to_string(GetID()) + ".ini";
    FString sss;
    GConfig->Flush(true, cfgfile.c_str());
    if (!GConfig->GetString(TEXT("UDP"), TEXT("ip"), sss, cfgfile.c_str()))
    {
        // UE_LOG(LogTemp, Warning, TEXT("cannot read ip"));
    }
    _udp_IP = sss;
    if (!GConfig->GetString(TEXT("UDP"), TEXT("iop_port"), sss, cfgfile.c_str()))
    {
        // UE_LOG(LogTemp, Warning, TEXT("cannot read iop_port"));
    }
    _udp_iop_port = FCString::Atoi(*sss);
    if (!GConfig->GetString(TEXT("UDP"), TEXT("pop_port"), sss, cfgfile.c_str()))
    {
        // UE_LOG(LogTemp, Warning, TEXT("cannot read pop_port"));
    }
    _udp_pop_port = FCString::Atoi(*sss);

    // load angle.csv
    _vertical_angles.clear();
    _horizontal_angles.clear();
    std::ifstream ifs(dir + "/angle.csv");
    if (ifs.good())
    {
        uint32_t loop_num = getRaysNum();
        std::string str;
        while (std::getline(ifs, str))
        {
            std::stringstream ss(str);
            std::vector<std::string> data;
            while (std::getline(ss, str, ','))
                data.push_back(str);
            if (data.empty())
            {
                UE_LOG(LogTemp, Warning, TEXT("lidar %d: Error in angle.csv."), GetID());
                return false;
            }
            _vertical_angles.push_back(std::atof(data[0].c_str()));
            if (data.size() > 1)
                _horizontal_angles.push_back(std::atof(data[1].c_str()));
            else
                _horizontal_angles.push_back(0);
        }
        if (_vertical_angles.size() != loop_num || _horizontal_angles.size() != loop_num)
        {
            UE_LOG(LogTemp, Warning, TEXT("lidar %d: Error in angle.csv."), GetID());
            _vertical_angles.clear();
            _horizontal_angles.clear();
            return false;
        }
    }
    return true;
}

bool UTLidar::setAngleFromString(const FString& str)
{
    if (str.IsEmpty())
    {
        return false;
    }
    std::stringstream sss(TCHAR_TO_ANSI(*str));
    std::string line;
    while (std::getline(sss, line))
    {
        std::string s;
        std::stringstream ss(line);
        std::vector<std::string> data;
        while (std::getline(ss, s, ','))
            data.push_back(s);
        if (data.empty())
        {
            UE_LOG(LogTemp, Warning, TEXT("lidar %d: Error in angle.str."), GetID());
            return false;
        }
        _vertical_angles.push_back(std::atof(data[0].c_str()));
        if (data.size() > 1)
            _horizontal_angles.push_back(std::atof(data[1].c_str()));
        else
            _horizontal_angles.push_back(0);
    }
    if (_ray_num > 0 && (_vertical_angles.size() != _ray_num || _horizontal_angles.size() != _ray_num))
    {
        UE_LOG(LogTemp, Warning, TEXT("lidar %d: Error in angle str."), GetID());
        _vertical_angles.clear();
        _horizontal_angles.clear();
        return false;
    }
    return true;
}

bool UTLidar::setIP(const FString& ip, const FString& port)
{
    if (ip.IsEmpty() || port.IsEmpty())
        return false;
    std::string s;
    std::stringstream ss(TCHAR_TO_ANSI(*port));
    std::vector<std::string> data;
    while (std::getline(ss, s, ','))
        data.push_back(s);
    _udp_IP = ip;
    if (data.size() != 2)
    {
        UE_LOG(LogTemp, Warning, TEXT("lidar %d: Error in port str."), GetID());
        return false;
    }

    _udp_iop_port = std::atoi(data[0].c_str());
    _udp_pop_port = std::atoi(data[1].c_str());
    return true;
}

bool UTLidar::Init()
{
    // ASSERT(_vertical_angles.size() == _horizontal_angles.size());
    if (_vertical_angles.size() != _ray_num)
    {
        UE_LOG(LogTemp, Log, TEXT("lidar %d: ray_num is %d, angle num is %d."), GetID(), _ray_num,
            _vertical_angles.size());
    }
    if (_vertical_angles.empty())
    {
        if (_ray_num == 0)
        {
            return false;
        }
        _horizontal_angles.resize(_ray_num, 0);
        _vertical_angles.resize(_ray_num, (fov_low + fov_up) * 0.5f);
        for (uint32_t i = 0; i < _ray_num; i++)
        {
            _vertical_angles.at(i) = fov_low + i * (fov_up - fov_low) / (_ray_num - 1);
        }
    }
    else
    {
        _ray_num = _vertical_angles.size();
        UE_LOG(LogTemp, Log, TEXT("lidar %d: reset ray_num = %d, as angle num."), GetID(), _ray_num);
    }
    if (_ray_num > MAX_RAYNUM)
    {
        UE_LOG(LogTemp, Warning, TEXT("lidar %d: ray_num  %d > MAX_RAYNUM"), GetID(), _ray_num);
        return false;
    }

    if (!_udp_IP.IsEmpty())
    {
        RemoteAddr_iop = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
        RemoteAddr_pop = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();

        bool bIsValid = false;
        RemoteAddr_iop->SetIp(*_udp_IP, bIsValid);
        RemoteAddr_iop->SetPlatformPort(_udp_iop_port);

        if (!bIsValid)
        {
            UE_LOG(LogTemp, Warning, TEXT("UDP Sender: iop IP address was not valid, iop ip=%s:%d"), *_udp_IP,
                _udp_iop_port);
            return false;
        }
        RemoteAddr_pop->SetIp(*_udp_IP, bIsValid);
        RemoteAddr_pop->SetPlatformPort(_udp_pop_port);

        if (!bIsValid)
        {
            UE_LOG(LogTemp, Warning, TEXT("UDP Sender: pop IP address was not valid, pop ip=%s:%d"), *_udp_IP,
                _udp_pop_port);
            return false;
        }
        int32 SendSize = MAX_BYTE * 10240;
        SenderSocket = FUdpSocketBuilder(TEXT("ULidar"))
                           .AsReusable()
                           .WithBroadcast()    /////////////
                           .WithSendBufferSize(SendSize)
            //.BoundToEndpoint(Endpoint)
            ;
        if (!SenderSocket)
        {
            UE_LOG(LogTemp, Warning, TEXT(" Socket failed."));
            return false;
        }

        UE_LOG(LogTemp, Log, TEXT("Create user lidar: iop ip=%s:%d, pop ip=%s:%d"), *_udp_IP, _udp_iop_port, *_udp_IP,
            _udp_pop_port);
        // check(SenderSocket->GetSocketType() == SOCKTYPE_Datagram);

        // Set Send Buffer Size
        SenderSocket->SetSendBufferSize(SendSize, SendSize);
        SenderSocket->SetReceiveBufferSize(SendSize, SendSize);
    }

    // set buf
    initPackage();

    return true;
}
void UTLidar::initPackage()
{
    _iop_data.header[0] = 0xFF;
    _iop_data.header[1] = 'T';
    _iop_data.header[2] = 'A';
    _iop_data.header[3] = 'D';
    _iop_data.header[4] = 'S';
    _iop_data.header[5] = 'I';
    _iop_data.header[6] = 'M';
    _iop_data.header[7] = 0xFF;
    _iop_data.id = GetID();
    _iop_data.version = ULIDAR_VERSION;
    _iop_data.frequency = _frequency;
    _iop_data.maxDistance = FLOAT2INT(_range);
    _iop_data.laserN = _ray_num;
    _iop_data.azimuthResolution = FLOAT2INT(_horizontal_resolution);
    _iop_data.fovUp = FLOAT2INT(fov_up);
    _iop_data.fovDown = FLOAT2INT(-fov_low);
    _iop_data.fovLeft = FLOAT2INT(-fov_left);
    _iop_data.fovRight = FLOAT2INT(-fov_right);
    _iop_data.reflectionMode = _reflection_mode;
    for (uint32_t i = 0; i < _ray_num; i++)
    {
        _iop_data.angle[i * 2] = FLOAT2INT(_vertical_angles.at(i) + 180.f);
        _iop_data.angle[i * 2 + 1] = FLOAT2INT(_horizontal_angles.at(i) + 180.f);
    }
    _iop_data.byteLen = (int) ((char*) &_iop_data.angle - (char*) &_iop_data) + _ray_num * 2 * sizeof(uint16_t);

    _pop_data.header[0] = 'T';
    _pop_data.header[1] = 'A';
    _pop_data.header[2] = 'D';
    _pop_data.header[3] = 'S';

    int pop_header_size = (int) ((char*) &_pop_data.block - (char*) &_pop_data);
    _pop_data.blockSize = _ray_num * sizeof(cell_data) + sizeof(block_data_header);
    _iop_data.blockNum = std::floor((MAX_BYTE - pop_header_size) / _pop_data.blockSize);
    uint32_t maxscan = getHorizontalScanCount();
    while (maxscan % _iop_data.blockNum != 0)
    {
        _iop_data.blockNum--;
    }
    _pop_data.byteLen = pop_header_size + _pop_data.blockSize * _iop_data.blockNum;

    _iop_data.pkgNum = maxscan / _iop_data.blockNum;
    UE_LOG(LogTemp, Log, TEXT(" Socket iop len is %d, pop len is %d, block num is %d."), _iop_data.byteLen,
        _pop_data.byteLen, _iop_data.blockNum);
}

// ±£´æpcdÎÄ¼þ
bool UTLidar::SavePcd()
{
    FString StringData;
    StringData += "# .PCD v.7 - Point Cloud Data file format\r\n";
    StringData += "VERSION .7\r\n";
    StringData += "FIELDS x y z rgb\r\n";
    StringData += "SIZE 4 4 4 4\r\n";
    StringData += "TYPE F F F F\r\n";
    StringData += "COUNT 1 1 1 1\r\n";
    StringData += "WIDTH 213\r\n";
    StringData += "HEIGHT 1\r\n";
    StringData += "VIEWPOINT 0 0 0 1 0 0 0\r\n";
    StringData += FString(TEXT("POINTS ")) + FString::FromInt(pcdPoints.Num()) + LINE_TERMINATOR;
    StringData += "DATA ascii\r\n";

    for (auto& p : pcdPoints)
    {
        StringData += FString::SanitizeFloat(p.x);
        StringData += " ";
        StringData += FString::SanitizeFloat(p.y);
        StringData += " ";
        StringData += FString::SanitizeFloat(p.z);
        StringData += " ";

        StringData += LINE_TERMINATOR;
    }

    /*UGameInstance* GI = GetWorld()->GetGameInstance();
    UDisplayGameInstance* DGI = NULL;
    if (GI)
    {
        DGI = Cast<UDisplayGameInstance>(GI);
        return DGI->GetSaveDataHandle()->SaveString(StringData, fname);
    }*/
    return true;
}

void utm_time::set(std::time_t t)
{
    us = (int) (t % 1000);
    t = t / 1000;
    ms = (int) (t % 1000);
    t = t / 1000;
    tm* ptm = std::localtime(&t);
    if (ptm)
    {
        year = ptm->tm_year < 100 ? 0 : (ptm->tm_year + 1900 - 2000);
        month = ptm->tm_mon + 1;
        day = ptm->tm_mday;
        hour = ptm->tm_hour;
        min = ptm->tm_min;
        sec = ptm->tm_sec;
    }
}

}    // namespace ulidar
