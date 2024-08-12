#include "VeloLidar.h"
#include <sstream>
#include <fstream>
#include <algorithm>
#include <ctime>
#include "XmlFile.h"

namespace vllidar
{

#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

void utm_time::set_utc(uint8_t* data) const
{
    data[0] = 0xff;
    data[1] = 0xee;
    data[2] = (year % 10) + 0x30;
    data[3] = (year / 10) + 0x30;
    data[4] = (month % 10) + 0x30;
    data[5] = (month / 10) + 0x30;
    data[6] = (day % 10) + 0x30;
    data[7] = (day / 10) + 0x30;
    data[8] = (sec % 10) + 0x30;
    data[9] = (sec / 10) + 0x30;
    data[10] = (min % 10) + 0x30;
    data[11] = (min / 10) + 0x30;
    data[12] = (hour % 10) + 0x30;
    data[13] = (hour / 10) + 0x30;
}

void utm_time::set_us(uint8_t* data) const
{
    uint32_t t = data[0] = (us & 0xff);
    data[1] = (us & 0xff00) >> 8;
    data[2] = (us & 0xff0000) >> 16;
    data[3] = (us & 0xff000000) >> 24;
}

point_data32::point_data32()
{
    memset(_data, 0, PTS_TOTAL_SIZE);
    uint8_t* data = _data;
    for (int i = 0; i < BLOCKS_PER_PACKAGE; i++)
    {
        data[0] = 0xff;
        data[1] = 0xee;

        for (int j = 0; j < CHANNELS_PER_BLOCK; j++)
        {
            _channel_data_ptr[i][j] = data + BLOCK_HEADER_SIZE + j * CHANNEL_SIZE;
        }
        data += BLOCK_SIZE;
    }
    _tail_ptr = _data + PACKAGE_DATA_SIZE;
    _tail_ptr[5] = 0x21;
    _GPS[507] = 1;
}

point_data32::~point_data32()
{
}

const uint8_t* point_data32::pt_data() const
{
    return _data;
}

const uint8_t* point_data32::gps_data() const
{
    return _GPS;
}

void point_data32::set_block_azimuth(uint8_t bn, uint16_t azimuth)
{
    assert(bn < BLOCKS_PER_PACKAGE);
    uint8_t* data = _data + BLOCK_SIZE * bn;
    data[2] = azimuth & 0xff;
    data[3] = azimuth >> 8;
}

void point_data32::set_channel_data(uint8_t bn, uint8_t cn, uint16_t distance, uint8_t reflectivity)
{
    assert(bn < BLOCKS_PER_PACKAGE);
    assert(cn < CHANNELS_PER_BLOCK);
    uint8_t*& data = _channel_data_ptr[bn][cn];
    data[0] = distance & 0xff;
    data[1] = distance >> 8;
    data[2] = reflectivity;
}

void point_data32::set_return_mode(uint8_t return_mode)
{
    _tail_ptr[4] = return_mode;
}

uint8_t point_data32::get_return_mode() const
{
    return _tail_ptr[4];
}

void point_data32::set_time(const utm_time& utime)
{
    utime.set_us(_tail_ptr);
    utime.set_utc(_GPS);
    utime.set_us(_GPS + 14);
}

VeloLidar32::VeloLidar32() : _range(200.f), _horizontal_resolution(0.2f), SenderSocket(nullptr), pretime(0)
{
    _pt_data = std::make_shared<point_data32>();
    setRange(120);
    setRotationFrequency(10.f);
    setReturnMode(lidar::RT_Strongest);
}

VeloLidar32::~VeloLidar32()
{
    if (SenderSocket)    // Clear all sockets!
    {
        SenderSocket->Close();
        ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(SenderSocket);
    }
}

void VeloLidar32::SetID(int id)
{
    _id = id;
}

float VeloLidar32::getRotationFrequency() const
{
    return _horizontal_resolution * 50.f;
}

float VeloLidar32::getRange() const
{
    return _range;
}

lidar::ReturnMode VeloLidar32::getReturnMode() const
{
    lidar::ReturnMode tmp[3] = {lidar::RT_Strongest, lidar::RT_Last, lidar::RT_Dual};
    return tmp[_pt_data->get_return_mode() - 0x37];
}

float VeloLidar32::getHorizontalResolution() const
{
    return _horizontal_resolution;
}

std::pair<float, float> VeloLidar32::getYawPitchAngle(uint32_t pos, uint32_t r) const
{
    return std::make_pair(getHorizontalScanAngle(pos) + _horizontal_angles.at(r), _vertical_angles.at(r));
}

void VeloLidar32::setRange(float r)
{
    _range = r;
}

// 5hz 10hz 20hz

void VeloLidar32::setRotationFrequency(float rf)
{
    _horizontal_resolution = 0.02f * rf;    // 0.1/5
}

void VeloLidar32::setReturnMode(lidar::ReturnMode rm)
{
    uint8_t tmp[3] = {0x39, 0x37, 0x38};
    _pt_data->set_return_mode(tmp[rm]);
}

bool VeloLidar32::loadInterReference(const std::string& dir)
{
    // ini config
    {
        std::map<std::string, std::map<std::string, std::string>> config;
        std::string cfgfile = dir + "/cfg" + std::to_string(GetID()) + ".ini";
        UE_LOG(LogTemp, Log, TEXT("load Lidar VeloLidar32: %s"), ANSI_TO_TCHAR(cfgfile.c_str()));
        if (readini(cfgfile, config))
        {
            uint8_t lidar_ip[4] = {0}, dest_pc_ip[4] = {0}, mac_addr[6] = {0};

            auto dd = config["INFO"]["ip"];
            if (!dd.empty())
            {
                std::vector<std::string> ipstr;
                split_string(dd, ".", ipstr);
                if (ipstr.size() == 4)
                {
                    lidar_ip[0] = atoi(ipstr.at(0).c_str());
                    lidar_ip[1] = atoi(ipstr.at(1).c_str());
                    lidar_ip[2] = atoi(ipstr.at(2).c_str());
                    lidar_ip[3] = atoi(ipstr.at(3).c_str());
                }
            }
            dd = config["INFO"]["mac"];
            if (!dd.empty())
            {
                std::vector<std::string> mstr;
                split_string(dd, ":", mstr);
                if (mstr.size() == 6)
                {
                    char* str;
                    mac_addr[0] = strtol(mstr.at(0).c_str(), &str, 16);
                    mac_addr[1] = strtol(mstr.at(1).c_str(), &str, 16);
                    mac_addr[2] = strtol(mstr.at(2).c_str(), &str, 16);
                    mac_addr[3] = strtol(mstr.at(3).c_str(), &str, 16);
                    mac_addr[4] = strtol(mstr.at(4).c_str(), &str, 16);
                    mac_addr[5] = strtol(mstr.at(5).c_str(), &str, 16);
                }
            }

            dd = config["UDP"]["ip"];
            if (!dd.empty())
            {
                std::vector<std::string> ipstr;
                split_string(dd, ".", ipstr);
                if (ipstr.size() == 4)
                {
                    _udp_IP = dd;
                    dest_pc_ip[0] = atoi(ipstr.at(0).c_str());
                    dest_pc_ip[1] = atoi(ipstr.at(1).c_str());
                    dest_pc_ip[2] = atoi(ipstr.at(2).c_str());
                    dest_pc_ip[3] = atoi(ipstr.at(3).c_str());
                }
            }
            dd = config["UDP"]["pt_port"];
            if (!dd.empty())
            {
                _udp_pt_port = atoi(dd.c_str());
            }
            dd = config["UDP"]["gps_port"];
            if (!dd.empty())
            {
                _udp_gps_port = atoi(dd.c_str());
            }
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("load faild: %s"), ANSI_TO_TCHAR(cfgfile.c_str()));
        }
    }

    _horizontal_angles.resize(getRaysNum(), 0);
    _vertical_angles.resize(getRaysNum(), 0);

    // load angle.csv
    FXmlFile xmlfifle((dir + "/calibration.xml").c_str());
    if (xmlfifle.IsValid())
    {
        FXmlNode* boost_serialization = xmlfifle.GetRootNode();
        if (boost_serialization)
        {
            FXmlNode* DB = boost_serialization->FindChildNode(TEXT("DB"));
            if (DB)
            {
                FXmlNode* points_ = DB->FindChildNode(TEXT("points_"));
                if (points_)
                {
                    TArray<FXmlNode*> ConfigArry = points_->GetChildrenNodes();
                    for (auto& Config : ConfigArry)
                    {
                        FXmlNode* px = Config->FindChildNode(TEXT("px"));
                        if (px)
                        {
                            FXmlNode* id_ = px->FindChildNode(TEXT("id_"));
                            FXmlNode* vertCorrection_ = px->FindChildNode(TEXT("vertCorrection_"));
                            if (id_ && vertCorrection_)
                            {
                                int32 id = FCString::Atoi(*id_->GetContent());
                                if (id >= 0 && id < 32)
                                {
                                    _vertical_angles[id] = FCString::Atof(*vertCorrection_->GetContent());
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        _error += "Cannot read xml";
        return false;
    }
    return true;
}

bool VeloLidar32::setAngleFromString(const FString& str)
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
    if (getRaysNum() > 0 && (_vertical_angles.size() != getRaysNum() || _horizontal_angles.size() != getRaysNum()))
    {
        UE_LOG(LogTemp, Warning, TEXT("lidar %d: Error in angle str."), GetID());
        _vertical_angles.clear();
        _horizontal_angles.clear();
        return false;
    }
    return true;
}

bool VeloLidar32::setIP(const FString& ip, const FString& port)
{
    if (ip.IsEmpty() || port.IsEmpty())
        return false;
    std::string s;
    std::stringstream ss(TCHAR_TO_ANSI(*port));
    std::vector<std::string> data;
    while (std::getline(ss, s, ','))
        data.push_back(s);
    _udp_IP = TCHAR_TO_ANSI(*ip);
    if (data.size() != 2)
    {
        UE_LOG(LogTemp, Warning, TEXT("lidar %d: Error in port str."), GetID());
        return false;
    }

    _udp_pt_port = std::atoi(data[0].c_str());
    _udp_gps_port = std::atoi(data[1].c_str());
    return true;
}
bool VeloLidar32::Init()
{
    if (_udp_IP.empty())
    {
        return true;
    }
    // FIPv4Endpoint Endpoint(FIPv4Address::Any, 6789);
    // Create Remote Address.
    RemoteAddr_msop = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
    RemoteAddr_difop = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();

    bool bIsValid = false;
    RemoteAddr_msop->SetIp(ANSI_TO_TCHAR(_udp_IP.c_str()), bIsValid);
    RemoteAddr_msop->SetPlatformPort(_udp_pt_port);

    if (!bIsValid)
    {
        _error = "UDP Sender: MSOP IP address was not valid,";
        _error += _udp_IP + ":" + std::to_string(_udp_pt_port);
        // return false;
    }
    RemoteAddr_difop->SetIp(ANSI_TO_TCHAR(_udp_IP.c_str()), bIsValid);
    RemoteAddr_difop->SetPlatformPort(_udp_gps_port);

    if (!bIsValid)
    {
        _error = "UDP Sender: DIFOP IP address was not valid,";
        _error += _udp_IP + ":" + std::to_string(_udp_gps_port);
        // return false;
    }
    int32 SendSize = MAX(point_data32::PTS_TOTAL_SIZE, point_data32::GPS_SIZE) * 10240;
    SenderSocket = FUdpSocketBuilder(TEXT("VeloLidar32"))
                       .AsReusable()
                       .WithBroadcast()    /////////////
                       .WithSendBufferSize(SendSize)
        //.BoundToEndpoint(Endpoint)
        ;
    if (!SenderSocket)
    {
        _error += " Socket failed.";
        return false;
    }

    // check(SenderSocket->GetSocketType() == SOCKTYPE_Datagram);

    // Set Send Buffer Size
    SenderSocket->SetSendBufferSize(SendSize, SendSize);
    SenderSocket->SetReceiveBufferSize(SendSize, SendSize);
    return true;
}

utm_time VeloLidar32::to_utime(std::time_t t) const
{
    utm_time ut;
    ut.us = (uint32_t) (t % 1000000);
    std::time_t unixTimestamp = t / 1000000;
    tm* ptm = std::localtime(&unixTimestamp);
    if (ptm)
    {
        ut.year = ptm->tm_year + 1900 - 2000;
        ut.month = ptm->tm_mon + 1;
        ut.day = ptm->tm_mday;
        ut.hour = ptm->tm_hour;
        ut.min = ptm->tm_min;
        ut.sec = ptm->tm_sec;
    }
    return ut;
}

bool VeloLidar32::readini(const std::string& ini, std::map<std::string, std::map<std::string, std::string>>& data)
{
    using namespace std;
    ifstream in_conf_file(ini);
    if (!in_conf_file)
    {
        return false;
    }
    data.clear();
    std::map<std::string, std::string> node;
    string str_line = "";
    string str_root = "";
    while (getline(in_conf_file, str_line))
    {
        if (str_line.empty() || str_line.front() == ';')
        {
            continue;
        }
        string::size_type left_pos = 0;
        string::size_type right_pos = 0;
        string::size_type equal_div_pos = 0;
        string str_key = "";
        string str_value = "";
        if ((str_line.npos != (left_pos = str_line.find("["))) && (str_line.npos != (right_pos = str_line.find("]"))))
        {
            if (!node.empty() && !str_root.empty())
            {
                data.insert(make_pair(str_root, node));
            }
            // cout << str_line.substr(left_pos+1, right_pos-1) << endl;
            str_root = str_line.substr(left_pos + 1, right_pos - 1);
            node.clear();
        }
        else if (str_line.npos != (equal_div_pos = str_line.find("=")))
        {
            str_key = str_line.substr(0, equal_div_pos);
            str_value = str_line.substr(equal_div_pos + 1, str_line.size() - 1);
            if (str_key.find_first_not_of(" ") != std::string::npos)
                str_key.erase(0, str_key.find_first_not_of(" "));
            if (str_key.find_last_not_of(" ") != std::string::npos)
                str_key.erase(str_key.find_last_not_of(" ") + 1);
            if (str_value.find_first_not_of(" ") != std::string::npos)
                str_value.erase(0, str_value.find_first_not_of(" "));
            if (str_value.find_last_not_of(" ") != std::string::npos)
                str_value.erase(str_value.find_last_not_of(" ") + 1);
            if (str_value.rfind('\n') != std::string::npos)
                str_value.erase(str_value.rfind('\n'));
            if (str_value.rfind('\r') != std::string::npos)
                str_value.erase(str_value.rfind('\r'));

            // cout << str_key << "=" << str_value << endl;
            node.insert(make_pair(str_key, str_value));
        }
    }
    if (!node.empty() && !str_root.empty())
    {
        data.insert(make_pair(str_root, node));
    }
    in_conf_file.close();
    return true;
}

void VeloLidar32::send_data(std::time_t t)
{
    if (SenderSocket)
    {
        int32 BytesSent = 0;
        _pt_data->set_time(to_utime(t));
        SenderSocket->SendTo(
            _pt_data->pt_data(), point_data32::PTS_SIZE, BytesSent, *RemoteAddr_msop);    // ï¿œï¿œï¿œÍžï¿œÔ¶ï¿œËµï¿œÖ·

        auto dt = (t - pretime) / 1000;
        if (dt >= 100)
        {
            SenderSocket->SendTo(_pt_data->gps_data(), point_data32::GPS_SIZE, BytesSent, *RemoteAddr_difop);
            pretime = t;
        }
    }
}

void VeloLidar32::split_string(const std::string& s, const std::string& delim, std::vector<std::string>& ret)
{
    ret.clear();
    size_t last = 0;
    size_t index = s.find_first_of(delim, last);
    while (index != std::string::npos)
    {
        ret.push_back(s.substr(last, index - last));
        last = index + 1;
        index = s.find_first_of(delim, last);
    }
    if (index - last > 0)
    {
        ret.push_back(s.substr(last, index - last));
    }
}

uint32_t VeloLidar32::package(const lidar_ptset& datas)
{
    uint32_t channels_per_packet = getHorizontalScanMinUnit();
    uint32_t buf_num = datas.channels.size() / channels_per_packet;
    uint32_t drm = 250;
    uint32_t blkper = getReturnMode() == lidar::RT_Dual ? 2 : 1;
    // A group ahead
    for (uint32_t i = 0; i < buf_num; i++)
    {
        const channel_data* cd = &datas.channels.at(i * channels_per_packet);
        for (uint32_t j = 0; j < channels_per_packet; j++)
        {
            float azimuth = getHorizontalScanAngle(cd->hor_pos);
            assert(cd->pn == getRaysNum() * blkper);
            for (size_t b = 0; b < blkper; ++b)
            {
                _pt_data->set_block_azimuth(j * blkper + b, round(azimuth * 100.f));
                for (uint32_t k = 0; k < point_data32::CHANNELS_PER_BLOCK; k++)
                {
                    const auto& p = cd->points[b * point_data32::CHANNELS_PER_BLOCK + k];
                    _pt_data->set_channel_data(
                        j * blkper + b, k, (uint16_t) round(p.distance * drm), uint8_t(round(p.instensity)));
                }
            }

            cd++;
        }
        send_data(datas.channels.at(i * channels_per_packet).utime);
    }
    return true;
}

uint32_t VeloLidar32::getHorizontalScanMinUnit() const
{
    if (getReturnMode() == lidar::RT_Dual)
    {
        return point_data32::BLOCKS_PER_PACKAGE / 2;
    }
    return point_data32::BLOCKS_PER_PACKAGE;
}

float VeloLidar32::getLaserRadius() const
{
    return 0.04445f;
}

float VeloLidar32::getLaserHeight() const
{
    return 0.0555f;
}

}    // namespace vllidar
