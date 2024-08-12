#include "RSLidar.h"
#include <sstream>
#include <fstream>
#include <algorithm>

namespace rslidar
{

#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

utc_time to_utime(std::time_t t)
{
    utc_time ut;
    ut.micro = (int) (t % 1000000);
    ut.t = t / 1000000;

    return ut;
}

void split_string(const std::string& s, const std::string& delim, std::vector<std::string>& ret)
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

bool readini(const std::string& ini, std::map<std::string, std::map<std::string, std::string>>& data)
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
            if (!node.empty() && !str_root.empty())
            {
                data.insert(make_pair(str_root, node));
            }
            // cout << str_line.substr(left_pos+1, right_pos-1) << endl;
            // If we have reached the end of this section, insert it into our tree structure
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

void utc_time::set(uint8_t* data) const
{
    data[0] = t >> 40;
    data[1] = (t & 0xff00000000) >> 32;
    data[2] = (t & 0xff000000) >> 24;
    ;
    data[3] = (t & 0xff0000) >> 16;
    ;
    data[4] = (t & 0xff00) >> 8;
    ;
    data[5] = t & 0xff;
    data[6] = (micro & 0xff000000) >> 24;
    ;
    data[7] = (micro & 0xff0000) >> 16;
    ;
    data[8] = (micro & 0xff00) >> 8;
    ;
    data[9] = micro & 0xff;
}
utm_time::utm_time(const utc_time& t)
{
    auto tt = t.t;
    tm* ptm = std::localtime(&tt);
    if (ptm)
    {
        year = ptm->tm_year + 1900 - 2000;
        month = ptm->tm_mon + 1;
        day = ptm->tm_mday;
        hour = ptm->tm_hour;
        min = ptm->tm_min;
        sec = ptm->tm_sec;
    }
    ms = t.micro / 1000;
    us = t.micro % 1000;
}

void utm_time::set(uint8_t* data) const
{
    data[0] = year;
    data[1] = month;
    data[2] = day;
    data[3] = hour;
    data[4] = min;
    data[5] = sec;
    data[6] = ms >> 8;
    data[7] = ms & 0xff;
    data[8] = us >> 8;
    data[9] = us & 0xff;
}

msop_data::msop_data()
{
}

msop_data16::msop_data16()
{
    memset(_data, 0, MSOP_SIZE);
    _data[0] = 0x55;
    _data[1] = 0xaa;
    _data[2] = 0x05;
    _data[3] = 0x0a;
    _data[4] = 0x5a;
    _data[5] = 0xa5;
    _data[6] = 0x50;
    _data[7] = 0xa0;
    _data[30] = 0x01;
    _data[MSOP_SIZE - 1] = 0xff;
    uint8_t* data = _data + MSOP_HEADER_SIZE;
    for (int i = 0; i < BLOCKS_PER_MSOP; i++)
    {
        data[0] = 0xff;
        data[1] = 0xee;

        for (int j = 0; j < CHANNELS_PER_BLOCK; j++)
        {
            _channel_data_ptr[i][j] = data + BLOCK_HEADER_SIZE + j * CHANNEL_SIZE;
        }
        data += BLOCK_SIZE;
    }
}

void msop_data16::copy(void* d) const
{
    memcpy(d, _data, MSOP_SIZE);
}

const uint8_t* msop_data16::data() const
{
    return _data;
}

void msop_data16::set_header_time(const utc_time& utime)
{
    utm_time(utime).set(_data + 20);
}

void msop_data16::set_temperature(float temp)
{
    uint16_t tt = abs(round(temp)) * 16.f;    // 1/0.0625
    _data[38] = (tt % 32) << 3;
    _data[39] = (tt / 32) | (temp < 0 ? 0x80 : 0);
}

void msop_data16::set_block_azimuth(uint8_t bn, uint16_t azimuth)
{
    assert(bn < BLOCKS_PER_MSOP);
    uint8_t* data = _data + MSOP_HEADER_SIZE + BLOCK_SIZE * bn;
    data[2] = azimuth >> 8;
    data[3] = azimuth & 0xff;
}

void msop_data16::set_channel_data(uint8_t bn, uint8_t cn, uint16_t distance, uint8_t reflectivity)
{
    assert(bn < BLOCKS_PER_MSOP);
    assert(cn < CHANNELS_PER_BLOCK);
    uint8_t*& data = _channel_data_ptr[bn][cn];
    data[0] = distance >> 8;
    data[1] = distance & 0xff;
    data[2] = reflectivity;
}

msop_data32::msop_data32()
{
    _data[30] = 0x02;
}

msop_data_bpearl::msop_data_bpearl()
{
    _data[0] = 0x55;
    _data[1] = 0xaa;
    _data[2] = 0x05;
    _data[3] = 0x0a;
    _data[4] = 0x5a;
    _data[5] = 0xa5;
    _data[6] = 0x50;
    _data[7] = 0xa0;
    _data[30] = 0x03;
}
msop_data_helios::msop_data_helios()
{
    _data[0] = 0x55;
    _data[1] = 0xaa;
    _data[2] = 0x05;
    _data[3] = 0x5A;
    _data[4] = 0x00;
    _data[5] = 0x01;
    _data[30] = 0x06;
}

void msop_data_helios::set_header_time(const utc_time& utime)
{
    utime.set(_data + 20);
}

msop_data_ruby::msop_data_ruby()
{
    memset(_data, 0, MSOP_SIZE);
    _data[0] = 0x55;
    _data[1] = 0xaa;
    _data[2] = 0x05;
    _data[3] = 0x5A;
    _data[4] = 0x00;
    _data[5] = 0x01;
    _data[7] = 0x01;
    _data[30] = 0x04;
    uint8_t* data = _data + MSOP_HEADER_SIZE;
    for (int i = 0; i < BLOCKS_PER_MSOP; i++)
    {
        data[0] = 0xFE;
        data[1] = 0x01;

        for (int j = 0; j < CHANNELS_PER_BLOCK; j++)
        {
            _channel_data_ptr[i][j] = data + BLOCK_HEADER_SIZE + j * CHANNEL_SIZE;
        }
        data += BLOCK_SIZE;
    }
}

void msop_data_ruby::copy(void* d) const
{
    memcpy(d, _data, MSOP_SIZE);
}

const uint8_t* msop_data_ruby::data() const
{
    return _data;
}

void msop_data_ruby::set_header_time(const utc_time& utime)
{
    utime.set(_data + 10);
}

void msop_data_ruby::set_temperature(float temp)
{
    uint16_t tt = abs(round(temp)) * 16.f;    // 1/0.0625
    _data[38] = (tt % 32) << 3;
    _data[39] = (tt / 32) | (temp < 0 ? 0x80 : 0);
}

void msop_data_ruby::set_block_azimuth(uint8_t bn, uint16_t azimuth)
{
    assert(bn < BLOCKS_PER_MSOP);
    uint8_t* data = _data + MSOP_HEADER_SIZE + BLOCK_SIZE * bn;
    data[2] = azimuth >> 8;
    data[3] = azimuth & 0xff;
}

void msop_data_ruby::set_channel_data(uint8_t bn, uint8_t cn, uint16_t distance, uint8_t reflectivity)
{
    assert(bn < BLOCKS_PER_MSOP);
    assert(cn < CHANNELS_PER_BLOCK);
    uint8_t*& data = _channel_data_ptr[bn][cn];
    data[0] = distance >> 8;
    data[1] = distance & 0xff;
    data[2] = reflectivity;
}

msop_data_m1::msop_data_m1()
{
    memset(_data, 0, MSOP_SIZE);
    _data[0] = 0x55;
    _data[1] = 0xaa;
    _data[2] = 0x5a;
    _data[3] = 0xa5;
    _data[8] = 0x06;

    _data[30] = 0x10;
    uint8_t* data = _data + MSOP_HEADER_SIZE;
    for (int i = 0; i < BLOCKS_PER_MSOP; i++)
    {
        for (int j = 0; j < CHANNELS_PER_BLOCK; j++)
        {
            _channel_data_ptr[i][j] = data + BLOCK_HEADER_SIZE + j * CHANNEL_SIZE;
        }
        data += BLOCK_SIZE;
    }
}

void msop_data_m1::copy(void* d) const
{
    memcpy(d, _data, MSOP_SIZE);
}

const uint8_t* msop_data_m1::data() const
{
    return _data;
}

void msop_data_m1::set_header_time(const utc_time& utime, uint32_t psn)
{
    utime.set(_data + 10);
    _data[4] = psn >> 8;
    _data[5] = psn & 0xff;
}

void msop_data_m1::set_return_mode(uint8_t return_mode)
{
    _data[8] = return_mode;
}

uint8_t msop_data_m1::get_return_mode() const
{
    return _data[8];
}

void msop_data_m1::set_temperature(float temp)
{
    int tt = round(temp) + 80;
    _data[31] = tt;
}

void msop_data_m1::set_block_azimuth(uint8_t bn, uint8_t toffset, uint8_t ret_seq)
{
    assert(bn < BLOCKS_PER_MSOP);
    uint8_t* data = _data + MSOP_HEADER_SIZE + BLOCK_SIZE * bn;
    data[0] = toffset;
    data[1] = ret_seq;
}

void msop_data_m1::set_channel_data(
    uint8_t bn, uint8_t cn, uint16_t pitch, uint16_t yaw, uint16_t distance, uint8_t reflectivity)
{
    assert(bn < BLOCKS_PER_MSOP);
    assert(cn < CHANNELS_PER_BLOCK);
    uint8_t*& data = _channel_data_ptr[bn][cn];
    data[0] = distance >> 8;
    data[1] = distance & 0xff;
    data[2] = pitch >> 8;
    data[3] = pitch & 0xff;
    data[4] = yaw >> 8;
    data[5] = yaw & 0xff;
    data[6] = reflectivity;
}

difop_data::difop_data()
{
    memset(_data, 0, DIFOP_SIZE);
    _data[0] = 0xa5;
    _data[1] = 0xff;
    _data[2] = 0x00;
    _data[3] = 0x5a;
    _data[4] = 0x11;
    _data[5] = 0x11;
    _data[6] = 0x55;
    _data[7] = 0x55;
    _data[1246] = 0x0f;
    _data[1247] = 0xf0;
}

void difop_data::copy(void* d) const
{
    memcpy(d, _data, DIFOP_SIZE);
}

const uint8_t* difop_data::data() const
{
    return _data;
}

////1200,600,300;clockwise rotation;

void difop_data::set_mot_spd(uint16_t mot_spd)
{
    _data[8] = mot_spd >> 8;
    _data[9] = mot_spd & 0xff;
}

uint16_t difop_data::get_mot_spd() const
{
    return (_data[8] << 8) + _data[9];
}

// eth

void difop_data::set_eth(
    uint8_t lidar_ip[4], uint8_t dest_pc_ip[4], uint8_t mac_addr[6], uint16_t msop_port, uint16_t difop_port)
{
    uint8_t* eth = _data + 10;
    memcpy(eth + 0, lidar_ip, 4);
    memcpy(eth + 4, dest_pc_ip, 4);
    memcpy(eth + 8, mac_addr, 6);
    eth[14] = eth[16] = msop_port >> 8;
    eth[15] = eth[17] = msop_port & 0xff;
    eth[18] = eth[20] = difop_port >> 8;
    eth[19] = eth[21] = difop_port & 0xff;
}

// du,*100

void difop_data::set_hor_fov(uint16_t fov_start, uint16_t fov_end)
{
    uint8_t* fov = _data + 32;
    fov[0] = fov_start >> 8;
    fov[1] = fov_start & 0xff;
    fov[2] = fov_end >> 8;
    fov[3] = fov_end & 0xff;
}

void difop_data::set_mot_phase(uint16_t mot_phase)
{
    _data[38] = mot_phase >> 8;
    _data[39] = mot_phase & 0xff;
}

void difop_data::set_frm(uint8_t top_frm[5], uint8_t bot_frm[5])
{
    memcpy(_data + 40, top_frm, 5);
    memcpy(_data + 45, bot_frm, 5);
}

void difop_data::set_intensity(uint8_t intensity_scale, uint8_t intensity_mode)    //=1 2 3
{
    _data[290] = intensity_scale;
    switch (intensity_mode)
    {
        case 1:
            _data[291] = 0xa1;
            break;
        case 2:
            _data[291] = 0xb1;
            break;
        case 3:
            _data[291] = 0xc1;
            break;
        default:
            break;
    }
}

dis_resolution_mode difop_data::get_dis_resolution_mode() const
{
    return ((_data[41] == 0xff && _data[42] == 0xff && _data[43] == 0xff) ||
               (_data[41] == 0x55 && _data[42] == 0xaa && _data[43] == 0x5a) ||
               (_data[41] == 0xe9 && _data[42] == 0x01 && _data[43] == 0x00))
               ? cm_10
               : cm_05;
}

uint8_t difop_data::get_intensity_mode() const
{
    switch (_data[291])
    {
        case 0xa1:
            return 1;
        case 0xb1:
            return 2;
        case 0xc1:
            return 3;
        default:
            break;
            return 1;
    }
    return _data[291];
}

void difop_data::set_sn(uint8_t sn[6])
{
    memcpy(_data + 292, sn, 6);
}

void difop_data::set_zero_angle_offset(uint16_t zero_angle_offset)
{
    _data[298] = zero_angle_offset >> 8;
    _data[299] = zero_angle_offset & 0xff;
}

void difop_data::set_return_mode(uint8_t return_mode)
{
    _data[300] = return_mode;
}

uint8_t difop_data::get_return_mode() const
{
    return _data[300];
}

// Upper computer compatibility

void difop_data::set_upper_cc(uint16_t upper_cc)
{
    _data[301] = upper_cc >> 8;
    _data[302] = upper_cc & 0xff;
}

void difop_data::set_gprmc(const char* gpr)
{
    if (gpr)
        memcpy(_data + 382, gpr, strlen(gpr));
    else
        memset(_data + 382, 0, 86);
}

void difop_data::set_time(const utc_time& utime)
{
    utm_time(utime).set(_data + 303);
}

// Corrected intensity curves coefficient
// Corrected static
// Corrected vertical angle

void difop_data16::set_cor_vert_ang(const std::vector<float> angles)
{
    assert(angles.size() == 16);
    uint8_t* d = _data + 1165;
    for (int i = 0; i < 16; i++)
    {
        int aa = abs(round(angles[i] * 10000));
        d[0] = aa >> 16;
        aa = aa & 0xffff;
        d[1] = aa >> 8;
        d[2] = aa & 0xff;
        d += 3;
    }
}

void difop_data32::set_cor_vert_ang(const std::vector<float> angles)
{
    assert(angles.size() == 32);
    uint8_t* d = _data + 468;
    for (int i = 0; i < 32; i++)
    {
        int aa = round(angles[i] * 1000);
        d[0] = aa > 0 ? 0 : 1;
        d[1] = abs(aa) / 0xff;
        d[2] = abs(aa) % 0xff;
        d += 3;
    }
}

void difop_data32::set_cor_hori_ang(const std::vector<float> angles)
{
    assert(angles.size() == 32);
    uint8_t* d = _data + 564;
    for (int i = 0; i < 32; i++)
    {
        int16_t aa = round(angles[i] * 1000);
        d[0] = aa > 0 ? 0 : 1;
        d[1] = abs(aa) / 0xff;
        d[2] = abs(aa) % 0xff;
        d += 3;
    }
}
void difop_data_helios::set_return_mode(uint8_t return_mode)
{
    _data[300] = return_mode == 3 ? 0 : return_mode;
}

uint8_t difop_data_helios::get_return_mode() const
{
    return _data[300];
}

void difop_data_helios::set_cor_vert_ang(const std::vector<float> angles)
{
    assert(angles.size() == 32);
    uint8_t* d = _data + 468;
    for (int i = 0; i < 32; i++)
    {
        int aa = round(angles[i] * 100);
        d[0] = aa > 0 ? 0 : 1;
        d[1] = abs(aa) / 0xff;
        d[2] = abs(aa) % 0xff;
        d += 3;
    }
}

void difop_data_helios::set_cor_hori_ang(const std::vector<float> angles)
{
    assert(angles.size() == 32);
    uint8_t* d = _data + 564;
    for (int i = 0; i < 32; i++)
    {
        int16_t aa = round(angles[i] * 100);
        d[0] = aa > 0 ? 0 : 1;
        d[1] = abs(aa) / 0xff;
        d[2] = abs(aa) % 0xff;
        d += 3;
    }
}

void difop_data_helios::set_time(const utc_time& utime)
{
    utime.set(_data + 303);
}

void difop_data_bpearl::set_cor_vert_ang(const std::vector<float> angles)
{
    assert(angles.size() == 32);
    uint8_t* d = _data + 468;
    for (int i = 0; i < 32; i++)
    {
        int aa = round(angles[i] * 100);
        d[0] = aa > 0 ? 0 : 1;
        d[1] = abs(aa) / 0xff;
        d[2] = abs(aa) % 0xff;
        d += 3;
    }
}

void difop_data_bpearl::set_cor_hori_ang(const std::vector<float> angles)
{
    assert(angles.size() == 32);
    uint8_t* d = _data + 564;
    for (int i = 0; i < 32; i++)
    {
        int16_t aa = round(angles[i] * 100);
        d[0] = aa > 0 ? 0 : 1;
        d[1] = abs(aa) / 0xff;
        d[2] = abs(aa) % 0xff;
        d += 3;
    }
}

void difop_data_ruby::set_cor_vert_ang(const std::vector<float> angles)
{
    assert(angles.size() == 128);
    uint8_t* d = _data + 468;
    for (int i = 0; i < 128; i++)
    {
        int aa = round(angles[i] * 100);
        d[0] = aa > 0 ? 0 : 1;
        d[1] = abs(aa) / 0xff;
        d[2] = abs(aa) % 0xff;
        d += 3;
    }
}

void difop_data_ruby::set_time(const utc_time& utime)
{
    utime.set(_data + 303);
}

void difop_data_ruby::set_cor_hori_ang(const std::vector<float> angles)
{
    assert(angles.size() == 128);
    uint8_t* d = _data + 852;
    for (int i = 0; i < 128; i++)
    {
        int16_t aa = round(angles[i] * 100);
        d[0] = aa > 0 ? 0 : 1;
        d[1] = abs(aa) / 0xff;
        d[2] = abs(aa) % 0xff;
        d += 3;
    }
}

difop_data_ss::difop_data_ss()
{
    memset(_data, 0, DIFOP_SIZE);
    _data[0] = 0xa5;
    _data[1] = 0xff;
    _data[2] = 0x00;
    _data[3] = 0x5a;
    _data[4] = 0x11;
    _data[5] = 0x11;
    _data[6] = 0x55;
    _data[7] = 0x55;
}

void difop_data_ss::copy(void* d) const
{
    memcpy(d, _data, DIFOP_SIZE);
}
const uint8_t* difop_data_ss::data() const
{
    return _data;
}

void difop_data_ss::set_frequency(uint8_t f)
{
    _data[9] = f;
}
// du,*100
void difop_data_ss::set_hor_fov(uint16_t start, uint16_t end)
{
    _data[28] = start >> 8;
    _data[29] = start & 0xff;
    _data[30] = end >> 8;
    _data[31] = end & 0xff;
}
void difop_data_ss::set_ele_fov(uint16_t start, uint16_t end)
{
    _data[32] = start >> 8;
    _data[33] = start & 0xff;
    _data[34] = end >> 8;
    _data[35] = end & 0xff;
}

void difop_data_ss::set_eth(
    uint8_t lidar_ip[4], uint8_t dest_pc_ip[4], uint8_t mac_addr[6], uint16_t msop_port, uint16_t difop_port)
{
    uint8_t* eth = _data + 10;
    memcpy(eth + 0, lidar_ip, 4);
    memcpy(eth + 4, dest_pc_ip, 4);
    memcpy(eth + 8, mac_addr, 6);
    eth[14] = msop_port >> 8;
    eth[15] = msop_port & 0xff;
    eth[16] = difop_port >> 8;
    eth[17] = difop_port & 0xff;
}

void difop_data_ss::set_sn(uint8_t sn[6])
{
    memcpy(_data + 46, sn, 6);
}
void difop_data_ss::set_return_mode(uint8_t return_mode)
{
    _data[52] = return_mode;
}
uint8_t difop_data_ss::get_return_mode() const
{
    return _data[52];
}

void difop_data_ss::set_time(const utc_time& utime)
{
    utm_time(utime).set(_data + 55);
}

RSLidar::RSLidar() : _range(200.f), _horizontal_resolution(0.2f), SenderSocket(nullptr), pretime(0)
{
}

RSLidar::~RSLidar()
{
    if (SenderSocket)    // Clear all sockets!
    {
        if (!SenderSocket->Close())
        {
            UE_LOG(LogTemp, Warning, TEXT("Rslidar close socket faild."));
        }
        ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(SenderSocket);
    }
}

float RSLidar::getRotationFrequency() const
{
    return _difop_data->get_mot_spd() / 60.f;
}

float RSLidar::getRange() const
{
    return _range;
}

lidar::ReturnMode RSLidar::getReturnMode() const
{
    return (lidar::ReturnMode) _difop_data->get_return_mode();
}

float RSLidar::getHorizontalResolution() const
{
    return _horizontal_resolution;
}

std::pair<float, float> RSLidar::getYawPitchAngle(uint32_t pos, uint32_t r) const
{
    return std::make_pair(getHorizontalScanAngle(pos) + _horizontal_angles.at(r), _vertical_angles.at(r));
}

void RSLidar::setRange(float r)
{
    _range = r;
}

// 5hz 10hz 20hz

void RSLidar::setRotationFrequency(float rf)
{
    _difop_data->set_mot_spd((uint16_t) round(rf * 60.f));
}

void RSLidar::setReturnMode(lidar::ReturnMode rm)
{
    _difop_data->set_return_mode(rm);
}

const uint8_t* RSLidar::getDifopBuf() const
{
    return _difop_data->_data;
}

bool RSLidar::loadInterReference(const std::string& dir)
{
    _temperature = MAX(MIN(_temperature, 71), TEMPERATURE_MIN);
    _msop_data->set_temperature(_temperature);
    _difop_data->set_mot_phase(0);
    _difop_data->set_return_mode(lidar::RT_Strongest);
    _difop_data->set_hor_fov(0, 36000);
    _difop_data->set_intensity(0, 3);

    // ini config
    {
        std::map<std::string, std::map<std::string, std::string>> config;
        std::string cfgfile = dir + "/cfg" + std::to_string(GetID()) + ".ini";
        UE_LOG(LogTemp, Log, TEXT("load Lidar rs: %s"), ANSI_TO_TCHAR(cfgfile.c_str()));
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
            dd = config["UDP"]["difop_port"];
            if (!dd.empty())
            {
                _udp_difop_port = atoi(dd.c_str());
            }
            dd = config["UDP"]["msop_port"];
            if (!dd.empty())
            {
                _udp_msop_port = atoi(dd.c_str());
            }

            _difop_data->set_eth(lidar_ip, dest_pc_ip, mac_addr, _udp_msop_port, _udp_difop_port);
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("load faild: %s"), ANSI_TO_TCHAR(cfgfile.c_str()));
        }
    }
    _horizontal_angles.reserve(getRaysNum());
    _vertical_angles.reserve(getRaysNum());
    _horizontal_angles.clear();
    _vertical_angles.clear();

    // load angle.csv
    {
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
                    _error += "Error in angle.csv.";
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
                _error += "Error in angle.csv.";
                return false;
            }
        }
        else
        {
            _error += "cannot open angle.csv.";
            return false;
        }
    }
    if (_vertical_angles.empty() || _horizontal_angles.empty())
    {
        return false;
    }

    _difop_data->set_cor_vert_ang(_vertical_angles);
    _difop_data->set_cor_hori_ang(_horizontal_angles);
    return true;
}

bool RSLidar::setAngleFromString(const FString& str)
{
    if (str.IsEmpty())
    {
        return false;
    }
    _horizontal_angles.clear();
    _vertical_angles.clear();
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

bool RSLidar::setIP(const FString& ip, const FString& port)
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

    _udp_msop_port = std::atoi(data[0].c_str());
    _udp_difop_port = std::atoi(data[1].c_str());
    return true;
}

bool RSLidar::Init()
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
    RemoteAddr_msop->SetPlatformPort(_udp_msop_port);

    if (!bIsValid)
    {
        _error = "UDP Sender: MSOP IP address was not valid,";
        _error += _udp_IP + ":" + std::to_string(_udp_msop_port);
        // return false;
    }
    RemoteAddr_difop->SetIp(ANSI_TO_TCHAR(_udp_IP.c_str()), bIsValid);
    RemoteAddr_difop->SetPlatformPort(_udp_difop_port);

    if (!bIsValid)
    {
        _error = "UDP Sender: DIFOP IP address was not valid,";
        _error += _udp_IP + ":" + std::to_string(_udp_difop_port);
        // return false;
    }
    int32 SendSize = MAX(_msop_data->length(), difop_data::DIFOP_SIZE) * 10240;
    SenderSocket = FUdpSocketBuilder(TEXT("RSLidar"))
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

    UE_LOG(LogTemp, Log, TEXT("Create Lidar RS: msop ip=%s:%d, difop ip=%s:%d"), ANSI_TO_TCHAR(_udp_IP.c_str()),
        _udp_msop_port, ANSI_TO_TCHAR(_udp_IP.c_str()), _udp_difop_port);
    // check(SenderSocket->GetSocketType() == SOCKTYPE_Datagram);

    // Set Send Buffer Size
    SenderSocket->SetSendBufferSize(SendSize, SendSize);
    SenderSocket->SetReceiveBufferSize(SendSize, SendSize);
    return true;
}
void RSLidar::send_data(std::time_t t)
{
    if (SenderSocket)
    {
        int32 BytesSent = 0;
        int32 faildnum = 0;
        while (!SenderSocket->SendTo(_msop_data->data(), _msop_data->length(), BytesSent, *RemoteAddr_msop))
        {
            BytesSent = 0;
            FPlatformProcess::Sleep(0.00001);
            faildnum++;
            if (faildnum == 100)
            {
                UE_LOG(LogTemp, Warning, TEXT("Rslidar send msop error: only sent %d bytes. errcode is %d"), BytesSent,
                    ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->TranslateErrorCode(BytesSent));
                break;
            }
        }

        auto dt = (t - pretime) / 1000;
        // UE_LOG(LogTemp, Log, TEXT("Rsslidar dt: %lld"), dt);
        if (dt >= 100)
        {
            _difop_data->set_time(to_utime(t));
            _difop_data->set_gprmc(getGPRMC());
            BytesSent = 0;
            if (!SenderSocket->SendTo(_difop_data->data(), difop_data::DIFOP_SIZE, BytesSent, *RemoteAddr_difop))
            {
                UE_LOG(LogTemp, Warning, TEXT("Rsslidar send difop error: only sent %d bytes. errcode is %d"),
                    BytesSent, ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->TranslateErrorCode(BytesSent));
            }
            pretime = t;
        }
    }
}
RSLidar16::RSLidar16()
{
    memset(_dChannelNum, 0, sizeof(_dChannelNum));
    _msop_data = std::make_shared<msop_data16>();
    _difop_data = std::make_shared<difop_data16>();
    setRange(120);
    setRotationFrequency(10.f);
    setReturnMode(lidar::RT_Strongest);
}

inline lidar::LidarType RSLidar16::getType() const
{
    return lidar::LT_RSLiDAR16;
}

uint32_t RSLidar16::getHorizontalScanMinUnit() const
{
    if (getReturnMode() == lidar::RT_Dual)
    {
        return msop_data16::BLOCKS_PER_MSOP;
    }
    return msop_data16::BLOCKS_PER_MSOP * 2;
}

float RSLidar16::getLaserRadius() const
{
    return 0.04638f;
}

float RSLidar16::getLaserHeight() const
{
    return 0.010875f;
}

bool RSLidar16::loadInterReference(const std::string& dir)
{
    if (!RSLidar::loadInterReference(dir))
    {
        return false;
    }
    uint8_t top_frm[5] = {0x00, 0x55, 0xaa, 0x5a, 0xa5}, bot_frm[5] = {0x08, 0x02, 0x01, 0x06, 0xf0};
    _difop_data->set_frm(top_frm, bot_frm);

    _horizontal_angles.resize(16, 0);
    _vertical_angles.resize(16, 0);

    // load channelnum.csv

    {
        FILE* f_channel = 0;
#ifdef WIN32

        if (0 == fopen_s(&f_channel, (dir + "/ChannelNum.csv").c_str(), "r") && f_channel)
#else
        f_channel = fopen((dir + "/ChannelNum.csv").c_str(), "r");
        if (f_channel)
#endif    //
        {
            uint32_t loopi = 0;
            while (!feof(f_channel))
            {
                if (loopi > getRaysNum())
                    break;
                int* c = _dChannelNum[loopi];
                int tmp = fscanf(f_channel,
                    "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%"
                    "d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                    &c[0], &c[1], &c[2], &c[3], &c[4], &c[5], &c[6], &c[7], &c[8], &c[9], &c[10], &c[11], &c[12],
                    &c[13], &c[14], &c[15], &c[16], &c[17], &c[18], &c[19], &c[20], &c[21], &c[22], &c[23], &c[24],
                    &c[25], &c[26], &c[27], &c[28], &c[29], &c[30], &c[31], &c[32], &c[33], &c[34], &c[35], &c[36],
                    &c[37], &c[38], &c[39], &c[40]);
                if (tmp != 41)
                {
                    _error += "Error in channelnum.csv.";
                    return false;
                }
                loopi++;
            }
            fclose(f_channel);
        }
    }

    return true;
}

void RSLidar16::setRotationFrequency(float rf)
{
    _horizontal_resolution = 0.01875f * rf;    // 0.09/5------
    RSLidar::setRotationFrequency(rf);
}

bool RSLidar16::Init()
{
    float degr = getHorizontalResolution() / 16.f;
    for (size_t i = 0; i < _horizontal_angles.size(); i++)
    {
        _horizontal_angles.at(i) += degr * i;
    }
    return RSLidar::Init();
}

uint32_t RSLidar16::package(const lidar_ptset& datas)
{
    // Calculate number of packets required for this set of data
    uint32_t channels_per_block = getReturnMode() == lidar::RT_Dual ? 1 : 2;
    uint32_t channels_per_packet = msop_data16::BLOCKS_PER_MSOP * channels_per_block;
    uint32_t buf_num = datas.channels.size() / channels_per_packet;
    uint32_t drm = _difop_data->get_dis_resolution_mode() == cm_10 ? 100 : 200;
    for (uint32_t i = 0; i < buf_num; ++i)
    {
        const channel_data* cd = &datas.channels.at(i * channels_per_packet);
        _msop_data->set_header_time(to_utime(cd->utime));
        for (uint32_t j = 0; j < msop_data16::BLOCKS_PER_MSOP; j++)
        {
            float azimuth = getHorizontalScanAngle(cd->hor_pos);
            _msop_data->set_block_azimuth(j, round(azimuth * 100.f));
            for (size_t c = 0; c < channels_per_block; ++c)
            {
                assert(cd->pn == getRaysNum() * 2 / channels_per_block);
                for (uint32_t k = 0, ks = cd->pn; k < ks; k++)
                {
                    const auto& p = cd->points[k];
                    // Round distances to avoid floating-point precision issues
                    _msop_data->set_channel_data(j, c * ks + k, distance2pixel(p.distance * drm, k % getRaysNum()),
                        reflectivity_calibration(p.instensity));
                }
                // Move to next point data structure
                cd++;
            }
        }
        // Send processed data to the host at appropriate time
        send_data(datas.channels.at(i * channels_per_packet).utime);
    }
    return buf_num;
}

uint8_t RSLidar16::reflectivity_calibration(float instensity) const
{
    return uint8_t(round(instensity));
}

uint16_t RSLidar16::distance2pixel(float distance, uint32_t passageway) const
{
    return (uint16_t) (round(distance)) + _dChannelNum[passageway][_temperature - TEMPERATURE_MIN];
}

RSLidar32::RSLidar32()
{
    memset(_dChannelNum, 0, sizeof(_dChannelNum));
    _msop_data = std::make_shared<msop_data32>();
    _difop_data = std::make_shared<difop_data32>();
    setRange(200);
    setRotationFrequency(10.f);
    setReturnMode(lidar::RT_Strongest);
}

uint32_t RSLidar32::getHorizontalScanMinUnit() const
{
    if (getReturnMode() == lidar::RT_Dual)
    {
        return msop_data16::BLOCKS_PER_MSOP / 2;
    }
    return msop_data16::BLOCKS_PER_MSOP;
}

float RSLidar32::getLaserRadius() const
{
    return 0.04583f;
}

float RSLidar32::getLaserHeight() const
{
    return 0.010875f;
}

bool RSLidar32::loadInterReference(const std::string& dir)
{
    if (!RSLidar::loadInterReference(dir))
    {
        return false;
    }
    uint8_t top_frm[5] = {0x00, 0x09, 0x23, 0x09, 0x03}, bot_frm[5] = {0x08, 0x02, 0x01, 0x06, 0xf1};
    _difop_data->set_frm(top_frm, bot_frm);

    // load channelnum.csv

    {
        FILE* f_channel = 0;
#ifdef WIN32

        if (0 == fopen_s(&f_channel, (dir + "/ChannelNum.csv").c_str(), "r") && f_channel)
#else
        f_channel = fopen((dir + "/ChannelNum.csv").c_str(), "r");
        if (f_channel)
#endif    //

        {
            uint32_t loopi = 0;
            while (!feof(f_channel))
            {
                if (loopi > getRaysNum())
                    break;
                int* c = _dChannelNum[loopi];
                int tmp = fscanf(f_channel,
                    "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%"
                    "d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                    &c[0], &c[1], &c[2], &c[3], &c[4], &c[5], &c[6], &c[7], &c[8], &c[9], &c[10], &c[11], &c[12],
                    &c[13], &c[14], &c[15], &c[16], &c[17], &c[18], &c[19], &c[20], &c[21], &c[22], &c[23], &c[24],
                    &c[25], &c[26], &c[27], &c[28], &c[29], &c[30], &c[31], &c[32], &c[33], &c[34], &c[35], &c[36],
                    &c[37], &c[38], &c[39], &c[40], &c[41], &c[42], &c[43], &c[44], &c[45], &c[46], &c[47], &c[48],
                    &c[49], &c[50]);
                if (tmp != 51)
                {
                    _error += "Error in channelnum.csv.";
                    return false;
                }
                loopi++;
            }
            fclose(f_channel);
        }
    }

    return true;
}

void RSLidar32::setRotationFrequency(float rf)
{
    _horizontal_resolution = 0.02f * rf;    // 0.1/5
    RSLidar::setRotationFrequency(rf);
}

bool RSLidar32::Init()
{
    float degr = getHorizontalResolution() / 16.f;
    for (size_t i = 0; i < 16u; i++)
    {
        _horizontal_angles.at(i) += degr * i;
        _horizontal_angles.at(i + 16) += degr * i;
    }
    return RSLidar::Init();
}

uint32_t RSLidar32 ::package(const lidar_ptset& datas)
{
    // Calculate number of packets required for this set of data
    uint32_t channels_per_packet = getHorizontalScanMinUnit();
    uint32_t buf_num = datas.channels.size() / channels_per_packet;
    uint32_t drm = _difop_data->get_dis_resolution_mode() == cm_10 ? 100 : 200;
    uint32_t blkper = getReturnMode() == lidar::RT_Dual ? 2 : 1;
    // A group ahead
    for (uint32_t i = 0; i < buf_num; i++)
    {
        const channel_data* cd = &datas.channels.at(i * channels_per_packet);

        _msop_data->set_header_time(to_utime(cd->utime));
        // Process all points within one horizontal scan unit
        for (uint32_t j = 0; j < channels_per_packet; j++)
        {
            assert(cd->pn == getRaysNum() * blkper);
            // Check if we have enough rays available for current block
            float azimuth = getHorizontalScanAngle(cd->hor_pos);
            // For each block within the horizontal scan unit...
            for (size_t b = 0; b < blkper; ++b)
            {
                _msop_data->set_block_azimuth(j * blkper + b, round(azimuth * 100.f));
                // Set distance and intensity values for each channel
                for (uint32_t k = 0; k < msop_data16::CHANNELS_PER_BLOCK; k++)
                {
                    const auto& p = cd->points[b * msop_data16::CHANNELS_PER_BLOCK + k];
                    // Round distances to avoid floating-point precision issues
                    _msop_data->set_channel_data(
                        j * blkper + b, k, distance2pixel(p.distance * drm, k), reflectivity_calibration(p.instensity));
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

uint8_t RSLidar32::reflectivity_calibration(float instensity) const
{
    // get_intensity_mode()==3
    return uint8_t(round(instensity));
}

uint16_t RSLidar32::distance2pixel(float distance, uint32_t passageway) const
{
    return (int) (round(distance)) + _dChannelNum[passageway][_temperature - TEMPERATURE_MIN];
}

RSBpearl::RSBpearl()
{
    _msop_data = std::make_shared<msop_data_bpearl>();
    _difop_data = std::make_shared<difop_data_bpearl>();
    setRange(30);
    setReturnMode(lidar::RT_Strongest);
}

bool RSBpearl::Init()
{
    return RSLidar::Init();
}

bool RSBpearl::loadInterReference(const std::string& dir)
{
    if (!RSLidar::loadInterReference(dir))
    {
        return false;
    }
    uint8_t top_frm[5] = {0x06, 0x23, 0x06, 0x06, 0xA0}, bot_frm[5] = {0x07, 0x14, 0x04, 0x01, 0xF0};
    _difop_data->set_frm(top_frm, bot_frm);

    return true;
}
RSHelios::RSHelios()
{
    _msop_data = std::make_shared<msop_data_helios>();
    _difop_data = std::make_shared<difop_data_helios>();
    setRange(200);
    setReturnMode(lidar::RT_Strongest);
}

lidar::ReturnMode RSHelios::getReturnMode() const
{
    int m = _difop_data->get_return_mode();
    switch (m)
    {
        case 0:
            return lidar::RT_Dual;
        case 4:
            return lidar::RT_Strongest;
        case 5:
            return lidar::RT_Last;
        default:
            break;
    }
    return lidar::RT_Strongest;
}

void RSHelios::setReturnMode(lidar::ReturnMode rm)
{
    switch (rm)
    {
        case lidar::RT_Dual:
            _difop_data->set_return_mode(0);
            break;
        case lidar::RT_Strongest:
            _difop_data->set_return_mode(4);
            break;
        case lidar::RT_Last:
            _difop_data->set_return_mode(5);
            break;
        default:
            break;
    }
}

RSRuby::RSRuby()
{
    _msop_data = std::make_shared<msop_data_ruby>();
    _difop_data = std::make_shared<difop_data_ruby>();
    setRange(200);
    setRotationFrequency(10.f);
    setReturnMode(lidar::RT_Strongest);
}

uint32_t RSRuby::getHorizontalScanMinUnit() const
{
    if (getReturnMode() == lidar::RT_Dual)
    {
        return msop_data_ruby::BLOCKS_PER_MSOP / 2;
    }
    return msop_data_ruby::BLOCKS_PER_MSOP;
}

void RSRuby::setRotationFrequency(float rf)
{
    _horizontal_resolution = 0.02f * rf;    // 0.2/10
    RSLidar::setRotationFrequency(rf);
}

bool RSRuby::loadInterReference(const std::string& dir)
{
    if (!RSLidar::loadInterReference(dir))
    {
        return false;
    }
    uint8_t top_frm[5] = {0x00, 0x02, 0x04, 0x0a, 0x00}, bot_frm[5] = {0x00, 0x20, 0x05, 0x30, 0x19};
    _difop_data->set_frm(top_frm, bot_frm);

    return true;
}

bool RSRuby::Init()
{
    return RSLidar::Init();
}

uint32_t RSRuby::package(const lidar_ptset& datas)
{
    uint32_t channels_per_packet = getHorizontalScanMinUnit();
    uint32_t buf_num = datas.channels.size() / channels_per_packet;
    uint32_t drm = 200;
    uint32_t blkper = getReturnMode() == lidar::RT_Dual ? 2 : 1;
    // A group ahead
    for (uint32_t i = 0; i < buf_num; i++)
    {
        const channel_data& cd0 = datas.channels.at(i * channels_per_packet);

        _msop_data->set_header_time(to_utime(cd0.utime));
        for (uint32_t j = 0; j < channels_per_packet; j++)
        {
            const channel_data& cd = datas.channels.at(i * channels_per_packet + j);
            float azimuth = getHorizontalScanAngle(cd.hor_pos);
            assert(cd.pn == getRaysNum() * blkper);
            for (size_t b = 0; b < blkper; ++b)
            {
                _msop_data->set_block_azimuth(j * blkper + b, round(azimuth * 100.f));
                for (uint32_t k = 0; k < msop_data_ruby::CHANNELS_PER_BLOCK; k++)
                {
                    const auto& p = cd.points[b * msop_data_ruby::CHANNELS_PER_BLOCK + k];
                    _msop_data->set_channel_data(
                        j * blkper + b, k, distance2pixel(p.distance * drm, k), reflectivity_calibration(p.instensity));
                }
            }
        }
        send_data(cd0.utime);
    }
    return buf_num;
}

uint8_t RSRuby::reflectivity_calibration(float instensity) const
{
    // get_intensity_mode()==3
    return uint8_t(round(instensity));
}

uint16_t RSRuby::distance2pixel(float distance, uint32_t passageway) const
{
    return (int) (round(distance));
}

lidar::ReturnMode RSRuby::getReturnMode() const
{
    int m = _difop_data->get_return_mode();
    switch (m)
    {
        case 3:
            return lidar::RT_Dual;
        case 1:
            return lidar::RT_Strongest;
        case 2:
            return lidar::RT_Last;
        default:
            break;
    }
    return lidar::RT_Strongest;
}

void RSRuby::setReturnMode(lidar::ReturnMode rm)
{
    switch (rm)
    {
        case lidar::RT_Dual:
            _difop_data->set_return_mode(3);
            break;
        case lidar::RT_Strongest:
            _difop_data->set_return_mode(1);
            break;
        case lidar::RT_Last:
            _difop_data->set_return_mode(2);
            break;
        default:
            break;
    }
}

void RSSLidar::setReturnMode(lidar::ReturnMode rm)
{
    _difop_data->set_return_mode(rm);
    _msop_data->set_return_mode(rm);
}
lidar::ReturnMode RSSLidar::getReturnMode() const
{
    return (lidar::ReturnMode) _difop_data->get_return_mode();
}

uint32_t RSSLidar::getHorizontalScanCount() const
{
    return _horizontal_angles.size() / getRaysNum();
}
float RSSLidar::getHorizontalScanAngle(uint32_t pos) const
{
    return 0;
}
std::pair<float, float> RSSLidar::getYawPitchAngle(uint32_t pos, uint32_t r) const
{
    return std::make_pair(_horizontal_angles.at(pos * getRaysNum() + r), _vertical_angles.at(pos * getRaysNum() + r));
}

const uint8_t* RSSLidar::getDifopBuf() const
{
    return _difop_data->_data;
}

bool RSSLidar::loadInterReference(const std::string& dir)
{
    _temperature = MAX(MIN(_temperature, 71), TEMPERATURE_MIN);
    _msop_data->set_temperature(_temperature);
    setReturnMode(lidar::RT_Strongest);

    // ini config
    {
        std::map<std::string, std::map<std::string, std::string>> config;
        std::string cfgfile = dir + "/cfg" + std::to_string(GetID()) + ".ini";
        UE_LOG(LogTemp, Log, TEXT("load Lidar rs: %s"), ANSI_TO_TCHAR(cfgfile.c_str()));
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
            dd = config["UDP"]["difop_port"];
            if (!dd.empty())
            {
                _udp_difop_port = atoi(dd.c_str());
            }
            dd = config["UDP"]["msop_port"];
            if (!dd.empty())
            {
                _udp_msop_port = atoi(dd.c_str());
            }

            _difop_data->set_eth(lidar_ip, dest_pc_ip, mac_addr, _udp_msop_port, _udp_difop_port);
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("load faild: %s"), ANSI_TO_TCHAR(cfgfile.c_str()));
        }
    }
    _horizontal_angles.clear();
    _vertical_angles.clear();
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
            if (data.size() != getRaysNum() * 2)
            {
                UE_LOG(LogTemp, Warning, TEXT("lidar %d: Error in angle.str."), GetID());
                return false;
            }
            for (uint32_t i = 0; i < getRaysNum(); i++)
            {
                _vertical_angles.push_back(std::atof(data[i * 2].c_str()));
                _horizontal_angles.push_back(std::atof(data[i * 2 + 1].c_str()));
            }
        }
    }
    else
    {
        _error += "cannot open angle.csv.";
        return false;
    }

    return true;
}

bool RSSLidar::setAngleFromString(const FString& str)
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
        if (data.size() != getRaysNum() * 2)
        {
            UE_LOG(LogTemp, Warning, TEXT("lidar %d: Error in angle.str."), GetID());
            return false;
        }
        for (uint32_t i = 0; i < getRaysNum(); i++)
        {
            _vertical_angles.push_back(std::atof(data[i * 2].c_str()));
            _horizontal_angles.push_back(std::atof(data[i * 2 + 1].c_str()));
        }
    }

    return true;
}

bool RSSLidar::setIP(const FString& ip, const FString& port)
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

    _udp_msop_port = std::atoi(data[0].c_str());
    _udp_difop_port = std::atoi(data[1].c_str());
    return true;
}

bool RSSLidar::Init()
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
    RemoteAddr_msop->SetPlatformPort(_udp_msop_port);

    if (!bIsValid)
    {
        _error = "UDP Sender: MSOP IP address was not valid,";
        _error += _udp_IP + ":" + std::to_string(_udp_msop_port);
        // return false;
    }
    RemoteAddr_difop->SetIp(ANSI_TO_TCHAR(_udp_IP.c_str()), bIsValid);
    RemoteAddr_difop->SetPlatformPort(_udp_difop_port);

    if (!bIsValid)
    {
        _error = "UDP Sender: DIFOP IP address was not valid,";
        _error += _udp_IP + ":" + std::to_string(_udp_difop_port);
        // return false;
    }
    int32 SendSize = MAX(_msop_data->length(), difop_data_ss::DIFOP_SIZE) * 10240;
    SenderSocket = FUdpSocketBuilder(TEXT("RSLidar"))
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

    UE_LOG(LogTemp, Log, TEXT("Create Lidar RSS: msop ip=%s:%d, difop ip=%s:%d"), ANSI_TO_TCHAR(_udp_IP.c_str()),
        _udp_msop_port, ANSI_TO_TCHAR(_udp_IP.c_str()), _udp_difop_port);
    // check(SenderSocket->GetSocketType() == SOCKTYPE_Datagram);

    // Set Send Buffer Size
    SenderSocket->SetSendBufferSize(SendSize, SendSize);
    SenderSocket->SetReceiveBufferSize(SendSize, SendSize);
    return true;
}
void RSSLidar::send_data(std::time_t t)
{
    if (SenderSocket)
    {
        int32 BytesSent = 0;
        int32 faildnum = 0;

        while (!SenderSocket->SendTo(_msop_data->data(), _msop_data->length(), BytesSent, *RemoteAddr_msop))
        {
            BytesSent = 0;
            FPlatformProcess::Sleep(0.00001);
            faildnum++;
            if (faildnum == 100)
            {
                UE_LOG(LogTemp, Warning, TEXT("Rsslidar send msop error: only sent %d bytes. errcode is %d"), BytesSent,
                    ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->TranslateErrorCode(BytesSent));
                break;
            }
        }

        auto dt = (t - pretime) / 1000;
        if (dt >= 100)
        {
            _difop_data->set_time(to_utime(t));
            BytesSent = 0;
            if (!SenderSocket->SendTo(_difop_data->data(), difop_data_ss::DIFOP_SIZE, BytesSent, *RemoteAddr_difop))
            {
                UE_LOG(LogTemp, Warning, TEXT("Rsslidar send difop error: only sent %d bytes. errcode is %d"),
                    BytesSent, ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->TranslateErrorCode(BytesSent));
            }
            pretime = t;
        }
    }
}

RSM1::RSM1()
{
    _msop_data = std::make_shared<msop_data_m1>();
    _difop_data = std::make_shared<difop_data_ss>();
    setRange(200);
    setReturnMode(lidar::RT_Strongest);
}
bool RSM1::loadInterReference(const std::string& dir)
{
    _difop_data->set_hor_fov(0, 12000);
    _difop_data->set_ele_fov(0, 2500);
    return RSSLidar::loadInterReference(dir);
}

lidar::ReturnMode RSM1::getReturnMode() const
{
    int m = _difop_data->get_return_mode();
    switch (m)
    {
        case 0:
            return lidar::RT_Dual;
        case 4:
            return lidar::RT_Strongest;
        case 5:
            return lidar::RT_Last;
        default:
            break;
    }
    return lidar::RT_Strongest;
}

void RSM1::setReturnMode(lidar::ReturnMode rm)
{
    switch (rm)
    {
        case lidar::RT_Dual:
            _difop_data->set_return_mode(0);
            break;
            _msop_data->set_return_mode(0);
            break;
        case lidar::RT_Strongest:
            _difop_data->set_return_mode(4);
            break;
            _msop_data->set_return_mode(4);
            break;
        case lidar::RT_Last:
            _difop_data->set_return_mode(5);
            break;
            _msop_data->set_return_mode(5);
            break;
        default:
            break;
    }
}

uint32_t RSM1::package(const lidar_ptset& datas)
{
    // Calculate number of packets required for this set of data
    uint32_t channels_per_packet = getHorizontalScanMinUnit();
    uint32_t buf_num = datas.channels.size() / channels_per_packet;
    uint32_t drm = 200;
    uint32_t blkper = getReturnMode() == lidar::RT_Dual ? 2 : 1;
    // A group ahead
    //  A group ahead: process each block in the dataset
    for (uint32_t i = 0; i < buf_num; i++)
    {
        const channel_data& cd = datas.channels.at(i * channels_per_packet);
        // Check if we have enough rays available for current block
        assert(cd.pn == getRaysNum() * blkper);

        // For each block within the horizontal scan unit...
        for (size_t b = 0; b < blkper; ++b)
        {
            _msop_data->set_header_time(to_utime(cd.utime), cd.hor_pos * blkper + b);
            for (uint32_t j = 0; j < msop_data_m1::BLOCKS_PER_MSOP; j++)
            {
                _msop_data->set_block_azimuth(j, 0, blkper == 2 ? (b + 1) : 0);
                for (uint32_t k = 0; k < msop_data_m1::CHANNELS_PER_BLOCK; k++)
                {
                    int vi = j * msop_data_m1::CHANNELS_PER_BLOCK + k;
                    auto py = getYawPitchAngle(cd.hor_pos, vi);
                    uint16_t pitch = std::round(py.second * 100) + 32768;
                    uint16_t yaw = std::round(-py.first * 100) + 32768;
                    const auto& p = cd.points[vi * blkper + b * msop_data_ruby::CHANNELS_PER_BLOCK];
                    // Round distances to avoid floating-point precision issues
                    _msop_data->set_channel_data(
                        j * blkper + b, k, pitch, yaw, (int) round(p.distance * drm), uint8_t(round(p.instensity)));
                }
            }
            // Send processed data to the host at appropriate time
            send_data(cd.utime);
        }
    }
    return buf_num * blkper;
}

}    // namespace rslidar
