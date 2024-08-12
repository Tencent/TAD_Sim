/**
 * @file TruthBase.cpp
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-03-19
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "truth_base.h"
#include <fstream>
#include <string>

sim_msg::Union TruthBase::getUnion(tx_sim::StepHelper &helper) {
  std::string payload_;
  sim_msg::Union union_location;
  union_location.ParseFromString(payload_);
  helper.GetSubscribedMessage(tx_sim::topic::kLocation, payload_);
  return union_location;
}

sim_msg::Traffic TruthBase::getTraffic(tx_sim::StepHelper &helper) {
  std::string payload_;
  helper.GetSubscribedMessage(tx_sim::topic::kTraffic, payload_);
  sim_msg::Traffic traffic;
  traffic.ParseFromString(payload_);
  return traffic;
}

sim_msg::Location TruthBase::getLocation(tx_sim::StepHelper &helper) {
  std::string payload_;
  sim_msg::Location loc;
  loc.ParseFromString(payload_);
  return loc;
}

bool SaveBMP(const std::string &fname, const std::vector<uint8_t> &buf, int w, int h) {
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

  std::vector<uint8_t> img = buf;
  for (int i = 0; i < h; ++i) {
    uint8_t *b = &(img[i * w]);
    // int8u_t* b = &(img[(h - 1 - i)*w]);
    // for (int j = 0; j < w; ++j)
    // {
    // b[j] *= 80;
    // }
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
