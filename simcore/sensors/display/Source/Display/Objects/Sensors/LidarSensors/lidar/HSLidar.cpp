/**
 * @
 */
#include "HSLidar.h"
#include <sstream>
#include <fstream>
#include <algorithm>
#include <ctime>
#include <sstream>
#include <vector>
#include "BoostMath.h"
namespace hslidar
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

void utm_time::set_utc6(uint8_t* data) const
{
    data[0] = year;
    data[1] = month;
    data[2] = day;
    data[3] = hour;
    data[4] = min;
    data[5] = sec;
}

void utm_time::set_us(uint8_t* data) const
{
    data[0] = (us & 0xff);
    data[1] = (us & 0xff00) >> 8;
    data[2] = (us & 0xff0000) >> 16;
    data[3] = (us & 0xff000000) >> 24;
}

std::pair<float, float> HSLidar::getYawPitchAngle(uint32_t pos, uint32_t r) const
{
    return std::make_pair(getHorizontalScanAngle(pos) + _horizontal_angles.at(r), _vertical_angles.at(r));
}
float HSLidar::getLaserRadius() const
{
    return 0.04445f;
}

float HSLidar::getLaserHeight() const
{
    return 0.0555f;
}

bool HSLidar::setAngleFromString(const FString& str)
{
    if (str.IsEmpty())
    {
        return false;
    }
    std::stringstream sss(TCHAR_TO_ANSI(*str));
    // Read each line of the file
    std::string line;
    while (std::getline(sss, line))
    {
        // Split the line by comma into separate strings
        std::string s;
        std::stringstream ss(line);
        std::vector<std::string> data;
        while (std::getline(ss, s, ','))
            data.push_back(s);
        // If there's no data left after splitting, log an error message and exit
        if (data.empty())
        {
            UE_LOG(LogTemp, Warning, TEXT("lidar %d: Error in angle.str."), GetID());
            return false;
        }
        // Add vertical angles to the vector
        _vertical_angles.push_back(std::atof(data[0].c_str()));
        // If there are more than two elements in the data array, add horizontal angles as well
        if (data.size() > 1)
            _horizontal_angles.push_back(std::atof(data[1].c_str()));
        else
            _horizontal_angles.push_back(0);
    }
    // Verify that both vectors have equal length
    if (getRaysNum() > 0 && (_vertical_angles.size() != getRaysNum() || _horizontal_angles.size() != getRaysNum()))
    {
        UE_LOG(LogTemp, Warning, TEXT("lidar %d: Error in angle str."), GetID());
        _vertical_angles.clear();
        _horizontal_angles.clear();
        return false;
    }
    return true;
}

bool HSLidar::setIP(const FString& ip, const FString& port)
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

utm_time HSLidar::to_utime(std::time_t t) const
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

bool HSLidar::readini(const std::string& ini, std::map<std::string, std::map<std::string, std::string>>& data)
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
        // Ignore empty lines and comments starting with ';'
        if (str_line.empty() || str_line.front() == ';')
        {
            continue;
        }
        string::size_type left_pos = 0;
        string::size_type right_pos = 0;
        string::size_type equal_div_pos = 0;
        string str_key = "";
        string str_value = "";
        // Find the start of a new section or key value pair
        if ((str_line.npos != (left_pos = str_line.find("["))) && (str_line.npos != (right_pos = str_line.find("]"))))
        {
            // If we have reached the end of this section, insert it into our tree structure
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

// 分割字符串
void HSLidar::split_string(const std::string& s, const std::string& delim, std::vector<std::string>& ret)
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

point_data32::point_data32()
{
    memset(_data, 0, PTS_TOTAL_SIZE);
    uint8_t* data = _data;
    data[0] = 0xEE;
    data[1] = 0xFF;
    data[2] = 0x06;
    data[3] = 0x01;
    data += PTS_HEADER_SIZE;
    data[0] = 0x20;
    data[1] = 0x08;
    data[3] = 0x04;
    data[4] = 0x02;
    data[5] = 0x01;
    data += PACKAGE_HEADER_SIZE;
    for (int i = 0; i < BLOCKS_PER_PACKAGE; i++)
    {
        for (int j = 0; j < CHANNELS_PER_BLOCK; j++)
        {
            _channel_data_ptr[i][j] = data + BLOCK_HEADER_SIZE + j * CHANNEL_SIZE;
        }
        data += BLOCK_SIZE;
    }
    _tail_ptr = data;
    _tail_ptr[10] = 0x37;
    _tail_ptr[23] = 0x42;
    _GPS[507] = 1;
    _GPS[0] = 0xFF;
    _GPS[1] = 0xEE;
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
    uint8_t* data = _data + PTS_HEADER_SIZE + PACKAGE_HEADER_SIZE + BLOCK_SIZE * bn;
    data[0] = azimuth & 0xff;
    data[1] = azimuth >> 8;
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
    _tail_ptr[10] = return_mode;
}

uint8_t point_data32::get_return_mode() const
{
    return _tail_ptr[10];
}

void point_data32::set_frequency(uint8_t f)
{
    int ff = f * 60;
    _tail_ptr[11] = ff & 0xff;
    _tail_ptr[12] = ff >> 8;
}
void point_data32::set_sequence(uint32_t seq)
{
    _tail_ptr[24] = seq & 0x000000ff;
    _tail_ptr[25] = (seq & 0x0000ff00) >> 8;
    _tail_ptr[26] = (seq & 0x00ff0000) >> 16;
    _tail_ptr[27] = (seq & 0xff000000) >> 24;
}
void point_data32::set_time(const utm_time& utime)
{
    utime.set_utc6(_tail_ptr + 13);
    utime.set_us(_tail_ptr + 19);
    utime.set_utc(_GPS);
    utime.set_us(_GPS + 14);

    _tail_ptr[13] += 100;
}

HSLidar32::HSLidar32()
{
    _pt_data = std::make_shared<point_data32>();
    setRange(120);
    setRotationFrequency(10.f);
    setReturnMode(lidar::RT_Strongest);
}

HSLidar32::~HSLidar32()
{
    if (SenderSocket)    // Clear all sockets!
    {
        SenderSocket->Close();
        ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(SenderSocket);
    }
}

float HSLidar32::getRotationFrequency() const
{
    return _horizontal_resolution / 0.018f;
}

lidar::ReturnMode HSLidar32::getReturnMode() const
{
    lidar::ReturnMode tmp[3] = {lidar::RT_Strongest, lidar::RT_Last, lidar::RT_Dual};
    return tmp[_pt_data->get_return_mode() - 0x37];
}

// 5hz 10hz 20hz

void HSLidar32::setRotationFrequency(float rf)
{
    _horizontal_resolution = 0.018f * rf;
    _pt_data->set_frequency(rf);
}

void HSLidar32::setReturnMode(lidar::ReturnMode rm)
{
    uint8_t tmp[3] = {0x39, 0x37, 0x38};
    _pt_data->set_return_mode(tmp[rm]);
}

bool HSLidar32::loadInterReference(const std::string& dir)
{
    // ini config
    {
        std::map<std::string, std::map<std::string, std::string>> config;
        std::string cfgfile = dir + "/cfg" + std::to_string(GetID()) + ".ini";
        UE_LOG(LogTemp, Log, TEXT("load Lidar HS32: %s"), ANSI_TO_TCHAR(cfgfile.c_str()));
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
    {
        FILE* f_ang = 0;
#ifdef WIN32

        if (0 != fopen_s(&f_ang, (dir + "/angle.csv").c_str(), "r") || !f_ang)
#else
        f_ang = fopen((dir + "/angle.csv").c_str(), "r");
        if (!f_ang)
#endif    //

        {
            _error += "Cannot read angle.csv.";
            return false;
        }
        uint32_t loop_num = getRaysNum();
        uint32_t loopi = 0;
        uint32_t i = 0;
        while (!feof(f_ang))
        {
            if (loopi >= loop_num)
                break;
            int tmp = fscanf(f_ang, "%d,%f,%f\n", &i, &_vertical_angles[loopi], &_horizontal_angles[loopi]);
            if (tmp != 3)
            {
                _error += "Error in angle.csv.";
                return false;
            }
            loopi++;
        }
        fclose(f_ang);

        // read to buffer
        std::fstream fin(dir + "/angle.csv", std::ios::binary | std::ios::in);
        if (fin.good())
        {
            int length = 0;
            fin.seekg(0, std::ios::end);
            length = fin.tellg();
            fin.seekg(0, std::ios::beg);
            m_CalibrationBuffer.SetNum(length);
            fin.read((char*) m_CalibrationBuffer.GetData(), length);
            fin.close();
        }
    }

    return true;
}

bool HSLidar32::Init()
{
    if (_udp_IP.empty())
    {
        return true;
    }
    // FIPv4Endpoint Endpoint(FIPv4Address::Any, 6789);
    // Create Remote Address.
    RemoteAddr_msop = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();

    bool bIsValid = false;
    RemoteAddr_msop->SetIp(ANSI_TO_TCHAR(_udp_IP.c_str()), bIsValid);
    RemoteAddr_msop->SetPlatformPort(_udp_pt_port);

    if (!bIsValid)
    {
        _error = "UDP Sender: MSOP IP address was not valid,";
        _error += _udp_IP + ":" + std::to_string(_udp_pt_port);
        // return false;
    }

    // RemoteAddr_difop = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
    // RemoteAddr_difop->SetIp(ANSI_TO_TCHAR(_udp_IP.c_str()), bIsValid);
    // RemoteAddr_difop->SetPlatformPort(_udp_gps_port);

    // if (!bIsValid)
    //{
    //     _error = "UDP Sender: DIFOP IP address was not valid,";
    //     _error += _udp_IP + ":" + std::to_string(_udp_gps_port);
    //     //return false;
    // }
    int32 SendSize = MAX(point_data32::PTS_TOTAL_SIZE, point_data32::GPS_SIZE) * 10240;
    SenderSocket = FUdpSocketBuilder(TEXT("HSLidar32"))
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
    UE_LOG(LogTemp, Log, TEXT("Create Lidar HS32: data ip=%s:%d, gps ip=%s:%d"), ANSI_TO_TCHAR(_udp_IP.c_str()),
        _udp_pt_port, ANSI_TO_TCHAR(_udp_IP.c_str()), _udp_gps_port);

    // check(SenderSocket->GetSocketType() == SOCKTYPE_Datagram);

    // Set Send Buffer Size
    SenderSocket->SetSendBufferSize(SendSize, SendSize);
    SenderSocket->SetReceiveBufferSize(SendSize, SendSize);

    m_TcpPtcThread = MakeShared<FTcpReceiverThread>();
    m_TcpPtcThread->tcpDesc = TEXT("HSLidar32");
    m_TcpPtcThread->tcpPort = _udp_gps_port;
    m_TcpPtcThread->callBack = std::bind(&HSLidar32::PtcFunc, this, std::placeholders::_1, std::placeholders::_2);
    m_TcpPtcThread->CreateThread();

    return true;
}

void HSLidar32::send_data(std::time_t t)
{
    if (SenderSocket)
    {
        int32 BytesSent = 0;
        _pt_data->set_time(to_utime(t));
        _pt_data->set_sequence(udp_seq++);
        if (!SenderSocket->SendTo(_pt_data->pt_data(), point_data32::PTS_SIZE, BytesSent, *RemoteAddr_msop))
        {
            UE_LOG(LogTemp, Warning, TEXT("HSLidar32 send error: only sent %d bytes. errcode is %d"), BytesSent,
                ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->TranslateErrorCode(BytesSent));
        }
        // auto dt = (t - pretime) / 1000;
        // if (dt >= 100)
        //{
        //     SenderSocket->SendTo(_pt_data->gps_data(), point_data32::GPS_SIZE, BytesSent, *RemoteAddr_difop);
        //     pretime = t;
        // }
    }
}

/* This function packages the given data into a buffer and sends it to the host */
uint32_t HSLidar32::package(const lidar_ptset& datas)
{
    // Calculate number of packets required for this set of data
    uint32_t channels_per_packet = getHorizontalScanMinUnit();
    uint32_t buf_num = datas.channels.size() / channels_per_packet;

    /* Get variables needed by package function */
    uint32_t drm = 250;
    uint32_t blkper = getReturnMode() == lidar::RT_Dual ? 2 : 1;
    // A group ahead
    //  A group ahead: process each block in the dataset
    for (uint32_t i = 0; i < buf_num; i++)
    {
        // Process all points within one horizontal scan unit
        const channel_data* cd = &datas.channels.at(i * channels_per_packet);
        for (uint32_t j = 0; j < channels_per_packet; j++)
        {
            float azimuth = getHorizontalScanAngle(cd->hor_pos);
            // Check if we have enough rays available for current block
            assert(cd->pn == getRaysNum() * blkper);
            // For each block within the horizontal scan unit...
            for (size_t b = 0; b < blkper; ++b)
            {
                _pt_data->set_block_azimuth(j * blkper + b, round(azimuth * 100.f));
                // Set distance and intensity values for each channel
                for (uint32_t k = 0; k < point_data32::CHANNELS_PER_BLOCK; k++)
                {
                    const auto& p = cd->points[b * point_data32::CHANNELS_PER_BLOCK + k];
                    // Round distances to avoid floating-point precision issues
                    _pt_data->set_channel_data(
                        j * blkper + b, k, (uint16_t) round(p.distance * drm), uint8_t(round(p.instensity)));
                }
            }
            // Move to next point data structure
            cd++;
        }
        // Send processed data to the host at appropriate time
        send_data(datas.channels.at(i * channels_per_packet).utime);
    }
    return buf_num;
}

uint32_t HSLidar32::getHorizontalScanMinUnit() const
{
    if (getReturnMode() == lidar::RT_Dual)
    {
        return point_data32::BLOCKS_PER_PACKAGE / 2;
    }
    return point_data32::BLOCKS_PER_PACKAGE;
}

void HSLidar32::PtcFunc(const TArray<uint8>& in, TArray<uint8>& out)
{
    if (in.Num() < 8)
    {
        return;
    }
    if (in[0] != 0x47 || in[1] != 0x74)
    {
        return;
    }
    if (in[2] == 0x05)    // PTC_COMMAND_GET_LIDAR_CALIBRATION
    {
        out.SetNumZeroed(8);
        out[0] = 0x47;
        out[1] = 0x74;
        out[2] = 0x05;
        *(uint32*) &out[4] = m_CalibrationBuffer.Num();
        out.Append(m_CalibrationBuffer);
    }
    else
    {
        out.SetNumZeroed(8);
        out[0] = 0x47;
        out[1] = 0x74;
        out[2] = in[2];
        out[3] = 0x05;
    }
}

point_data40::point_data40()
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
    _tail_ptr[15] = 0x42;
    _GPS[507] = 1;
}

point_data40::~point_data40()
{
}

const uint8_t* point_data40::pt_data() const
{
    return _data;
}

const uint8_t* point_data40::gps_data() const
{
    return _GPS;
}

void point_data40::set_block_azimuth(uint8_t bn, uint16_t azimuth)
{
    assert(bn < BLOCKS_PER_PACKAGE);
    uint8_t* data = _data + BLOCK_SIZE * bn;
    data[2] = azimuth & 0xff;
    data[3] = azimuth >> 8;
}

void point_data40::set_channel_data(uint8_t bn, uint8_t cn, uint16_t distance, uint8_t reflectivity)
{
    assert(bn < BLOCKS_PER_PACKAGE);
    assert(cn < CHANNELS_PER_BLOCK);
    uint8_t*& data = _channel_data_ptr[bn][cn];
    data[0] = distance & 0xff;
    data[1] = distance >> 8;
    data[2] = reflectivity;
}

void point_data40::set_return_mode(uint8_t return_mode)
{
    _tail_ptr[14] = return_mode;
}

uint8_t point_data40::get_return_mode() const
{
    return _tail_ptr[14];
}

void point_data40::set_time(const utm_time& utime)
{
    utime.set_us(_tail_ptr + 10);
    utime.set_utc(_GPS);
    utime.set_us(_GPS + 14);
}

HSLidar40::HSLidar40()
{
    _pt_data = std::make_shared<point_data40>();
    setRange(120);
    setRotationFrequency(10.f);
    setReturnMode(lidar::RT_Strongest);
}

HSLidar40::~HSLidar40()
{
    if (SenderSocket)    // Clear all sockets!
    {
        SenderSocket->Close();
        ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(SenderSocket);
    }
}

float HSLidar40::getRotationFrequency() const
{
    return _horizontal_resolution * 50.f;
}

lidar::ReturnMode HSLidar40::getReturnMode() const
{
    lidar::ReturnMode tmp[3] = {lidar::RT_Strongest, lidar::RT_Last, lidar::RT_Dual};
    return tmp[_pt_data->get_return_mode() - 0x37];
}

// 5hz 10hz 20hz

void HSLidar40::setRotationFrequency(float rf)
{
    _horizontal_resolution = 0.02f * rf;    // 0.1/5
}

void HSLidar40::setReturnMode(lidar::ReturnMode rm)
{
    uint8_t tmp[3] = {0x39, 0x37, 0x38};
    _pt_data->set_return_mode(tmp[rm]);
}

bool HSLidar40::loadInterReference(const std::string& dir)
{
    // ini config
    {
        std::map<std::string, std::map<std::string, std::string>> config;
        std::string cfgfile = dir + "/cfg" + std::to_string(GetID()) + ".ini";
        UE_LOG(LogTemp, Log, TEXT("load Lidar HS40: %s"), ANSI_TO_TCHAR(cfgfile.c_str()));
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
    {
        FILE* f_ang = 0;
#ifdef WIN32

        if (0 != fopen_s(&f_ang, (dir + "/angle.csv").c_str(), "r") || !f_ang)
#else
        f_ang = fopen((dir + "/angle.csv").c_str(), "r");
        if (!f_ang)
#endif    //

        {
            _error += "Cannot read angle.csv.";
            return false;
        }
        uint32_t loop_num = getRaysNum();
        uint32_t loopi = 0;
        uint32_t i = 0;
        while (!feof(f_ang))
        {
            if (loopi >= loop_num)
                break;
            int tmp = fscanf(f_ang, "%d,%f,%f\n", &i, &_vertical_angles[loopi], &_horizontal_angles[loopi]);
            if (tmp != 3)
            {
                _error += "Error in angle.csv.";
                return false;
            }
            loopi++;
        }
        fclose(f_ang);
    }

    return true;
}

bool HSLidar40::Init()
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
    int32 SendSize = MAX(point_data40::PTS_TOTAL_SIZE, point_data40::GPS_SIZE) * 10240;
    SenderSocket = FUdpSocketBuilder(TEXT("HSLidar40"))
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
    UE_LOG(LogTemp, Log, TEXT("Create Lidar HS40: data ip=%s:%d, gps ip=%s:%d"), ANSI_TO_TCHAR(_udp_IP.c_str()),
        _udp_pt_port, ANSI_TO_TCHAR(_udp_IP.c_str()), _udp_gps_port);

    // check(SenderSocket->GetSocketType() == SOCKTYPE_Datagram);

    // Set Send Buffer Size
    SenderSocket->SetSendBufferSize(SendSize, SendSize);
    SenderSocket->SetReceiveBufferSize(SendSize, SendSize);
    return true;
}

void HSLidar40::send_data(std::time_t t)
{
    if (SenderSocket)
    {
        int32 BytesSent = 0;
        _pt_data->set_time(to_utime(t));
        if (!SenderSocket->SendTo(_pt_data->pt_data(), point_data40::PTS_SIZE, BytesSent, *RemoteAddr_msop))
        {
            UE_LOG(LogTemp, Warning, TEXT("Hslidar40 send error: only sent %d bytes. errcode is %d"), BytesSent,
                ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->TranslateErrorCode(BytesSent));
        }
        auto dt = (t - pretime) / 1000;
        if (dt >= 100)
        {
            SenderSocket->SendTo(_pt_data->gps_data(), point_data40::GPS_SIZE, BytesSent, *RemoteAddr_difop);
            pretime = t;
        }
    }
}

uint32_t HSLidar40::package(const lidar_ptset& datas)
{
    // Calculate number of packets required for this set of data
    uint32_t channels_per_packet = getHorizontalScanMinUnit();
    uint32_t buf_num = datas.channels.size() / channels_per_packet;
    uint32_t drm = 250;
    uint32_t blkper = getReturnMode() == lidar::RT_Dual ? 2 : 1;
    // A group ahead
    for (uint32_t i = 0; i < buf_num; i++)
    {
        const channel_data* cd = &datas.channels.at(i * channels_per_packet);
        // Process all points within one horizontal scan unit
        for (uint32_t j = 0; j < channels_per_packet; j++)
        {
            float azimuth = getHorizontalScanAngle(cd->hor_pos);
            // Check if we have enough rays available for current bloc
            assert(cd->pn == getRaysNum() * blkper);
            for (size_t b = 0; b < blkper; ++b)
            {
                _pt_data->set_block_azimuth(j * blkper + b, round(azimuth * 100.f));
                // Set distance and intensity values for each channel
                for (uint32_t k = 0; k < point_data40::CHANNELS_PER_BLOCK; k++)
                {
                    const auto& p = cd->points[b * point_data40::CHANNELS_PER_BLOCK + k];
                    // Round distances to avoid floating-point precision issues
                    _pt_data->set_channel_data(
                        j * blkper + b, k, (uint16_t) round(p.distance * drm), uint8_t(round(p.instensity)));
                }
            }
            // Move to next point data structure
            cd++;
        }
        // Send processed data to the host at appropriate time
        send_data(datas.channels.at(i * channels_per_packet).utime);
    }
    return buf_num;
}

uint32_t HSLidar40::getHorizontalScanMinUnit() const
{
    if (getReturnMode() == lidar::RT_Dual)
    {
        return point_data40::BLOCKS_PER_PACKAGE / 2;
    }
    return point_data40::BLOCKS_PER_PACKAGE;
}

point_data64::point_data64()
{
    memset(_data, 0, PTS_TOTAL_SIZE);
    uint8_t* data = _data;
    data[0] = 0xee;
    data[1] = 0xff;
    data[2] = 0x40;
    data[3] = BLOCKS_PER_PACKAGE;
    data[4] = 0x37;
    data[5] = 0x02;
    data = _data + PACKAGE_HEADER_SIZE;
    for (int i = 0; i < BLOCKS_PER_PACKAGE; i++)
    {
        for (int j = 0; j < CHANNELS_PER_BLOCK; j++)
        {
            _channel_data_ptr[i][j] = data + BLOCK_HEADER_SIZE + j * CHANNEL_SIZE;
        }
        data += BLOCK_SIZE;
    }
    _tail_ptr = _data + PACKAGE_HEADER_SIZE + PACKAGE_DATA_SIZE;
    _tail_ptr[14] = 0x37;
    _tail_ptr[15] = 0x42;
    _tail_ptr[22] = 0XFF;
    _tail_ptr[23] = 0XFF;
    _tail_ptr[24] = 0XFF;
    _tail_ptr[25] = 0XFF;
    _GPS[0] = 0xff;
    _GPS[1] = 0xee;
    _GPS[507] = 1;
}

point_data64::~point_data64()
{
}

const uint8_t* point_data64::pt_data() const
{
    return _data;
}

const uint8_t* point_data64::gps_data() const
{
    return _GPS;
}

void point_data64::set_block_azimuth(uint8_t bn, uint16_t azimuth)
{
    assert(bn < BLOCKS_PER_PACKAGE);
    uint8_t* data = _data + PACKAGE_HEADER_SIZE + BLOCK_SIZE * bn;
    data[0] = azimuth & 0xff;
    data[1] = azimuth >> 8;
}

void point_data64::set_channel_data(uint8_t bn, uint8_t cn, uint16_t distance, uint8_t reflectivity)
{
    assert(bn < BLOCKS_PER_PACKAGE);
    assert(cn < CHANNELS_PER_BLOCK);
    uint8_t*& data = _channel_data_ptr[bn][cn];
    data[0] = distance & 0xff;
    data[1] = distance >> 8;
    data[2] = reflectivity;
}

void point_data64::set_return_mode(uint8_t return_mode)
{
    _tail_ptr[14] = return_mode;
}

void point_data64::set_frequency(uint8_t f)
{
    int ff = f * 60;
    _tail_ptr[8] = ff & 0xff;
    _tail_ptr[9] = ff >> 8;
}

uint8_t point_data64::get_return_mode() const
{
    return _tail_ptr[14];
}

void point_data64::set_time(const utm_time& utime)
{
    utime.set_utc6(_tail_ptr + 16);
    utime.set_us(_tail_ptr + 10);
    utime.set_utc(_GPS);
}

HSLidar64::HSLidar64()
{
    _pt_data = std::make_shared<point_data64>();
    setRange(200);
    setRotationFrequency(10.f);
    setReturnMode(lidar::RT_Strongest);
}

HSLidar64::~HSLidar64()
{
    if (SenderSocket)    // Clear all sockets!
    {
        SenderSocket->Close();
        ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(SenderSocket);
    }
}

float HSLidar64::getRotationFrequency() const
{
    return _horizontal_resolution * 50.f;
}

lidar::ReturnMode HSLidar64::getReturnMode() const
{
    lidar::ReturnMode tmp[3] = {lidar::RT_Strongest, lidar::RT_Last, lidar::RT_Dual};
    return tmp[_pt_data->get_return_mode() - 0x37];
}
// 10hz 20hz

void HSLidar64::setRotationFrequency(float rf)
{
    _horizontal_resolution = 0.02f * rf;    // 0.2/10
    _pt_data->set_frequency(rf);
}

void HSLidar64::setReturnMode(lidar::ReturnMode rm)
{
    uint8_t tmp[3] = {0x39, 0x37, 0x38};
    _pt_data->set_return_mode(tmp[rm]);
}

bool HSLidar64::loadInterReference(const std::string& dir)
{
    // ini config
    {
        std::map<std::string, std::map<std::string, std::string>> config;
        std::string cfgfile = dir + "/cfg" + std::to_string(GetID()) + ".ini";
        UE_LOG(LogTemp, Log, TEXT("load Lidar HS64: %s"), ANSI_TO_TCHAR(cfgfile.c_str()));
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
    {
        std::ifstream ifs(dir + "/angle.csv");
        if (ifs.good())
        {
            uint32_t loop_num = getRaysNum();
            std::string str;
            std::getline(ifs, str);
            while (std::getline(ifs, str))
            {
                std::stringstream ss(str);
                std::vector<std::string> data;
                while (std::getline(ss, str, ','))
                    data.push_back(str);
                if (data.size() < 3)
                {
                    _error += "Error in angle.csv.";
                    return false;
                }
                uint32_t loopi = std::atoi(data[0].c_str());
                if (loopi < 1 || loopi > loop_num)
                {
                    _error += "Error in angle.csv.";
                    return false;
                }
                _vertical_angles[loopi - 1] = std::atof(data[1].c_str());
                _horizontal_angles[loopi - 1] = std::atof(data[2].c_str());
            }
        }
    }
    return true;
}

bool HSLidar64::Init()
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
    int32 SendSize = MAX(point_data64::PTS_TOTAL_SIZE, point_data64::GPS_SIZE) * 10240;
    SenderSocket = FUdpSocketBuilder(TEXT("HSLidar64"))
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
    UE_LOG(LogTemp, Log, TEXT("Create Lidar HS64: data ip=%s:%d, gps ip=%s:%d"), ANSI_TO_TCHAR(_udp_IP.c_str()),
        _udp_pt_port, ANSI_TO_TCHAR(_udp_IP.c_str()), _udp_gps_port);

    // check(SenderSocket->GetSocketType() == SOCKTYPE_Datagram);

    // Set Send Buffer Size
    SenderSocket->SetSendBufferSize(SendSize, SendSize);
    SenderSocket->SetReceiveBufferSize(SendSize, SendSize);
    return true;
}

void HSLidar64::send_data(std::time_t t)
{
    if (SenderSocket)
    {
        int32 BytesSent = 0;
        int32 faildnum = 0;
        _pt_data->set_time(to_utime(t));
        while (!SenderSocket->SendTo(_pt_data->pt_data(), point_data64::PTS_SIZE, BytesSent, *RemoteAddr_msop))
        {
            BytesSent = 0;
            FPlatformProcess::Sleep(0.00001);
            faildnum++;
            if (faildnum == 100)
            {
                UE_LOG(LogTemp, Warning, TEXT("Hslidar64 send error: only sent %d bytes. errcode is %d"), BytesSent,
                    ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->TranslateErrorCode(BytesSent));
                break;
            }
        }
        auto dt = (t - pretime) / 1000;
        if (dt >= 100)
        {
            SenderSocket->SendTo(_pt_data->gps_data(), point_data64::GPS_SIZE, BytesSent, *RemoteAddr_difop);
            pretime = t;
        }
    }
}

uint32_t HSLidar64::package(const lidar_ptset& datas)
{
    // Calculate number of packets required for this set of data
    uint32_t channels_per_packet = getHorizontalScanMinUnit();
    uint32_t buf_num = datas.channels.size() / channels_per_packet;
    uint32_t drm = 250;
    uint32_t blkper = getReturnMode() == lidar::RT_Dual ? 2 : 1;
    // A group ahead
    // A group ahead: process each block in the dataset
    for (uint32_t i = 0; i < buf_num; i++)
    {
        const channel_data* cd = &datas.channels.at(i * channels_per_packet);
        // Process all points within one horizontal scan unit
        for (uint32_t j = 0; j < channels_per_packet; j++)
        {
            float azimuth = getHorizontalScanAngle(cd->hor_pos);
            // Check if we have enough rays available for current block
            assert(cd->pn == getRaysNum() * blkper);
            // For each block within the horizontal scan unit...
            for (size_t b = 0; b < blkper; ++b)
            {
                _pt_data->set_block_azimuth(j * blkper + b, round(azimuth * 100.f));
                // Set distance and intensity values for each channel
                for (uint32_t k = 0; k < point_data64::CHANNELS_PER_BLOCK; k++)
                {
                    const auto& p = cd->points[b * point_data64::CHANNELS_PER_BLOCK + k];
                    // Round distances to avoid floating-point precision issues
                    _pt_data->set_channel_data(
                        j * blkper + b, k, (uint16_t) round(p.distance * drm), uint8_t(round(p.instensity)));
                }
            }
            // Move to next point data structure

            cd++;
        }
        // Send processed data to the host at appropriate time
        send_data(datas.channels.at(i * channels_per_packet).utime);
    }
    return buf_num;
}

uint32_t HSLidar64::getHorizontalScanMinUnit() const
{
    if (getReturnMode() == lidar::RT_Dual)
    {
        return point_data64::BLOCKS_PER_PACKAGE / 2;
    }
    return point_data64::BLOCKS_PER_PACKAGE;
}

point_data64qt::point_data64qt()
{
    memset(_data, 0, PTS_TOTAL_SIZE);
    uint8_t* data = _data;
    data[0] = 0xee;
    data[1] = 0xff;
    data[2] = 0x03;
    data[3] = 0x01;
    data = _data + PACKAGE_HEADER_SIZE;
    data[0] = 0x40;
    data[1] = 0x04;
    data[3] = 0x04;
    data[4] = 0x01;
    data[5] = 0x01;
    data = _data + PACKAGE_HEADER_SIZE + DATA_HEADER_SIZE;
    for (int i = 0; i < BLOCKS_PER_PACKAGE; i++)
    {
        for (int j = 0; j < CHANNELS_PER_BLOCK; j++)
        {
            _channel_data_ptr[i][j] = data + BLOCK_HEADER_SIZE + j * CHANNEL_SIZE;
        }
        data += BLOCK_SIZE;
    }
    _tail_ptr = _data + PACKAGE_HEADER_SIZE + DATA_HEADER_SIZE + PACKAGE_DATA_SIZE;
    _tail_ptr[17] = 0x42;
}
point_data64qt::~point_data64qt()
{
}
const uint8_t* point_data64qt::pt_data() const
{
    return _data;
}

void point_data64qt::set_block_azimuth(uint8_t bn, uint16_t azimuth)
{
    assert(bn < BLOCKS_PER_PACKAGE);
    uint8_t* data = _data + PACKAGE_HEADER_SIZE + DATA_HEADER_SIZE + BLOCK_SIZE * bn;
    data[0] = azimuth & 0xff;
    data[1] = azimuth >> 8;
}

void point_data64qt::set_channel_data(uint8_t bn, uint8_t cn, uint16_t distance, uint8_t reflectivity)
{
    assert(bn < BLOCKS_PER_PACKAGE);
    assert(cn < CHANNELS_PER_BLOCK);
    uint8_t*& data = _channel_data_ptr[bn][cn];
    data[0] = distance & 0xff;
    data[1] = distance >> 8;
    data[2] = reflectivity;
}

void point_data64qt::set_return_mode(uint8_t return_mode)
{
    _tail_ptr[16] = return_mode;
}

void point_data64qt::set_frequency(uint8_t f)
{
    int ff = f * 60;
    _tail_ptr[10] = ff & 0xff;
    _tail_ptr[11] = ff >> 8;
}

uint8_t point_data64qt::get_return_mode() const
{
    return _tail_ptr[16];
}

void point_data64qt::set_time(const utm_time& utime)
{
    utime.set_utc6(_tail_ptr + 18);
    utime.set_us(_tail_ptr + 12);
}
void point_data64qt::set_udp_sequence(uint32_t sqc)
{
    _tail_ptr[24] = sqc & 0x000000ff;
    _tail_ptr[25] = (sqc & 0x0000ff00) >> 8;
    _tail_ptr[26] = (sqc & 0x00ff0000) >> 16;
    _tail_ptr[27] = (sqc & 0xff000000) >> 24;
}

HSLidar64QT::HSLidar64QT()
{
    _pt_data = std::make_shared<point_data64qt>();
    setRange(60);
    setRotationFrequency(10.f);
    setReturnMode(lidar::RT_Strongest);
}

HSLidar64QT::~HSLidar64QT()
{
    if (SenderSocket)    // Clear all sockets!
    {
        SenderSocket->Close();
        ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(SenderSocket);
    }
}

float HSLidar64QT::getRotationFrequency() const
{
    return 10.f;
}

lidar::ReturnMode HSLidar64QT::getReturnMode() const
{
    switch (_pt_data->get_return_mode())
    {
        case 0x33:
            return lidar::RT_Strongest;
        case 0x38:
            return lidar::RT_Last;
        case 0x3B:
            return lidar::RT_Dual;
        default:
            break;
    }
    return lidar::RT_Strongest;
}
// 10hz 20hz

void HSLidar64QT::setRotationFrequency(float rf)
{
    _horizontal_resolution = 0.6f;
    _pt_data->set_frequency(10);
}

void HSLidar64QT::setReturnMode(lidar::ReturnMode rm)
{
    uint8_t tmp[3] = {0x3B, 0x33, 0x38};
    _pt_data->set_return_mode(tmp[rm]);
}

bool HSLidar64QT::loadInterReference(const std::string& dir)
{
    _horizontal_angles.resize(getRaysNum(), 0);
    _vertical_angles.resize(getRaysNum(), 0);

    // load angle.csv
    {
        std::ifstream ifs(dir + "/angle.csv");
        if (ifs.good())
        {
            uint32_t loop_num = getRaysNum();
            std::string str;
            std::getline(ifs, str);
            while (std::getline(ifs, str))
            {
                std::stringstream ss(str);
                std::vector<std::string> data;
                while (std::getline(ss, str, ','))
                    data.push_back(str);
                if (data.size() < 3)
                {
                    _error += "Error in angle.csv.";
                    return false;
                }
                uint32_t loopi = std::atoi(data[0].c_str());
                if (loopi < 1 || loopi > loop_num)
                {
                    _error += "Error in angle.csv.";
                    return false;
                }
                _vertical_angles[loopi - 1] = std::atof(data[1].c_str());
                _horizontal_angles[loopi - 1] = std::atof(data[2].c_str());
            }
        }
    }
    return true;
}

bool HSLidar64QT::Init()
{
    if (_udp_IP.empty())
    {
        return true;
    }
    // FIPv4Endpoint Endpoint(FIPv4Address::Any, 6789);
    // Create Remote Address.
    RemoteAddr_msop = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();

    bool bIsValid = false;
    RemoteAddr_msop->SetIp(ANSI_TO_TCHAR(_udp_IP.c_str()), bIsValid);
    RemoteAddr_msop->SetPlatformPort(_udp_pt_port);

    if (!bIsValid)
    {
        _error = "UDP Sender: MSOP IP address was not valid,";
        _error += _udp_IP + ":" + std::to_string(_udp_pt_port);
        // return false;
    }

    int32 SendSize = point_data64qt::PTS_TOTAL_SIZE * 10240;
    SenderSocket = FUdpSocketBuilder(TEXT("HSLidar64QT"))
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
    UE_LOG(LogTemp, Log, TEXT("Create Lidar HS64QT: data ip=%s:%d"), ANSI_TO_TCHAR(_udp_IP.c_str()), _udp_pt_port);

    // check(SenderSocket->GetSocketType() == SOCKTYPE_Datagram);

    // Set Send Buffer Size
    SenderSocket->SetSendBufferSize(SendSize, SendSize);
    SenderSocket->SetReceiveBufferSize(SendSize, SendSize);
    return true;
}

utm_time HSLidar64QT::to_utime(std::time_t t) const
{
    utm_time ut;
    ut.us = (uint32_t) (t % 1000000);
    std::time_t unixTimestamp = t / 1000000;
    tm* ptm = std::localtime(&unixTimestamp);
    if (ptm)
    {
        ut.year = ptm->tm_year;
        ut.month = ptm->tm_mon + 1;
        ut.day = ptm->tm_mday;
        ut.hour = ptm->tm_hour;
        ut.min = ptm->tm_min;
        ut.sec = ptm->tm_sec;
    }
    return ut;
}
void HSLidar64QT::send_data(std::time_t t)
{
    if (SenderSocket)
    {
        int32 BytesSent = 0;
        int32 faildnum = 0;
        _pt_data->set_time(to_utime(t));
        _pt_data->set_udp_sequence(udp_seq++);
        while (!SenderSocket->SendTo(_pt_data->pt_data(), point_data64qt::PTS_SIZE, BytesSent, *RemoteAddr_msop))
        {
            BytesSent = 0;
            FPlatformProcess::Sleep(0.00001);
            faildnum++;
            if (faildnum == 100)
            {
                UE_LOG(LogTemp, Warning, TEXT("Hslidar64qt send error: only sent %d bytes. errcode is %d"), BytesSent,
                    ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->TranslateErrorCode(BytesSent));
                break;
            }
        }
    }
}

uint32_t HSLidar64QT::package(const lidar_ptset& datas)
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
            // Check if we have enough rays available for current block
            assert(cd->pn == getRaysNum() * blkper);
            for (size_t b = 0; b < blkper; ++b)
            {
                _pt_data->set_block_azimuth(j * blkper + b, round(azimuth * 100.f));
                // Set distance and intensity values for each channel
                for (uint32_t k = 0; k < point_data64qt::CHANNELS_PER_BLOCK; k++)
                {
                    const auto& p = cd->points[b * point_data64qt::CHANNELS_PER_BLOCK + k];
                    // Round distances to avoid floating-point precision issues
                    _pt_data->set_channel_data(
                        j * blkper + b, k, (uint16_t) round(p.distance * drm), uint8_t(round(p.instensity)));
                }
            }

            // Move to next point data structure
            cd++;
        }
        // Send processed data to the host at appropriate time
        send_data(datas.channels.at(i * channels_per_packet).utime);
    }
    return buf_num;
}

uint32_t HSLidar64QT::getHorizontalScanMinUnit() const
{
    if (getReturnMode() == lidar::RT_Dual)
    {
        return point_data64qt::BLOCKS_PER_PACKAGE / 2;
    }
    return point_data64qt::BLOCKS_PER_PACKAGE;
}

point_data128::point_data128()
{
    memset(_data, 0, PTS_TOTAL_SIZE);
    uint8_t* data = _data;
    data[0] = 0xee;
    data[1] = 0xff;
    data[2] = 0x01;
    data[3] = 0x04;
    data = _data + PACKAGE_HEADER_SIZE;
    data[0] = 0x80;
    data[1] = 0x02;
    data[3] = 0x04;
    data[4] = 0x02;
    data[5] = 0x01;    // udp seq
    data = _data + PACKAGE_HEADER_SIZE + DATA_HEADER_SIZE;
    for (int i = 0; i < BLOCKS_PER_PACKAGE; i++)
    {
        for (int j = 0; j < CHANNELS_PER_BLOCK; j++)
        {
            _channel_data_ptr[i][j] = data + BLOCK_HEADER_SIZE + j * CHANNEL_SIZE;
        }
        data += BLOCK_SIZE;
    }
    data = _data + PACKAGE_HEADER_SIZE + DATA_HEADER_SIZE + PACKAGE_DATA_SIZE;    // functional safety
    data[1] = 0x1;
    auto r = ueboost::crc(data, 13);
    data[13] = r & 0x000000ff;
    data[14] = (r & 0x0000ff00) >> 8;
    data[15] = (r & 0x00ff0000) >> 16;
    data[16] = (r & 0xff000000) >> 24;

    //
    // data = _data + PACKAGE_HEADER_SIZE + DATA_HEADER_SIZE + PACKAGE_DATA_SIZE;
    // data[1] = 0x20;

    _tail_ptr = _data + PACKAGE_HEADER_SIZE + DATA_HEADER_SIZE + PACKAGE_DATA_SIZE + PACKAGE_SEC_SIZE;
    //_tail_ptr[10] = 0x10;
    //_tail_ptr[11] = 0x02;
    _tail_ptr[12] = 0x37;
    _tail_ptr[25] = 0x42;
    _GPS[0] = 0xff;
    _GPS[1] = 0xee;
    _GPS[507] = 1;
}

point_data128::~point_data128()
{
}

const uint8_t* point_data128::pt_data() const
{
    return _data;
}

const uint8_t* point_data128::gps_data() const
{
    return _GPS;
}

void point_data128::set_block_azimuth(uint8_t bn, uint16_t azimuth)
{
    assert(bn < BLOCKS_PER_PACKAGE);
    uint8_t* data = _data + PACKAGE_HEADER_SIZE + DATA_HEADER_SIZE + BLOCK_SIZE * bn;
    data[0] = azimuth & 0xff;
    data[1] = azimuth >> 8;
}

void point_data128::set_channel_data(uint8_t bn, uint8_t cn, uint16_t distance, uint8_t reflectivity)
{
    assert(bn < BLOCKS_PER_PACKAGE);
    assert(cn < CHANNELS_PER_BLOCK);
    uint8_t*& data = _channel_data_ptr[bn][cn];
    data[0] = distance & 0xff;
    data[1] = distance >> 8;
    data[2] = reflectivity;
}
void point_data128::set_data_crc()
{
    uint8_t* data = _data + PACKAGE_HEADER_SIZE + DATA_HEADER_SIZE;
    auto r = ueboost::crc(_data, BLOCK_SIZE * BLOCKS_PER_PACKAGE);
    data += BLOCK_SIZE * BLOCKS_PER_PACKAGE;
    data[0] = r & 0x000000ff;
    data[1] = (r & 0x0000ff00) >> 8;
    data[2] = (r & 0x00ff0000) >> 16;
    data[3] = (r & 0xff000000) >> 24;
}
void point_data128::set_tail_crc()
{
    auto r = ueboost::crc(_tail_ptr, PACKAGE_TAIL_SIZE - 4);
    uint8_t* data = _tail_ptr + PACKAGE_TAIL_SIZE - 4;
    data[0] = r & 0x000000ff;
    data[1] = (r & 0x0000ff00) >> 8;
    data[2] = (r & 0x00ff0000) >> 16;
    data[3] = (r & 0xff000000) >> 24;
}

void point_data128::set_return_mode(uint8_t return_mode)
{
    _tail_ptr[12] = return_mode;
}

uint8_t point_data128::get_return_mode() const
{
    return _tail_ptr[12];
}

void point_data128::set_udp_sequence(uint32_t sqc)
{
    _tail_ptr[26] = sqc & 0x000000ff;
    _tail_ptr[27] = (sqc & 0x0000ff00) >> 8;
    _tail_ptr[28] = (sqc & 0x00ff0000) >> 16;
    _tail_ptr[29] = (sqc & 0xff000000) >> 24;
}

void point_data128::set_frequency(uint8_t f)
{
    int ff = f * 60;
    _tail_ptr[13] = ff & 0xff;
    _tail_ptr[14] = ff >> 8;
}
void point_data128::set_time(const utm_time& utime)
{
    utime.set_utc6(_tail_ptr + 15);
    utime.set_us(_tail_ptr + 21);
    utime.set_utc(_GPS);
}

HSLidar128::HSLidar128()
{
    _pt_data = std::make_shared<point_data128>();
    setRange(200);
    setRotationFrequency(10.f);
    setReturnMode(lidar::RT_Strongest);
}

HSLidar128::~HSLidar128()
{
    if (SenderSocket)    // Clear all sockets!
    {
        SenderSocket->Close();
        ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(SenderSocket);
    }
}

float HSLidar128::getRotationFrequency() const
{
    return _horizontal_resolution * 50.f;
}

lidar::ReturnMode HSLidar128::getReturnMode() const
{
    lidar::ReturnMode tmp[3] = {lidar::RT_Strongest, lidar::RT_Last, lidar::RT_Dual};
    return tmp[_pt_data->get_return_mode() - 0x37];
}

// 10hz 20hz

void HSLidar128::setRotationFrequency(float rf)
{
    _horizontal_resolution = 0.02f * rf;    // 0.2/10
    _pt_data->set_frequency(rf);
}

void HSLidar128::setReturnMode(lidar::ReturnMode rm)
{
    uint8_t tmp[3] = {0x39, 0x37, 0x38};
    _pt_data->set_return_mode(tmp[rm]);
}

bool HSLidar128::loadInterReference(const std::string& dir)
{
    // ini config
    {
        std::map<std::string, std::map<std::string, std::string>> config;
        std::string cfgfile = dir + "/cfg" + std::to_string(GetID()) + ".ini";
        UE_LOG(LogTemp, Log, TEXT("load Lidar HS128: %s"), ANSI_TO_TCHAR(cfgfile.c_str()));
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
    {
        std::ifstream ifs(dir + "/angle.csv");
        if (ifs.good())
        {
            uint32_t loop_num = getRaysNum();
            std::string str;
            std::getline(ifs, str);
            while (std::getline(ifs, str))
            {
                std::stringstream ss(str);
                std::vector<std::string> data;
                while (std::getline(ss, str, ','))
                    data.push_back(str);
                if (data.size() < 3)
                {
                    _error += "Error in angle.csv.";
                    return false;
                }
                uint32_t loopi = std::atoi(data[0].c_str());
                if (loopi < 1 || loopi > loop_num)
                {
                    _error += "Error in angle.csv.";
                    return false;
                }
                _vertical_angles[loopi - 1] = std::atof(data[1].c_str());
                _horizontal_angles[loopi - 1] = std::atof(data[2].c_str());
            }
        }
    }
    return true;
}

bool HSLidar128::Init()
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
    int32 SendSize = MAX(point_data128::PTS_TOTAL_SIZE, point_data128::GPS_SIZE) * 10240;
    SenderSocket = FUdpSocketBuilder(TEXT("HSLidar128"))
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
    UE_LOG(LogTemp, Log, TEXT("Create Lidar HS128: data ip=%s:%d, gps ip=%s:%d"), ANSI_TO_TCHAR(_udp_IP.c_str()),
        _udp_pt_port, ANSI_TO_TCHAR(_udp_IP.c_str()), _udp_gps_port);

    // check(SenderSocket->GetSocketType() == SOCKTYPE_Datagram);

    // Set Send Buffer Size
    SenderSocket->SetSendBufferSize(SendSize, SendSize);
    SenderSocket->SetReceiveBufferSize(SendSize, SendSize);
    return true;
}

utm_time HSLidar128::to_utime(std::time_t t) const
{
    utm_time ut;
    ut.us = (uint32_t) (t % 1000000);
    std::time_t unixTimestamp = t / 1000000;
    tm* ptm = std::localtime(&unixTimestamp);
    if (ptm)
    {
        ut.year = ptm->tm_year;
        ut.month = ptm->tm_mon + 1;
        ut.day = ptm->tm_mday;
        ut.hour = ptm->tm_hour;
        ut.min = ptm->tm_min;
        ut.sec = ptm->tm_sec;
    }
    return ut;
}

void HSLidar128::send_data(std::time_t t)
{
    if (SenderSocket)
    {
        int32 BytesSent = 0;
        _pt_data->set_time(to_utime(t));
        _pt_data->set_data_crc();
        _pt_data->set_udp_sequence(udp_seq++);
        _pt_data->set_tail_crc();
        if (!SenderSocket->SendTo(_pt_data->pt_data(), point_data128::PTS_SIZE, BytesSent, *RemoteAddr_msop))
        {
            UE_LOG(LogTemp, Warning, TEXT("Hslidar128 send error: only sent %d bytes. errcode is %d"), BytesSent,
                ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->TranslateErrorCode(BytesSent));
        }
        // auto dt = (t - pretime)  / 1000;
        // if (dt > 100)
        //{
        //     SenderSocket->SendTo(_pt_data->gps_data(), point_data128::GPS_SIZE, BytesSent, *RemoteAddr_difop);
        //     pretime = t;
        // }
    }
}

uint32_t HSLidar128::package(const lidar_ptset& datas)
{
    // Calculate number of packets required for this set of data
    uint32_t channels_per_packet = getHorizontalScanMinUnit();
    uint32_t buf_num = datas.channels.size() / channels_per_packet;
    uint32_t drm = 250;
    uint32_t blkper = getReturnMode() == lidar::RT_Dual ? 2 : 1;
    // A group ahead process each block in the dataset
    for (uint32_t i = 0; i < buf_num; i++)
    {
        const channel_data* cd = &datas.channels.at(i * channels_per_packet);
        // Process all points within one horizontal scan unit
        for (uint32_t j = 0; j < channels_per_packet; j++)
        {
            float azimuth = getHorizontalScanAngle(cd->hor_pos);
            // Check if we have enough rays available for current block
            assert(cd->pn == getRaysNum() * blkper);
            // For each block within the horizontal scan unit...
            for (size_t b = 0; b < blkper; ++b)
            {
                _pt_data->set_block_azimuth(j * blkper + b, round(azimuth * 100.f));
                for (uint32_t k = 0; k < point_data128::CHANNELS_PER_BLOCK; k++)
                {
                    const auto& p = cd->points[b * point_data128::CHANNELS_PER_BLOCK + k];
                    _pt_data->set_channel_data(
                        j * blkper + b, k, (uint16_t) round(p.distance * drm), uint8_t(round(p.instensity)));
                }
            }
            // Move to next point data structure
            cd++;
        }
        // Send processed data to the host at appropriate time
        send_data(datas.channels.at(i * channels_per_packet).utime);
    }
    return buf_num;
}

uint32_t HSLidar128::getHorizontalScanMinUnit() const
{
    if (getReturnMode() == lidar::RT_Dual)
    {
        return point_data128::BLOCKS_PER_PACKAGE / 2;
    }
    return point_data128::BLOCKS_PER_PACKAGE;
}

point_data128at::point_data128at()
{
    memset(_data, 0, PTS_TOTAL_SIZE);
    uint8_t* data = _data;
    data[0] = 0xee;
    data[1] = 0xff;
    data[2] = 0x04;
    data[3] = 0x03;
    data = _data + PACKAGE_HEADER_SIZE;
    data[0] = 0x80;
    data[1] = 0x02;
    data[2] = 0x01;
    data[3] = 0x04;
    data[4] = 0x01;
    data[5] = 0x19;    // udp seq
    data = _data + PACKAGE_HEADER_SIZE + DATA_HEADER_SIZE;
    for (int i = 0; i < BLOCKS_PER_PACKAGE; i++)
    {
        for (int j = 0; j < CHANNELS_PER_BLOCK; j++)
        {
            _channel_data_ptr[i][j] = data + BLOCK_HEADER_SIZE + j * CHANNEL_SIZE;
        }
        data += BLOCK_SIZE;
    }

    _tail_ptr = _data + PACKAGE_HEADER_SIZE + DATA_HEADER_SIZE + PACKAGE_DATA_SIZE;
    //_tail_ptr[10] = 0x10;
    //_tail_ptr[11] = 0x02;
    _tail_ptr[24] = 0x37;
    _tail_ptr[25] = 0x42;
}

point_data128at::~point_data128at()
{
}

const uint8_t* point_data128at::pt_data() const
{
    return _data;
}

void point_data128at::set_block_azimuth(uint8_t bn, uint16_t azimuth1, uint8_t azimuth2)
{
    assert(bn < BLOCKS_PER_PACKAGE);
    uint8_t* data = _data + PACKAGE_HEADER_SIZE + DATA_HEADER_SIZE + BLOCK_SIZE * bn;
    data[0] = azimuth1 & 0xff;
    data[1] = azimuth1 >> 8;
    data[2] = azimuth2;
}

void point_data128at::set_channel_data(uint8_t bn, uint8_t cn, uint16_t distance, uint8_t reflectivity)
{
    assert(bn < BLOCKS_PER_PACKAGE);
    assert(cn < CHANNELS_PER_BLOCK);
    uint8_t*& data = _channel_data_ptr[bn][cn];
    data[0] = distance & 0xff;
    data[1] = distance >> 8;
    data[2] = reflectivity;
}
void point_data128at::set_data_crc()
{
    uint8_t* data = _data + PACKAGE_HEADER_SIZE + DATA_HEADER_SIZE;
    auto r = ueboost::crc(_data, BLOCK_SIZE * BLOCKS_PER_PACKAGE);
    data += BLOCK_SIZE * BLOCKS_PER_PACKAGE;
    data[0] = r & 0x000000ff;
    data[1] = (r & 0x0000ff00) >> 8;
    data[2] = (r & 0x00ff0000) >> 16;
    data[3] = (r & 0xff000000) >> 24;
}
void point_data128at::set_tail_crc()
{
    auto r = ueboost::crc(_tail_ptr, PACKAGE_TAIL_SIZE - 4);
    uint8_t* data = _tail_ptr + PACKAGE_TAIL_SIZE - 4;
    data[0] = r & 0x000000ff;
    data[1] = (r & 0x0000ff00) >> 8;
    data[2] = (r & 0x00ff0000) >> 16;
    data[3] = (r & 0xff000000) >> 24;
}

void point_data128at::set_return_mode(uint8_t return_mode)
{
    _tail_ptr[24] = return_mode;
    if (return_mode == 0x39)
    {
        _data[PACKAGE_HEADER_SIZE + 4] = 0x02;
    }
    else
    {
        _data[PACKAGE_HEADER_SIZE + 4] = 0x01;
    }
}

uint8_t point_data128at::get_return_mode() const
{
    return _tail_ptr[24];
}

void point_data128at::set_udp_sequence(uint32_t sqc)
{
    _tail_ptr[32] = sqc & 0x000000ff;
    _tail_ptr[33] = (sqc & 0x0000ff00) >> 8;
    _tail_ptr[34] = (sqc & 0x00ff0000) >> 16;
    _tail_ptr[35] = (sqc & 0xff000000) >> 24;
}

void point_data128at::set_frequency(uint8_t f)
{
    int ff = f * 20;
    _tail_ptr[18] = ff & 0xff;
    _tail_ptr[19] = ff >> 8;
}
void point_data128at::set_time(const utm_time& utime)
{
    utime.set_utc6(_tail_ptr + 26);
    utime.set_us(_tail_ptr + 20);
}

HSLidar128AT::HSLidar128AT()
{
    _pt_data = std::make_shared<point_data128at>();
    setRange(180);
    setRotationFrequency(10.f);
    setReturnMode(lidar::RT_Strongest);
}

HSLidar128AT::~HSLidar128AT()
{
    if (SenderSocket)    // Clear all sockets!
    {
        SenderSocket->Close();
        ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(SenderSocket);
    }
}

float HSLidar128AT::getRotationFrequency() const
{
    return _horizontal_resolution * 100.f;
}

lidar::ReturnMode HSLidar128AT::getReturnMode() const
{
    lidar::ReturnMode tmp[3] = {lidar::RT_Strongest, lidar::RT_Last, lidar::RT_Dual};
    return tmp[_pt_data->get_return_mode() - 0x37];
}

// 10hz 20hz

void HSLidar128AT::setRotationFrequency(float rf)
{
    _horizontal_resolution = 0.01f * rf;    // 0.1/10
    _pt_data->set_frequency(rf);
}

void HSLidar128AT::setReturnMode(lidar::ReturnMode rm)
{
    uint8_t tmp[3] = {0x39, 0x37, 0x38};
    _pt_data->set_return_mode(tmp[rm]);
}

bool HSLidar128AT::loadInterReference(const std::string& dir)
{
    // ini config
    {
        std::map<std::string, std::map<std::string, std::string>> config;
        std::string cfgfile = dir + "/cfg" + std::to_string(GetID()) + ".ini";
        UE_LOG(LogTemp, Log, TEXT("load Lidar HS128at: %s"), ANSI_TO_TCHAR(cfgfile.c_str()));
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
            dd = config["UDP"]["ptc_port"];
            if (!dd.empty())
            {
                //_tcp_pt_port = atoi(dd.c_str());
            }
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("load faild: %s"), ANSI_TO_TCHAR(cfgfile.c_str()));
        }
    }
    {
        std::fstream fin(dir + "/angle.dat", std::ios::binary | std::ios::in);
        if (!fin.good())
        {
            _error += "Error in angle.dat.";
            return false;
        }
        int length = 0;
        fin.seekg(0, std::ios::end);
        length = fin.tellg();
        fin.seekg(0, std::ios::beg);
        m_CalibrationBuffer.SetNum(length);
        fin.read((char*) m_CalibrationBuffer.GetData(), length);
        fin.close();

        uint8* p = m_CalibrationBuffer.GetData();
        PandarATCorrectionsHeader header = *(PandarATCorrectionsHeader*) p;
        if (0xee == header.delimiter[0] && 0xff == header.delimiter[1])
        {
            switch (header.version[1])
            {
                case 3:
                {
                    m_PandarAT_corrections.header = header;
                    auto frame_num = m_PandarAT_corrections.header.frame_number;
                    auto channel_num = m_PandarAT_corrections.header.channel_number;
                    p += sizeof(PandarATCorrectionsHeader);
                    memcpy((void*) &m_PandarAT_corrections.frame3.start_frame, p, sizeof(uint16_t) * frame_num);
                    p += sizeof(uint16_t) * frame_num;
                    memcpy((void*) &m_PandarAT_corrections.frame3.end_frame, p, sizeof(uint16_t) * frame_num);
                    p += sizeof(uint16_t) * frame_num;

                    memcpy((void*) &m_PandarAT_corrections.frame3.azimuth, p, sizeof(int16_t) * channel_num);
                    p += sizeof(int16_t) * channel_num;
                    memcpy((void*) &m_PandarAT_corrections.frame3.elevation, p, sizeof(int16_t) * channel_num);
                    p += sizeof(int16_t) * channel_num;
                    memcpy((void*) &m_PandarAT_corrections.frame3.azimuth_offset, p, sizeof(int8_t) * 36000);
                    p += sizeof(int8_t) * 36000;
                    memcpy((void*) &m_PandarAT_corrections.frame3.elevation_offset, p, sizeof(int8_t) * 36000);
                    p += sizeof(int8_t) * 36000;
                    memcpy((void*) &m_PandarAT_corrections.SHA256, p, sizeof(uint8_t) * 32);
                    p += sizeof(uint8_t) * 32;

                    for (int i = 0; i < 128; i++)
                    {
                        m_PandarAT_corrections.azimuth[i] = (double) m_PandarAT_corrections.frame3.azimuth[i] * 0.01;
                        m_PandarAT_corrections.elevation[i] =
                            (double) m_PandarAT_corrections.frame3.elevation[i] * 0.01;
                    }
                    for (int i = 0; i < 3; i++)
                    {
                        m_PandarAT_corrections.start_frame[i] = m_PandarAT_corrections.frame3.start_frame[i] * 0.01f;
                    }
                }
                break;
                case 5:
                {
                    m_PandarAT_corrections.header = header;
                    auto frame_num = m_PandarAT_corrections.header.frame_number;
                    auto channel_num = m_PandarAT_corrections.header.channel_number;
                    p += sizeof(PandarATCorrectionsHeader);
                    memcpy((void*) &m_PandarAT_corrections.frame5.start_frame, p, sizeof(uint32_t) * frame_num);
                    p += sizeof(uint32_t) * frame_num;
                    memcpy((void*) &m_PandarAT_corrections.frame5.end_frame, p, sizeof(uint32_t) * frame_num);
                    p += sizeof(uint32_t) * frame_num;
                    memcpy((void*) &m_PandarAT_corrections.frame5.azimuth, p, sizeof(int32_t) * channel_num);
                    p += sizeof(int32_t) * channel_num;
                    memcpy((void*) &m_PandarAT_corrections.frame5.elevation, p, sizeof(int32_t) * channel_num);
                    p += sizeof(int32_t) * channel_num;
                    auto adjust_length = channel_num * 180;
                    // memcpy((void*)&m_PandarAT_corrections.frame5.azimuth_offset, p,
                    //     sizeof(int8_t) * adjust_length);
                    memset(p, 0, sizeof(int8_t) * adjust_length);
                    p += sizeof(int8_t) * adjust_length;
                    // memcpy((void*)&m_PandarAT_corrections.frame5.elevation_offset, p,
                    //     sizeof(int8_t) * adjust_length);
                    memset(p, 0, sizeof(int8_t) * adjust_length);
                    p += sizeof(int8_t) * adjust_length;
                    // memcpy((void*)&m_PandarAT_corrections.SHA256, p,
                    //     sizeof(uint8_t) * 32);
                    memset(p, 0, sizeof(int8_t) * 32);
                    p += sizeof(uint8_t) * 32;

                    for (int i = 0; i < adjust_length; i++)
                    {
                        m_PandarAT_corrections.azimuth_offset[i] =
                            (double) m_PandarAT_corrections.frame5.azimuth_offset[i] *
                            (double) m_PandarAT_corrections.header.resolution * 0.01;
                        m_PandarAT_corrections.elevation_offset[i] =
                            (double) m_PandarAT_corrections.frame5.elevation_offset[i] *
                            (double) m_PandarAT_corrections.header.resolution * 0.01;
                    }
                    for (int i = 0; i < 128; i++)
                    {
                        m_PandarAT_corrections.azimuth[i] = (double) m_PandarAT_corrections.frame5.azimuth[i] *
                                                            m_PandarAT_corrections.header.resolution / 25600.0;
                        m_PandarAT_corrections.elevation[i] = (double) m_PandarAT_corrections.frame5.elevation[i] *
                                                              m_PandarAT_corrections.header.resolution / 25600.0;
                    }
                    for (int i = 0; i < 3; i++)
                    {
                        m_PandarAT_corrections.start_frame[i] = (double) m_PandarAT_corrections.frame5.start_frame[i] *
                                                                (double) m_PandarAT_corrections.header.resolution /
                                                                25600.0;
                    }
                }
                break;
                default:
                    break;
            }
        }
        else
        {
            return false;
        }
        // std::fstream fout(dir + "/angle.new.dat", std::ios::binary | std::ios::out);
        // fout.write((char*)m_CalibrationBuffer.GetData(), length);
        // fout.close();
    }
    _horizontal_angles.clear();
    _vertical_angles.clear();
    for (uint32_t i = 0; i < getHorizontalScanCount(); i++)
    {
        for (uint32_t j = 0; j < getRaysNum(); j++)
        {
            int aid = std::max(0, std::min(180, (int) (getHorizontalScanAngle(i) * 0.5 + 45.f)));
            _horizontal_angles.push_back(getHorizontalScanAngle(i) - m_PandarAT_corrections.azimuth[j] +
                                         m_PandarAT_corrections.azimuth_offset[j * 180 + aid]);
            _vertical_angles.push_back(
                m_PandarAT_corrections.elevation[j] + m_PandarAT_corrections.elevation_offset[j * 180 + aid]);
        }
    }

    return true;
}

bool HSLidar128AT::Init()
{
    if (_udp_IP.empty())
    {
        return true;
    }
    // FIPv4Endpoint Endpoint(FIPv4Address::Any, 6789);
    // Create Remote Address.
    RemoteAddr_msop = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();

    bool bIsValid = false;
    RemoteAddr_msop->SetIp(ANSI_TO_TCHAR(_udp_IP.c_str()), bIsValid);
    RemoteAddr_msop->SetPlatformPort(_udp_pt_port);

    if (!bIsValid)
    {
        _error = "UDP Sender: MSOP IP address was not valid,";
        _error += _udp_IP + ":" + std::to_string(_udp_pt_port);
        // return false;
    }
    int32 SendSize = point_data128at::PTS_TOTAL_SIZE * 10240;
    SenderSocket = FUdpSocketBuilder(TEXT("HSLidar128AT"))
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
    UE_LOG(LogTemp, Log, TEXT("Create Lidar HS128at: data ip=%s:%d"), ANSI_TO_TCHAR(_udp_IP.c_str()), _udp_pt_port);

    // check(SenderSocket->GetSocketType() == SOCKTYPE_Datagram);

    // Set Send Buffer Size
    SenderSocket->SetSendBufferSize(SendSize, SendSize);
    SenderSocket->SetReceiveBufferSize(SendSize, SendSize);

    // ptc server
    m_TcpPtcThread = MakeShared<FTcpReceiverThread>();
    m_TcpPtcThread->tcpDesc = TEXT("HSLidar128AT");
    m_TcpPtcThread->tcpPort = _udp_gps_port;
    m_TcpPtcThread->callBack = std::bind(&HSLidar128AT::PtcFunc, this, std::placeholders::_1, std::placeholders::_2);
    m_TcpPtcThread->CreateThread();

    return true;
}

utm_time HSLidar128AT::to_utime(std::time_t t) const
{
    utm_time ut;
    ut.us = (uint32_t) (t % 1000000);
    std::time_t unixTimestamp = t / 1000000;
    tm* ptm = std::localtime(&unixTimestamp);
    if (ptm)
    {
        ut.year = ptm->tm_year;
        ut.month = ptm->tm_mon + 1;
        ut.day = ptm->tm_mday;
        ut.hour = ptm->tm_hour;
        ut.min = ptm->tm_min;
        ut.sec = ptm->tm_sec;
    }
    return ut;
}

void HSLidar128AT::send_data(std::time_t t)
{
    if (SenderSocket)
    {
        int32 BytesSent = 0;
        _pt_data->set_time(to_utime(t));
        _pt_data->set_data_crc();
        _pt_data->set_udp_sequence(udp_seq++);
        _pt_data->set_tail_crc();
        if (!SenderSocket->SendTo(_pt_data->pt_data(), point_data128at::PTS_SIZE, BytesSent, *RemoteAddr_msop))
        {
            UE_LOG(LogTemp, Warning, TEXT("HSLidar128AT send error: only sent %d bytes. errcode is %d"), BytesSent,
                ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->TranslateErrorCode(BytesSent));
        }
    }
}

uint32_t HSLidar128AT::package(const lidar_ptset& datas)
{
    uint32_t channels_per_packet = getHorizontalScanMinUnit();
    uint32_t buf_num = datas.channels.size() / channels_per_packet;
    uint32_t drm = 250;
    uint32_t blkper = getReturnMode() == lidar::RT_Dual ? 2 : 1;
    // A group ahead
    for (uint32_t i = 0; i < buf_num; i++)
    {
        const channel_data* cd = &datas.channels.at(i * channels_per_packet);
        // Process all points within one horizontal scan unit
        for (uint32_t j = 0; j < channels_per_packet; j++)
        {
            if (cd->hor_pos == 0)
            {
                //********只使用前方镜片，否则引起抖动，因为3个镜片的偏角不完全一样，而getYawPitchAngle只考虑了前方镜片。
                framefix = (framefix + 1) % 3;
            }
            double azimuth = getHorizontalScanAngle(cd->hor_pos) + 90.f;
            azimuth *= 0.5;
            azimuth += m_PandarAT_corrections.start_frame[framefix];
            // Check if we have enough rays available for current block
            assert(cd->pn == getRaysNum() * blkper);
            // For each block within the horizontal scan unit...
            for (size_t b = 0; b < blkper; ++b)
            {
                _pt_data->set_block_azimuth(
                    j * blkper + b, floor(azimuth * 100), round(azimuth * 25600 - (int) (azimuth * 25600)));
                // Set distance and intensity values for each channel
                for (uint32_t k = 0; k < point_data128at::CHANNELS_PER_BLOCK; k++)
                {
                    const auto& p = cd->points[b * point_data128at::CHANNELS_PER_BLOCK + k];
                    // Round distances to avoid floating-point precision issues
                    _pt_data->set_channel_data(
                        j * blkper + b, k, (uint16_t) round(p.distance * drm), uint8_t(round(p.instensity)));
                }
            }
            // Move to next point data structure
            cd++;
        }
        // Send processed data to the host at appropriate time
        send_data(datas.channels.at(i * channels_per_packet).utime);
    }
    return buf_num;
}

uint32_t HSLidar128AT::getHorizontalScanMinUnit() const
{
    if (getReturnMode() == lidar::RT_Dual)
    {
        return point_data128at::BLOCKS_PER_PACKAGE / 2;
    }
    return point_data128at::BLOCKS_PER_PACKAGE;
}

// 获取水平扫描个数
uint32_t HSLidar128AT::getHorizontalScanCount() const
{
    return floor(120.f / getHorizontalResolution());
}

// 获取水平扫描角度
float HSLidar128AT::getHorizontalScanAngle(uint32_t pos) const
{
    return -60.f + pos * getHorizontalResolution();
}

std::pair<float, float> HSLidar128AT::getYawPitchAngle(uint32_t pos, uint32_t r) const
{
    return std::make_pair(_horizontal_angles.at(pos * getRaysNum() + r), _vertical_angles.at(pos * getRaysNum() + r));
}

// 处理ptc响应
void HSLidar128AT::PtcFunc(const TArray<uint8>& in, TArray<uint8>& out)
{
    if (in.Num() < 8)
    {
        return;
    }
    if (in[0] != 0x47 || in[1] != 0x74)
    {
        return;
    }
    // 获取内参
    if (in[2] == 0x05)    // PTC_COMMAND_GET_LIDAR_CALIBRATION
    {
        UE_LOG(LogTemp, Log, TEXT("PTC Reviced: PTC_COMMAND_GET_LIDAR_CALIBRATION"));
        out.SetNumZeroed(8);
        out[0] = 0x47;
        out[1] = 0x74;
        out[2] = 0x05;
        *(uint32*) &out[4] = m_CalibrationBuffer.Num();
        out.Append(m_CalibrationBuffer);
    }
    else
    {
        // 其他还没有支持
        UE_LOG(LogTemp, Warning, TEXT("PTC Service: not support %x for now."), in[2]);
        out.SetNumZeroed(8);
        out[0] = 0x47;
        out[1] = 0x74;
        out[2] = in[2];
        out[3] = 0x05;
    }
}

}    // namespace hslidar
