/**
 * @file Rar_Init_Echo.cpp
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-03-25
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "rar_init_echo.h"
#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include "fftw3.h"

#define PI 3.1415926535897932384626433832795
namespace radar_bit {

const double weather_L1_24[20] = {0.32, 0.618, 2.09, 4.04, 7.99, 0.34, 0.48, 0.64, 0.38,
                                  0.8,  1.28,  0.33, 0.32, 0.32, 0.32, 0.32, 0.32};
const double weather_L1_77[20] = {0.62, 2.28, 6.66, 11.02, 18.52, 0.68, 0.94, 1.26,
                                  0.74, 1.58, 2.54, 0.64,  0.62,  0.62, 0.62, 0.62};
const double c = 3e8;

/**
 * @brief radar parameter
 *
 */
struct radar_exinfo {
  double fc = 77;
  double B = 0;
  double lambda = 0;
  double Tc = 0;
  int Ns = 256;  // virtual channel
  int Nr = 128;  // sampling in a strip
  int Nd = 64;   // num of strip in frame
  double delta_range = 0;
  double delta_veolity = 0;
  double maxV = 0;
};

radar_parameter _radar_parameter[MAX_RAD_NUM] = {};
radar_exinfo _radar_exinfo[MAX_RAD_NUM] = {};
output _radar_delta;

/**
 * @brief decibel to watt
 *
 * @param db decibel
 * @return watt
 */
double db2w(double db) { return pow(10, db * 0.1); }

/**
 * @brief calculate max range
 *
 * @param m radar parameter
 * @return max range
 */
double max_range(const radar_parameter &m) {
  if (m.F0_GHz <= 1) return 0;
  double lamda = 0.3 / m.F0_GHz;
  double Pr = m.Pt_dBm * m.Gt_dBi * m.Gr_dBi * lamda * lamda * m.rcs * m.accumtime;
  if (Pr <= 0) return 0;
  double k = 1.38e-23;
  double Pn_snr = db2w(m.SNR_min_dB) * 64 * PI * PI * PI * k * m.Ts_K * db2w(m.Fn_dB) * db2w(m.L0_dB) * db2w(m.L1_dB);
  if (Pn_snr <= 0) return 0;
  return pow(Pr / Pn_snr, 0.25);
}

/**
 * @brief calculate L1(weather)
 *
 * @param f0
 * @param weather
 * @param r
 * @return double
 */
double L1(int f0, int weather, double r) {
  weather = std::min(11, std::max(0, weather));
  if (f0 == 24) {
    return weather_L1_24[weather] * r * 0.001;
  } else {
    return weather_L1_77[weather] * r * 0.001;
  }
  return 0;
}

/**
 * @brief calculate signal-to-noise ratio
 *
 * @param m radar parameter
 * @param tar target
 * @return snr
 */
double calculate_snr(const radar_parameter &m, const tar_info_in &tar) {
  double lamda = 0.3 / m.F0_GHz;
  double k = 1.38e-23;
  float L1_dB = L1(round(m.F0_GHz), m.weather, tar.tar_range);
  double Pr = m.Pt_dBm * m.Gt_dBi * m.Gr_dBi * lamda * lamda * tar.tar_rcs * m.accumtime;
  double Pn = 64 * PI * PI * PI * k * m.Ts_K * tar.tar_range * tar.tar_range * tar.tar_range * tar.tar_range *
              db2w(m.L0_dB) * db2w(L1_dB);
  if (Pn <= 0) return 0;
  return 10 * log10(Pr / Pn);
}

/**
 * @brief save BMP image. For debug
 *
 * @param fname file path
 * @param buf image array
 * @param w width
 * @param h height
 * @return true on success
 * @return false on failure
 */
bool SaveBMP(const std::string &fname, unsigned char *buf, int w, int h) {
#pragma pack(push, 1)

  typedef struct tagBITMAPFILEHEADER {
    std::uint16_t bfType;
    unsigned int bfSize;
    std::uint16_t bfReserved1;
    std::uint16_t bfReserved2;
    unsigned int bfOffBits;
  } BITMAPFILEHEADER;
  typedef struct tagBITMAPINFOHEADER {
    unsigned int biSize;
    int biWidth;
    int biHeight;
    std::uint16_t biPlanes;
    std::uint16_t biBitCount;
    unsigned int biCompression;
    unsigned int biSizeImage;
    int biXPelsPerMeter;
    int biYPelsPerMeter;
    unsigned int biClrUsed;
    unsigned int biClrImportant;
  } BITMAPINFOHEADER;

  typedef struct tagRGBQUAD {
    unsigned char rgbBlue;
    unsigned char rgbGreen;
    unsigned char rgbRed;
    unsigned char rgbReserved;
  } RGBQUAD;

  int w4 = (((w + 3) / 4) * 4);

  BITMAPFILEHEADER targetfileheader;
  BITMAPINFOHEADER targetinfoheader;
  memset(&targetfileheader, 0, sizeof(BITMAPFILEHEADER));
  memset(&targetinfoheader, 0, sizeof(BITMAPINFOHEADER));

  targetfileheader.bfOffBits = 1078;
  targetfileheader.bfSize = h * w4 + targetfileheader.bfOffBits;
  targetfileheader.bfReserved1 = 0;
  targetfileheader.bfReserved2 = 0;
  targetfileheader.bfType = 0x4d42;

  targetinfoheader.biBitCount = 8;
  targetinfoheader.biSize = 40;
  targetinfoheader.biHeight = h;
  targetinfoheader.biWidth = w;
  targetinfoheader.biPlanes = 1;
  targetinfoheader.biCompression = 0L;  // #define BI_RGB        0L
  targetinfoheader.biSizeImage = h * w4;

  RGBQUAD rgbquad[256];

  for (int i = 0; i < 256; i++) {
    rgbquad[i].rgbBlue = i;
    rgbquad[i].rgbGreen = i;
    rgbquad[i].rgbRed = i;
    rgbquad[i].rgbReserved = 0;
  }

  assert(sizeof(BITMAPFILEHEADER) == 14);
  assert(sizeof(BITMAPINFOHEADER) == 40);
  assert(sizeof(RGBQUAD) == 4);

  std::ofstream cf(fname, std::ios::binary);
  if (!cf.is_open()) {
    return false;
  }
  cf.write((const char *)&targetfileheader, sizeof(BITMAPFILEHEADER));
  cf.write((const char *)&targetinfoheader, sizeof(BITMAPINFOHEADER));
  cf.write((const char *)&rgbquad, 4 * 256);

  for (int i = 0; i < h; ++i) {
    auto *b = &(buf[(h - 1 - i) * w]);
    cf.write((const char *)b, w);
    char tmp[4] = {0};
    if (w < w4) {
      cf.write(tmp, w4 - w);
    }
  }

  cf.close();

  return true;

#pragma pack(pop)
}

/**
 * @brief radar initialization
 *
 * @param radar_figure radar figure
 * @param radar_idx radar index
 * @return output
 */
output Radar_Initialization(radar_parameter radar_figure, int radar_idx) {
  if (radar_idx >= MAX_RAD_NUM) return _radar_delta;
  _radar_delta.hfov[radar_idx] = radar_figure.hfov;
  radar_exinfo ex;
  int f0 = round(radar_figure.F0_GHz);
  radar_figure.L1_dB = L1(f0, radar_figure.weather, 500);
  int Ns = 256;
  int Nr = 128;
  int Nd = 64;
  if (f0 == 24) {
    ex.fc = 24e9;
    ex.Tc = 100e-6;
    ex.B = 150e6;
    ex.Nr = 88;
    ex.Nd = 64;
    ex.Ns = 16;
  } else if (f0 == 77) {
    ex.fc = 77e9;
    ex.Tc = 50e-6;
    ex.B = 150e6;
    ex.Nr = 250;
    ex.Nd = 64;
    ex.Ns = 16;
  } else if (f0 == 79) {
    ex.fc = 77e9;
    ex.Tc = 20e-6;
    ex.B = 4e9;
    ex.Nr = 4096;
    ex.Nd = 1024;
    ex.Ns = 192;
  } else {
    std::cout << "radar: not support " << f0 << " GHz\n";
    return _radar_delta;
  }
  radar_figure.accumtime = ex.Tc;
  ex.lambda = c / ex.fc;
  ex.maxV = ex.lambda / (4 * ex.Tc);
  ex.delta_range = c / 2 / ex.B;
  ex.delta_veolity = ex.maxV * 2 / ex.Nd;

  _radar_delta.delta_R[radar_idx] = ex.delta_range;
  _radar_delta.delta_V[radar_idx] = ex.delta_veolity;
  _radar_delta.delta_A[radar_idx] = 2 * 180 / PI / ex.Ns;
  _radar_delta.sigma_R[radar_idx] = c / 3.6 / ex.B / sqrt(2 * radar_figure.SNR_min_dB);
  _radar_delta.sigma_V[radar_idx] = ex.lambda / 3.6 / Nd / ex.Tc / sqrt(radar_figure.SNR_min_dB);
  _radar_delta.sigma_A[radar_idx] = radar_figure.hfov / 1.6 / sqrt(2 * radar_figure.SNR_min_dB);
  _radar_delta.R_max[radar_idx] = std::min(_radar_delta.delta_R[radar_idx] * ex.Nr / 2, max_range(radar_figure));

  _radar_parameter[radar_idx] = radar_figure;
  _radar_exinfo[radar_idx] = ex;
  return _radar_delta;
}

/**
 * @brief shift for fft
 *
 * @param i index
 * @param n number of signal
 * @return shift of i
 */
int shift(int i, int n) { return (i + n) % n; }

/**
 * @brief shift back for fft
 *
 * @param i index
 * @param n number of signal
 * @return shift back of i
 */
int shifti(int i, int n) {
  if (i > n / 2) {
    return i - n;
  }
  return i;
}

/**
 * @brief fft to signal
 *
 * @param m radar parameter
 * @param tar_info_out object (in and out)
 * @param ex radar information
 */
void signal_fft3(const radar_parameter &m, tar_info_out &tar_info_out, const radar_exinfo &ex) {
  if (tar_info_out.tar_num == 0) {
    return;
  }

  double *ifx = new double[ex.Ns * ex.Nr * ex.Nd];
  double dt = ex.Tc / ex.Nr;
  double sd = ex.lambda / 2;
  double S = ex.B / ex.Tc;
  for (int i = 0; i < ex.Ns; i++) {
    for (int j = 0; j < ex.Nd; j++) {
      for (int k = 0; k < ex.Nr; k++) {
        double t = dt * k;
        double t2 = ex.Tc * j + t;
        double tx = cos(2 * PI * (ex.fc * t + S * t * t / 2));
        double rx = 0;
        for (int l = 0; l < tar_info_out.tar_num; l++) {
          double r = tar_info_out.tar_range[l] + tar_info_out.tar_speed[l] * t2;
          double x = r * cos(tar_info_out.tar_angle[l] * PI / 180);
          double y = -r * sin(tar_info_out.tar_angle[l] * PI / 180);
          y -= sd * i;
          double r2 = sqrt(x * x + y * y);
          double td = (r + r2) / c;
          double freqRx = ex.fc + S * (t - td);
          rx += tar_info_out.tar_Ar[l] * cos(2 * PI * (ex.fc * (t - td) + (S * (t - td) * (t - td)) / 2));
        }
        ifx[i * ex.Nr * ex.Nd + k * ex.Nd + j] = rx * tx;
      }
    }
  }
  fftw_complex *ofx = reinterpret_cast<fftw_complex *>(fftw_malloc(sizeof(fftw_complex) * ex.Ns * ex.Nr * ex.Nd));
  fftw_plan fft = fftw_plan_dft_r2c_3d(ex.Ns, ex.Nr, ex.Nd, ifx, ofx, FFTW_ESTIMATE);
  fftw_execute(fft);
  fftw_destroy_plan(fft);
  delete[] ifx;

  fftw_free(ofx);
}

/**
 * @brief 2d fft to signal
 *
 * @param m radar parameter
 * @param object (in and out)
 * @param ex radar information
 */
void signal_fft2(const radar_parameter &m, tar_info_out &tar_info_out, const radar_exinfo &ex) {
  if (tar_info_out.tar_num == 0) {
    return;
  }
  const float max_ar = 100.f;
  double *ifx = new double[ex.Nr * ex.Nd];
  double dt = ex.Tc / (ex.Nr - 1);
  double S = ex.B / ex.Tc;
  for (int i = 0; i < ex.Nd; i++) {
    for (int j = 0; j < ex.Nr; j++) {
      double t = dt * j;
      double t2 = ex.Tc * i;
      double tx = cos(2 * PI * (ex.fc * t + S * t * t / 2));
      double rx = 0;
      for (int l = 0; l < tar_info_out.tar_num; l++) {
        double r = tar_info_out.tar_range[l] + t2 * tar_info_out.tar_speed[l];
        double td = (r + r) / c;
        double freqRx = ex.fc + S * (t - td);
        rx +=
            std::min(max_ar, tar_info_out.tar_Ar[l]) * cos(2 * PI * (ex.fc * (t - td) + (S * (t - td) * (t - td)) / 2));
      }
      ifx[i * ex.Nr + j] = rx * tx;
    }
  }
  int fw = ex.Nr / 2 + 1;
  fftw_complex *ofx = reinterpret_cast<fftw_complex *>(fftw_malloc(sizeof(fftw_complex) * ex.Nd * fw));
  fftw_plan fft = fftw_plan_dft_r2c_2d(ex.Nd, ex.Nr, ifx, ofx, FFTW_ESTIMATE);
  fftw_execute(fft);
  fftw_destroy_plan(fft);
  delete[] ifx;
  double dmin = DBL_MAX, dmax = 0;
  double *fti = reinterpret_cast<double *>(ofx);
  for (int i = 0; i < ex.Nd; i++) {
    for (int j = 0; j < fw; j++) {
      double *d = fti + (i * fw + j) * 2;
      double s = d[0] * d[0] + d[1] * d[1];
      if (s < 1) {
        fti[i * fw + j] = 0;
        continue;
      }
      s = sqrt(s);
      dmin = std::min(dmin, s);
      dmax = std::max(dmax, s);
      fti[i * fw + j] = s;
    }
  }
  // debug
  if (0) {
    unsigned char *buf = new unsigned char[ex.Nd * fw];
    memset(buf, 0, ex.Nd * fw);
    for (int i = 0; i < ex.Nd; i++) {
      for (int j = 0; j < fw; j++) {
        double s = fti[i * fw + j];
        s = (s - dmin) / (dmax - dmin);
        int r = s * 255;
        buf[i * fw + j] = std::min(255, std::max(0, r));
      }
    }
    SaveBMP("test.bmp", buf, fw, ex.Nd);
    delete[] buf;
  }
  int r = 2;
  int sucl = 0;
  std::map<std::pair<int, int>, std::set<int>> mtop;
  for (int l = 0; l < tar_info_out.tar_num; l++) {
    int i = round(tar_info_out.tar_speed[l] / ex.delta_veolity);

    int j = round(tar_info_out.tar_range[l] / ex.delta_range);

    int lt = std::max(0, j - r);
    int rt = std::min(fw - 1, j + r);
    int tp = i - r;
    int bm = i + r;

    double maxd = 0;
    int maxm = i, maxn = j;
    for (int m = tp; m <= bm; m++) {
      int mm = shift(m, ex.Nd);
      for (int n = lt; n <= rt; n++) {
        double s = fti[mm * fw + n];
        if (s > maxd) {
          maxd = s;
          maxm = mm;
          maxn = n;
        }
      }
    }
    maxd = (maxd - dmin) / (dmax - dmin);
    if (maxd < 0.5) {
      continue;
    }

    int a = round(sin(tar_info_out.tar_angle[l] * PI / 180.0) * ex.Ns / 2.0);
    if (mtop.find(std::make_pair(maxm, maxn)) != mtop.end()) {
      auto &as = mtop.at(std::make_pair(maxm, maxn));
      if (as.find(a) != as.end()) {
        continue;
      }
    }
    mtop[std::make_pair(maxm, maxn)].insert(a);
    tar_info_out.tar_range[l] = ex.delta_range * maxn;
    tar_info_out.tar_speed[l] = ex.delta_veolity * shifti(maxm, ex.Nd);
    tar_info_out.tar_angle[l] = asin(a * 2.0 / ex.Ns) * 180.0 / PI;
    if (l > sucl) {
      tar_info_out.tar_range[sucl] = tar_info_out.tar_range[l];
      tar_info_out.tar_speed[sucl] = tar_info_out.tar_speed[l];
      tar_info_out.tar_angle[sucl] = tar_info_out.tar_angle[l];
      tar_info_out.tar_rcs[sucl] = tar_info_out.tar_rcs[l];
      tar_info_out.tar_Ar[sucl] = tar_info_out.tar_Ar[l];
      tar_info_out.tar_ID[sucl] = tar_info_out.tar_ID[l];
    }
    sucl++;
  }
  tar_info_out.tar_num = sucl;
  fftw_free(ofx);
}

/**
 * @brief radar signal processing
 *
 * @param radar_angle radar angle
 * @param tar_info target information input
 * @param num target number
 * @param initialization_out radar parameter of initialization
 * @param radar_ID radar id
 * @param weather weather
 * @return target information output
 */
tar_info_out Rar_signal_pro(float radar_angle, tar_info_in tar_info[], int num, const output &initialization_out,
                            int radar_ID, int weather) {
  tar_info_out out = {0};
  if (num >= MAX_TAR_NUM) return out;
  if (radar_ID >= MAX_RAD_NUM) return out;
  const radar_parameter &m = _radar_parameter[radar_ID];
  const auto &ex = _radar_exinfo[radar_ID];

  out.tar_num = 0;
  for (int i = 0; i < num; i++) {
    const tar_info_in &tar = tar_info[i];
    if (tar.tar_range >= _radar_delta.R_max[radar_ID]) {
      continue;
    }
    if (abs(tar.tar_speed) >= ex.maxV) {
      continue;
    }
    double snr = calculate_snr(m, tar);
    if (snr < m.SNR_min_dB) {
      continue;
    }

    out.tar_Ar[out.tar_num] = pow(10, snr / 20);
    out.tar_rcs[out.tar_num] = tar.tar_rcs;
    out.tar_ID[out.tar_num] = tar.tar_ID;
    out.tar_range[out.tar_num] = round(tar.tar_range / ex.delta_range) * ex.delta_range;
    out.tar_speed[out.tar_num] = round(tar.tar_speed / ex.delta_veolity) * ex.delta_veolity;
    out.tar_angle[out.tar_num] = asin(round(sin(tar.tar_angle * PI / 180.0) * ex.Ns / 2.0) * 2.0 / ex.Ns) * 180.0 / PI;
    out.tar_num++;
  }
  signal_fft2(m, out, ex);
  return out;
}

}  // namespace radar_bit
