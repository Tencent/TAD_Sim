/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "sensors.h"
#include <tinyxml.h>
#include <boost/algorithm/string.hpp>
#include "common/log/system_logger.h"
#include "common/log/xml_logger.h"

int CSensors::Parse(const char* strSensorFile) {
  if (!strSensorFile) return -1;

  TiXmlDocument doc;
  bool bRet = doc.LoadFile(strSensorFile);

  if (!bRet) return -1;

  TiXmlElement* xmlRoot = doc.RootElement();

  if (!xmlRoot) return -1;

  std::string strName = xmlRoot->Value();
  // if (_stricmp(strName.c_str(), "traffic") != 0)
  if (!boost::algorithm::iequals(strName, "Sensor")) return -1;

  TiXmlElement* elemCamera = xmlRoot->FirstChildElement("Camera");
  if (elemCamera) {
    int nRet = ParseCameras(elemCamera, m_mapCameras);
    if (nRet != 0) {
      return -1;
    }
  }

  TiXmlElement* elemRadar = xmlRoot->FirstChildElement("Radar");
  if (elemRadar) {
    int nRet = ParseRadars(elemRadar, m_mapRadars);
    if (nRet != 0) {
      return -1;
    }
  }

  TiXmlElement* elemTraditionalLidar = xmlRoot->FirstChildElement("TraditionalLidar");
  if (elemTraditionalLidar) {
    int nRet = ParseTraditionalLidars(elemTraditionalLidar, m_mapTraditionalLidars);
    if (nRet != 0) {
      return -1;
    }
  }

  TiXmlElement* elemSensorTruth = xmlRoot->FirstChildElement("Truth");
  if (elemSensorTruth) {
    int nRet = ParseTruths(elemSensorTruth, m_mapSensorTruths);
    if (nRet != 0) {
      return -1;
    }
  }

  TiXmlElement* elemIMU = xmlRoot->FirstChildElement("IMU");
  if (elemIMU) {
    int nRet = ParseIMUs(elemIMU, m_mapIMUs);
    if (nRet != 0) {
      return -1;
    }
  }

  TiXmlElement* elemGPS = xmlRoot->FirstChildElement("GPS");
  if (elemGPS) {
    int nRet = ParseGPSes(elemGPS, m_mapGPSes);
    if (nRet != 0) {
      return -1;
    }
  }
  return 0;
}

int CSensors::ParseCameras(TiXmlElement* elemCamera, CameraMap& cameras) {
  if (!elemCamera) return -1;

  cameras.clear();

  TiXmlElement* elemConfig = elemCamera->FirstChildElement("Config");
  while (elemConfig) {
    CSensorCamera c;

    int nRet = ParseOneCamera(elemConfig, c);
    if (nRet) {
      SYSTEM_LOGGER_ERROR("parse cameras error!");
      assert(false);

      elemConfig = elemConfig->NextSiblingElement("Config");
      continue;
    }

    cameras.insert(std::make_pair(c.m_strID, c));

    elemConfig = elemConfig->NextSiblingElement("Config");
  }

  return 0;
}

int CSensors::ParseOneBaseInfo(TiXmlElement* elemConfig, CSensorBase& b) {
  if (!elemConfig) return -1;

  const char* p = elemConfig->Attribute("ID");
  if (p) b.m_strID = p;

  p = elemConfig->Attribute("LocationX");
  if (p) b.m_strLocationX = p;

  p = elemConfig->Attribute("LocationY");
  if (p) b.m_strLocationY = p;

  p = elemConfig->Attribute("LocationZ");
  if (p) b.m_strLocationZ = p;

  p = elemConfig->Attribute("RotationX");
  if (p) b.m_strRotationX = p;

  p = elemConfig->Attribute("RotationY");
  if (p) b.m_strRotationY = p;

  p = elemConfig->Attribute("RotationZ");
  if (p) b.m_strRotationZ = p;

  p = elemConfig->Attribute("InstallSlot");
  if (p) b.m_strInstallSlot = p;

  return 0;
}

int CSensors::ParseOneCamera(TiXmlElement* elemConfig, CSensorCamera& c) {
  if (!elemConfig) return -1;

  ParseOneBaseInfo(elemConfig, c);

  const char* p = elemConfig->Attribute("Enabled");
  if (p && boost::algorithm::iequals(p, "true"))
    c.m_bEnabled = true;
  else
    c.m_bEnabled = false;

  p = elemConfig->Attribute("IntrinsicParamType");
  if (p) {
    if (boost::algorithm::iequals(p, "intrinsic")) {
      c.m_nIntrinsicParamType = 0;
    } else if (boost::algorithm::iequals(p, "fov")) {
      c.m_nIntrinsicParamType = 1;
    } else if (boost::algorithm::iequals(p, "ccd")) {
      c.m_nIntrinsicParamType = 2;
    }
  }

  p = elemConfig->Attribute("Frequency");
  if (p) c.m_strFrequency = p;

  p = elemConfig->Attribute("Device");
  if (p) c.m_strDevice = p;

  /*p = elemConfig->Attribute("SaveData");
  if (p && boost::algorithm::iequals(p, "true"))
          c.m_bSaveData = true;
  else
          c.m_bSaveData = false;*/

  p = elemConfig->Attribute("DisplayMode");
  if (p) c.m_strDisplayMode = p;

  p = elemConfig->Attribute("FOV_Horizontal");
  if (p) c.m_strFOVHorizontal = p;

  p = elemConfig->Attribute("FOV_Vertical");
  if (p) c.m_strFOVVertical = p;

  p = elemConfig->Attribute("Res_Horizontal");
  if (p) c.m_strResHorizontal = p;

  p = elemConfig->Attribute("Res_Vertical");
  if (p) c.m_strResVertical = p;

  p = elemConfig->Attribute("Blur_Intensity");
  if (p) c.m_strBlurIntensity = p;

  p = elemConfig->Attribute("MotionBlur_Amount");
  if (p) c.m_strMotionBlurAmount = p;

  p = elemConfig->Attribute("Vignette_Intensity");
  if (p) c.m_strVignetteIntensity = p;

  p = elemConfig->Attribute("Noise_Intensity");
  if (p) c.m_strNoiseIntensity = p;

  p = elemConfig->Attribute("Intrinsic_Matrix");
  if (p) c.m_strIntrinsicMatrix = p;

  p = elemConfig->Attribute("Distortion_Parameters");
  if (p) c.m_strDistortionParamters = p;

  p = elemConfig->Attribute("CCD_Width");
  if (p) c.m_strCCDWidth = p;

  p = elemConfig->Attribute("CCD_Height");
  if (p) c.m_strCCDHeight = p;

  p = elemConfig->Attribute("Focal_Length");
  if (p) c.m_strFocalLength = p;

  return 0;
}

int CSensors::ParseRadars(TiXmlElement* elemRadars, RadarMap& radars) {
  if (!elemRadars) return -1;

  radars.clear();

  TiXmlElement* elemConfig = elemRadars->FirstChildElement("Config");
  while (elemConfig) {
    CSensorRadar r;

    int nRet = ParseOneRadar(elemConfig, r);
    if (nRet) {
      SYSTEM_LOGGER_ERROR("parse radars error!");
      assert(false);

      elemConfig = elemConfig->NextSiblingElement("Config");
      continue;
    }

    radars.insert(std::make_pair(r.m_strID, r));

    elemConfig = elemConfig->NextSiblingElement("Config");
  }

  return 0;
}

int CSensors::ParseOneRadar(TiXmlElement* elemConfig, CSensorRadar& r) {
  if (!elemConfig) return -1;

  ParseOneBaseInfo(elemConfig, r);

  const char* p = elemConfig->Attribute("Enabled");
  if (p && boost::algorithm::iequals(p, "true"))
    r.m_bEnabled = true;
  else
    r.m_bEnabled = false;

  p = elemConfig->Attribute("Frequency");
  if (p) r.m_strFrequency = p;

  p = elemConfig->Attribute("Device");
  if (p) r.m_strDevice = p;

  /*p = elemConfig->Attribute("SaveData");
  if (p && boost::algorithm::iequals(p, "true"))
          r.m_bSaveData = true;
  else
          r.m_bSaveData = false;*/

  p = elemConfig->Attribute("F0_GHz");
  if (p) r.m_strF0GHz = p;

  p = elemConfig->Attribute("Pt_dBm");
  if (p) r.m_strPtDBm = p;

  p = elemConfig->Attribute("Gt_dBi");
  if (p) r.m_strGTDBi = p;

  p = elemConfig->Attribute("Gr_dBi");
  if (p) r.m_strGrDBi = p;

  p = elemConfig->Attribute("Ts_K");
  if (p) r.m_strTsK = p;

  p = elemConfig->Attribute("Fn_dB");
  if (p) r.m_strFnDB = p;

  p = elemConfig->Attribute("L0_dB");
  if (p) r.m_strL0DB = p;

  p = elemConfig->Attribute("SNR_min_dB");
  if (p) r.m_strSNRMinDB = p;

  p = elemConfig->Attribute("radar_angle");
  if (p) r.m_strRadarAngle = p;

  p = elemConfig->Attribute("R_m");
  if (p) r.m_strRM = p;

  p = elemConfig->Attribute("rcs");
  if (p) r.m_strRcs = p;

  p = elemConfig->Attribute("weather");
  if (p) r.m_strWeather = p;

  p = elemConfig->Attribute("tag");
  if (p) r.m_strTag = p;

  p = elemConfig->Attribute("anne_tag");
  if (p) r.m_strAnneTag = p;

  p = elemConfig->Attribute("hwidth");
  if (p) r.m_strHWidth = p;

  p = elemConfig->Attribute("vwidth");
  if (p) r.m_strVWidth = p;

  p = elemConfig->Attribute("hfov");
  if (p) r.m_strHFov = p;

  p = elemConfig->Attribute("vfov");
  if (p) r.m_strVFov = p;

  p = elemConfig->Attribute("ANTENNA_ANGLE_path1");
  if (p) r.m_strAntennaAnglePath1 = p;

  p = elemConfig->Attribute("ANTENNA_ANGLE_path2");
  if (p) r.m_strAntennaAnglePath2 = p;

  return 0;
}
int CSensors::ParseTraditionalLidars(TiXmlElement* elemTraditionalLidars, TraditionalLidarMap& traditionalLidars) {
  if (!elemTraditionalLidars) return -1;

  traditionalLidars.clear();

  TiXmlElement* elemConfig = elemTraditionalLidars->FirstChildElement("Config");
  while (elemConfig) {
    CSensorTraditionalLindar tl;

    int nRet = ParseOneTraditionalLidar(elemConfig, tl);
    if (nRet) {
      SYSTEM_LOGGER_ERROR("parse traditonal lidar error!");
      assert(false);

      elemConfig = elemConfig->NextSiblingElement("Config");
      continue;
    }

    traditionalLidars.insert(std::make_pair(tl.m_strID, tl));

    elemConfig = elemConfig->NextSiblingElement("Config");
  }

  return 0;
}

int CSensors::ParseOneTraditionalLidar(TiXmlElement* elemConfig, CSensorTraditionalLindar& tl) {
  if (!elemConfig) return -1;

  ParseOneBaseInfo(elemConfig, tl);

  const char* p = elemConfig->Attribute("Enabled");
  if (p && boost::algorithm::iequals(p, "true"))
    tl.m_bEnabled = true;
  else
    tl.m_bEnabled = false;

  p = elemConfig->Attribute("Frequency");
  if (p) tl.m_strFrequency = p;

  p = elemConfig->Attribute("Device");
  if (p) tl.m_strDevice = p;

  p = elemConfig->Attribute("DrawPoint");
  if (p && boost::algorithm::iequals(p, "true"))
    // tl.m_strDrawPoint = p;
    tl.m_bDrawPoint = true;
  else
    tl.m_bDrawPoint = false;

  p = elemConfig->Attribute("DrawRay");
  if (p && boost::algorithm::iequals(p, "true"))
    tl.m_bDrawRay = true;
  else
    tl.m_bDrawRay = false;

  p = elemConfig->Attribute("Model");
  if (p) tl.m_strModel = p;

  /*	p = elemConfig->Attribute("Threads");
          if (p)
                  tl.m_strThreads = p;
  */
  p = elemConfig->Attribute("uChannels");
  if (p) tl.m_strUChannels = p;

  p = elemConfig->Attribute("uRange");
  if (p) tl.m_strURange = p;

  p = elemConfig->Attribute("uHorizontalResolution");
  if (p) tl.m_strUHorizontalResolution = p;

  p = elemConfig->Attribute("uUpperFov");
  if (p) tl.m_strUUpperFov = p;

  p = elemConfig->Attribute("uLowerFov");
  if (p) tl.m_strULowerFov = p;

  /*p = elemConfig->Attribute("uSaveData");
  if (p && boost::algorithm::iequals(p, "true"))
          tl.m_bUSaveData = true;
  else
          tl.m_bUSaveData = false;*/

  return 0;
}
int CSensors::ParseTruths(TiXmlElement* elemTruths, SensorTruthMap& truths) {
  if (!elemTruths) return -1;

  truths.clear();

  TiXmlElement* elemConfig = elemTruths->FirstChildElement("Config");
  while (elemConfig) {
    CSensorTruth t;

    int nRet = ParseOneTruth(elemConfig, t);
    if (nRet) {
      SYSTEM_LOGGER_ERROR("parse truths error!");
      assert(false);

      elemConfig = elemConfig->NextSiblingElement("Config");
      continue;
    }

    truths.insert(std::make_pair(t.m_strID, t));

    elemConfig = elemConfig->NextSiblingElement("Config");
  }

  return 0;
}

int CSensors::ParseOneTruth(TiXmlElement* elemConfig, CSensorTruth& t) {
  if (!elemConfig) return -1;

  ParseOneBaseInfo(elemConfig, t);

  const char* p = elemConfig->Attribute("Enabled");
  if (p && boost::algorithm::iequals(p, "true"))
    t.m_bEnabled = true;
  else
    t.m_bEnabled = false;

  p = elemConfig->Attribute("Device");
  if (p) t.m_strDevice = p;

  /*p = elemConfig->Attribute("SaveData");
  if (p && boost::algorithm::iequals(p, "true"))
          t.m_bSaveData = true;
  else
          t.m_bSaveData = false;*/

  p = elemConfig->Attribute("hfov");
  if (p) t.m_strHFov = p;

  p = elemConfig->Attribute("vfov");
  if (p) t.m_strVFov = p;

  p = elemConfig->Attribute("drange");
  if (p) t.m_strDRange = p;

  return 0;
}

int CSensors::ParseIMUs(TiXmlElement* elemIMUs, IMUMap& imus) {
  if (!elemIMUs) return -1;

  imus.clear();

  TiXmlElement* elemConfig = elemIMUs->FirstChildElement("Config");
  while (elemConfig) {
    CSensorIMU imu;

    int nRet = ParseOneIMU(elemConfig, imu);
    if (nRet) {
      SYSTEM_LOGGER_ERROR("parse imus error!");
      assert(false);

      elemConfig = elemConfig->NextSiblingElement("Config");
      continue;
    }

    imus.insert(std::make_pair(imu.m_strID, imu));

    elemConfig = elemConfig->NextSiblingElement("Config");
  }

  return 0;
}

int CSensors::ParseOneIMU(TiXmlElement* elemConfig, CSensorIMU& imu) {
  if (!elemConfig) return -1;

  ParseOneBaseInfo(elemConfig, imu);

  const char* p = elemConfig->Attribute("Enabled");
  if (p && boost::algorithm::iequals(p, "true"))
    imu.m_bEnabled = true;
  else
    imu.m_bEnabled = false;

  p = elemConfig->Attribute("Quaternion");
  if (p) imu.m_strQuaternion = p;

  return 0;
}

int CSensors::ParseGPSes(TiXmlElement* elemGPSes, GPSMap& gpses) {
  if (!elemGPSes) return -1;

  gpses.clear();

  TiXmlElement* elemConfig = elemGPSes->FirstChildElement("Config");
  while (elemConfig) {
    CSensorGPS gps;

    int nRet = ParseOneGPS(elemConfig, gps);
    if (nRet) {
      SYSTEM_LOGGER_ERROR("parse gpses error!");
      assert(false);

      elemConfig = elemConfig->NextSiblingElement("Config");
      continue;
    }

    gpses.insert(std::make_pair(gps.m_strID, gps));

    elemConfig = elemConfig->NextSiblingElement("Config");
  }

  return 0;
}

int CSensors::ParseOneGPS(TiXmlElement* elemConfig, CSensorGPS& gps) {
  if (!elemConfig) return -1;

  ParseOneBaseInfo(elemConfig, gps);

  const char* p = elemConfig->Attribute("Enabled");
  if (p && boost::algorithm::iequals(p, "true"))
    gps.m_bEnabled = true;
  else
    gps.m_bEnabled = false;

  p = elemConfig->Attribute("Quaternion");
  if (p) gps.m_strQuaternion = p;

  return 0;
}

int CSensors::Save(const char* strSensorFile) {
  if (!strSensorFile) return -1;

  TiXmlDocument doc;

  TiXmlDeclaration* dec = new TiXmlDeclaration("1.0", "utf-8", "yes");
  TiXmlElement* elemSensor = new TiXmlElement("Sensor");
  elemSensor->SetAttribute("version", "1.0");

  int nRet = SaveCameras(elemSensor, m_mapCameras);
  if (nRet) {
    assert(false);
  }

  nRet = SaveRadars(elemSensor, m_mapRadars);
  if (nRet) {
    assert(false);
  }

  nRet = SaveTraditionalLidars(elemSensor, m_mapTraditionalLidars);
  if (nRet) {
    assert(false);
  }

  nRet = SaveTruths(elemSensor, m_mapSensorTruths);
  if (nRet) {
    assert(false);
  }

  nRet = SaveIMUs(elemSensor, m_mapIMUs);
  if (nRet) {
    assert(false);
  }

  nRet = SaveGPSes(elemSensor, m_mapGPSes);
  if (nRet) {
    assert(false);
  }
  doc.LinkEndChild(dec);
  doc.LinkEndChild(elemSensor);

  doc.SaveFile(strSensorFile);
  return 0;
}

int CSensors::SaveCameras(TiXmlElement* elemParent, CameraMap& cameras) {
  TiXmlElement* elemCamera = new TiXmlElement("Camera");
  elemCamera->SetAttribute("Active", "1");

  CameraMap::iterator itr = cameras.begin();
  for (; itr != cameras.end(); ++itr) {
    TiXmlElement* elemConfig = new TiXmlElement("Config");

    int nRet = SaveOneCamera(elemConfig, (itr->second));
    if (nRet) {
      SYSTEM_LOGGER_ERROR("save one camera error");
      assert(false);
      delete elemConfig;
      continue;
    }

    elemCamera->LinkEndChild(elemConfig);
  }

  elemParent->LinkEndChild(elemCamera);

  return 0;
}

int CSensors::SaveOneBaseInfo(TiXmlElement* elemBase, CSensorBase& baseInfo) {
  if (!elemBase) {
    return -1;
  }

  elemBase->SetAttribute("ID", baseInfo.m_strID);
  elemBase->SetAttribute("LocationX", baseInfo.m_strLocationX);
  elemBase->SetAttribute("LocationY", baseInfo.m_strLocationY);
  elemBase->SetAttribute("LocationZ", baseInfo.m_strLocationZ);
  elemBase->SetAttribute("RotationX", baseInfo.m_strRotationX);
  elemBase->SetAttribute("RotationY", baseInfo.m_strRotationY);
  elemBase->SetAttribute("RotationZ", baseInfo.m_strRotationZ);
  elemBase->SetAttribute("InstallSlot", baseInfo.m_strInstallSlot);

  return 0;
}

int CSensors::SaveOneCamera(TiXmlElement* elemCamera, CSensorCamera& camera) {
  if (!elemCamera) {
    return -1;
  }

  SaveOneBaseInfo(elemCamera, camera);
  elemCamera->SetAttribute("Enabled", (camera.m_bEnabled ? "true" : "false"));
  elemCamera->SetAttribute("Frequency", camera.m_strFrequency);
  elemCamera->SetAttribute("Device", camera.m_strDevice);
  std::string strIntrinsicParamType = "intrinsic";
  if (camera.m_nIntrinsicParamType == 0) {
    strIntrinsicParamType = "intrinsic";
  } else if (camera.m_nIntrinsicParamType == 1) {
    strIntrinsicParamType = "fov";
  } else if (camera.m_nIntrinsicParamType == 2) {
    strIntrinsicParamType = "ccd";
  }
  elemCamera->SetAttribute("IntrinsicParamType", strIntrinsicParamType.c_str());
  // elemCamera->SetAttribute("SaveData", (camera.m_bSaveData?"true":"false"));
  elemCamera->SetAttribute("DisplayMode", camera.m_strDisplayMode);
  elemCamera->SetAttribute("FOV_Horizontal", camera.m_strFOVHorizontal);
  elemCamera->SetAttribute("FOV_Vertical", camera.m_strFOVVertical);
  elemCamera->SetAttribute("Res_Horizontal", camera.m_strResHorizontal);
  elemCamera->SetAttribute("Res_Vertical", camera.m_strResVertical);
  elemCamera->SetAttribute("Blur_Intensity", camera.m_strBlurIntensity);
  elemCamera->SetAttribute("MotionBlur_Amount", camera.m_strMotionBlurAmount);
  elemCamera->SetAttribute("Vignette_Intensity", camera.m_strVignetteIntensity);
  elemCamera->SetAttribute("Noise_Intensity", camera.m_strNoiseIntensity);
  elemCamera->SetAttribute("Intrinsic_Matrix", camera.m_strIntrinsicMatrix);
  // elemCamera->SetAttribute("LensType", camera.m_strLens);
  elemCamera->SetAttribute("Distortion_Parameters", camera.m_strDistortionParamters);
  elemCamera->SetAttribute("CCD_Width", camera.m_strCCDWidth);
  elemCamera->SetAttribute("CCD_Height", camera.m_strCCDHeight);
  elemCamera->SetAttribute("Focal_Length", camera.m_strFocalLength);
  return 0;
}

int CSensors::SaveRadars(TiXmlElement* elemParent, RadarMap& radars) {
  TiXmlElement* elemRadar = new TiXmlElement("Radar");
  elemRadar->SetAttribute("Active", "1");

  RadarMap::iterator itr = radars.begin();
  for (; itr != radars.end(); ++itr) {
    TiXmlElement* elemConfig = new TiXmlElement("Config");

    int nRet = SaveOneRadar(elemConfig, (itr->second));
    if (nRet) {
      SYSTEM_LOGGER_ERROR("save one radar error");
      assert(false);
      delete elemConfig;
      continue;
    }

    elemRadar->LinkEndChild(elemConfig);
  }

  elemParent->LinkEndChild(elemRadar);

  return 0;
}

int CSensors::SaveOneRadar(TiXmlElement* elemRadar, CSensorRadar& radar) {
  if (!elemRadar) {
    return -1;
  }

  SaveOneBaseInfo(elemRadar, radar);

  elemRadar->SetAttribute("Enabled", (radar.m_bEnabled ? "true" : "false"));
  elemRadar->SetAttribute("Frequency", radar.m_strFrequency);
  elemRadar->SetAttribute("Device", radar.m_strDevice);
  // elemRadar->SetAttribute("SaveData", (radar.m_bSaveData?"true":"false"));
  elemRadar->SetAttribute("F0_GHz", radar.m_strF0GHz);
  elemRadar->SetAttribute("Pt_dBm", radar.m_strPtDBm);
  elemRadar->SetAttribute("Gt_dBi", radar.m_strGTDBi);
  elemRadar->SetAttribute("Gr_dBi", radar.m_strGrDBi);
  elemRadar->SetAttribute("Ts_K", radar.m_strTsK);
  elemRadar->SetAttribute("Fn_dB", radar.m_strFnDB);
  elemRadar->SetAttribute("L0_dB", radar.m_strL0DB);
  elemRadar->SetAttribute("SNR_min_dB", radar.m_strSNRMinDB);
  elemRadar->SetAttribute("radar_angle", radar.m_strRadarAngle);
  elemRadar->SetAttribute("R_m", radar.m_strRM);
  elemRadar->SetAttribute("rcs", radar.m_strRcs);
  elemRadar->SetAttribute("weather", radar.m_strWeather);
  elemRadar->SetAttribute("tag", radar.m_strTag);
  elemRadar->SetAttribute("anne_tag", radar.m_strAnneTag);
  elemRadar->SetAttribute("hwidth", radar.m_strHWidth);
  elemRadar->SetAttribute("vwidth", radar.m_strVWidth);
  elemRadar->SetAttribute("vfov", radar.m_strVFov);
  elemRadar->SetAttribute("hfov", radar.m_strHFov);
  elemRadar->SetAttribute("ANTENNA_ANGLE_path1", radar.m_strAntennaAnglePath1);
  elemRadar->SetAttribute("ANTENNA_ANGLE_path2", radar.m_strAntennaAnglePath2);

  return 0;
}

int CSensors::SaveTraditionalLidars(TiXmlElement* elemParent, TraditionalLidarMap& traditionLidars) {
  TiXmlElement* elemTraditionalLidar = new TiXmlElement("TraditionalLidar");
  elemTraditionalLidar->SetAttribute("Active", "1");

  TraditionalLidarMap::iterator itr = traditionLidars.begin();
  for (; itr != traditionLidars.end(); ++itr) {
    TiXmlElement* elemConfig = new TiXmlElement("Config");

    int nRet = SaveOneTraditionalLidar(elemConfig, (itr->second));
    if (nRet) {
      SYSTEM_LOGGER_ERROR("save one traditional lidar error");
      assert(false);
      delete elemConfig;
      continue;
    }

    elemTraditionalLidar->LinkEndChild(elemConfig);
  }

  elemParent->LinkEndChild(elemTraditionalLidar);

  return 0;
}

int CSensors::SaveOneTraditionalLidar(TiXmlElement* elemTraditionalLidar, CSensorTraditionalLindar& traditionalLidar) {
  if (!elemTraditionalLidar) {
    return -1;
  }

  SaveOneBaseInfo(elemTraditionalLidar, traditionalLidar);

  elemTraditionalLidar->SetAttribute("Enabled", (traditionalLidar.m_bEnabled ? "true" : "false"));
  elemTraditionalLidar->SetAttribute("Frequency", traditionalLidar.m_strFrequency);
  elemTraditionalLidar->SetAttribute("Device", traditionalLidar.m_strDevice);
  elemTraditionalLidar->SetAttribute("DrawPoint", (traditionalLidar.m_bDrawPoint ? "true" : "false"));
  elemTraditionalLidar->SetAttribute("DrawRay", (traditionalLidar.m_bDrawRay ? "true" : "false"));
  elemTraditionalLidar->SetAttribute("Model", traditionalLidar.m_strModel);
  elemTraditionalLidar->SetAttribute("uChannels", traditionalLidar.m_strUChannels);
  elemTraditionalLidar->SetAttribute("uRange", traditionalLidar.m_strURange);
  elemTraditionalLidar->SetAttribute("uHorizontalResolution", traditionalLidar.m_strUHorizontalResolution);
  elemTraditionalLidar->SetAttribute("uUpperFov", traditionalLidar.m_strUUpperFov);
  elemTraditionalLidar->SetAttribute("uLowerFov", traditionalLidar.m_strULowerFov);
  // elemTraditionalLidar->SetAttribute("uSaveData", (traditionalLidar.m_bUSaveData ? "true" : "false"));

  return 0;
}

int CSensors::SaveTruths(TiXmlElement* elemParent, SensorTruthMap& truths) {
  TiXmlElement* elemTruth = new TiXmlElement("Truth");
  elemTruth->SetAttribute("Active", "1");

  SensorTruthMap::iterator itr = truths.begin();
  for (; itr != truths.end(); ++itr) {
    TiXmlElement* elemConfig = new TiXmlElement("Config");

    int nRet = SaveOneTruth(elemConfig, (itr->second));
    if (nRet) {
      SYSTEM_LOGGER_ERROR("save one truth error");
      assert(false);
      delete elemConfig;
      continue;
    }

    elemTruth->LinkEndChild(elemConfig);
  }

  elemParent->LinkEndChild(elemTruth);

  return 0;
}

int CSensors::SaveOneTruth(TiXmlElement* elemTruth, CSensorTruth& truth) {
  if (!elemTruth) {
    return -1;
  }

  SaveOneBaseInfo(elemTruth, truth);

  elemTruth->SetAttribute("Enabled", (truth.m_bEnabled ? "true" : "false"));
  elemTruth->SetAttribute("Device", truth.m_strDevice);
  // elemTruth->SetAttribute("SaveData", (truth.m_bSaveData ? "true" : "false"));
  elemTruth->SetAttribute("vfov", truth.m_strVFov);
  elemTruth->SetAttribute("hfov", truth.m_strHFov);
  elemTruth->SetAttribute("drange", truth.m_strDRange);

  return 0;
}

int CSensors::SaveIMUs(TiXmlElement* elemParent, IMUMap& imus) {
  TiXmlElement* elemIMU = new TiXmlElement("IMU");
  elemIMU->SetAttribute("Active", "1");

  IMUMap::iterator itr = imus.begin();
  for (; itr != imus.end(); ++itr) {
    TiXmlElement* elemConfig = new TiXmlElement("Config");

    int nRet = SaveOneIMU(elemConfig, (itr->second));
    if (nRet) {
      SYSTEM_LOGGER_ERROR("save one imu error");
      assert(false);
      delete elemConfig;
      continue;
    }

    elemIMU->LinkEndChild(elemConfig);
  }

  elemParent->LinkEndChild(elemIMU);

  return 0;
}

int CSensors::SaveOneIMU(TiXmlElement* elemIMU, CSensorIMU& imu) {
  if (!elemIMU) {
    return -1;
  }

  SaveOneBaseInfo(elemIMU, imu);

  elemIMU->SetAttribute("Enabled", (imu.m_bEnabled ? "true" : "false"));
  elemIMU->SetAttribute("Quaternion", imu.m_strQuaternion);

  return 0;
}

int CSensors::SaveGPSes(TiXmlElement* elemParent, GPSMap& gpses) {
  TiXmlElement* elemGPS = new TiXmlElement("GPS");
  elemGPS->SetAttribute("Active", "1");

  GPSMap::iterator itr = gpses.begin();
  for (; itr != gpses.end(); ++itr) {
    TiXmlElement* elemConfig = new TiXmlElement("Config");

    int nRet = SaveOneGPS(elemConfig, (itr->second));
    if (nRet) {
      SYSTEM_LOGGER_ERROR("save one gps error");
      assert(false);
      delete elemConfig;
      continue;
    }

    elemGPS->LinkEndChild(elemConfig);
  }

  elemParent->LinkEndChild(elemGPS);

  return 0;
}

int CSensors::SaveOneGPS(TiXmlElement* elemGPS, CSensorGPS& gps) {
  if (!elemGPS) {
    return -1;
  }

  SaveOneBaseInfo(elemGPS, gps);

  elemGPS->SetAttribute("Enabled", (gps.m_bEnabled ? "true" : "false"));
  elemGPS->SetAttribute("Quaternion", gps.m_strQuaternion);

  return 0;
}
