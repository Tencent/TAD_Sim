/**
 * @file camera_sensor.cpp
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-03-19
 *
 * @copyright Copyright (c) 2024
 *
 */
#include <fstream>
#include "../camera_sensor.h"
#include "gtest/gtest.h"


/**
 * @brief Construct a new TEST object
 *
 */
TEST(CameraSensorTest, Load) {
  std::string xmlFile = "SensorCatalog.xosc";
  std::string group = "2";
  std::string device = "";

  std::ofstream test(xmlFile);
  test << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n<TADSim>\n    "
          "<FileHeader revMajor=\"1\" revMinor=\"0\" "
          "date=\"2023-03-08T17:00:00\" author=\"marsyu\" />\n    <Catalog "
          "name=\"SensorGroup\">\n        <SensorGroup name=\"1\">\n            "
          "<Sensor idx=\"12\" ID=\"0\" InstallSlot=\"C0\" Device=\".0\" "
          "LocationX=\"185.2259\" LocationY=\"2.3111\" LocationZ=\"163.1456\" "
          "RotationX=\"0.223021\" RotationY=\"-0.643842\" "
          "RotationZ=\"0.378665\" />\n            <Sensor idx=\"12\" ID=\"1\" "
          "InstallSlot=\"C0\" Device=\".0\" LocationX=\"161.2965\" "
          "LocationY=\"63.4211\" LocationZ=\"168.9438\" RotationX=\"0.392487\" "
          "RotationY=\"4.220168\" RotationZ=\"104.439330\" />\n            "
          "<Sensor idx=\"12\" ID=\"2\" InstallSlot=\"C0\" Device=\".0\" "
          "LocationX=\"-32.3069\" LocationY=\"-2.7995\" LocationZ=\"161.4423\" "
          "RotationX=\"0.589193\" RotationY=\" -0.595110\" "
          "RotationZ=\"179.875227\" />\n            <Sensor idx=\"12\" ID=\"3\" "
          "InstallSlot=\"C0\" Device=\".0\" LocationX=\"164.635700\" "
          "LocationY=\"-62.0490\" LocationZ=\"167.935200\" "
          "RotationX=\"-0.105822\" RotationY=\"5.222701\" "
          "RotationZ=\"-104.616938\" />\n        </SensorGroup>\n        "
          "<SensorGroup name=\"2\">\n   <Sensor idx=\"0\" "
          "ID=\"0\" InstallSlot=\"C0\" Device=\".0\" LocationX=\"185.2259\" "
          "LocationY=\"2.3111\" LocationZ=\"163.1456\" RotationX=\"0.223021\" "
          "RotationY=\"-0.643842\" RotationZ=\"0.378665\" />\n            "
          "<Sensor idx=\"0\" ID=\"1\" InstallSlot=\"C0\" Device=\".0\" "
          "LocationX=\"161.2965\" LocationY=\"63.4211\" LocationZ=\"168.9438\" "
          "RotationX=\"0.392487\" RotationY=\"4.220168\" "
          "RotationZ=\"104.439330\" />\n            <Sensor idx=\"0\" ID=\"2\" "
          "InstallSlot=\"C0\" Device=\".0\" LocationX=\"-32.3069\" "
          "LocationY=\"-2.7995\" LocationZ=\"161.4423\" RotationX=\"0.589193\" "
          "RotationY=\" -0.595110\" RotationZ=\"179.875227\" />\n            "
          "<Sensor idx=\"0\" ID=\"3\" InstallSlot=\"C0\" Device=\".0\" "
          "LocationX=\"164.635700\" LocationY=\"-62.0490\" "
          "LocationZ=\"167.935200\" RotationX=\"-0.105822\" "
          "RotationY=\"5.222701\" RotationZ=\"-104.616938\" />\n  "
          "</SensorGroup>\n        <SensorGroup name=\"3\">\n  "
          " <Sensor idx=\"9\" ID=\"0\" InstallSlot=\"C0\" Device=\".0\" "
          "LocationX=\"185.2259\" LocationY=\"2.3111\" LocationZ=\"163.1456\" "
          "RotationX=\"0.223021\" RotationY=\"-0.643842\" "
          "RotationZ=\"0.378665\" />\n            <Sensor idx=\"9\" ID=\"1\" "
          "InstallSlot=\"C0\" Device=\".0\" LocationX=\"161.2965\" "
          "LocationY=\"63.4211\" LocationZ=\"168.9438\" RotationX=\"0.392487\" "
          "RotationY=\"4.220168\" RotationZ=\"104.439330\" />\n            "
          "<Sensor idx=\"9\" ID=\"2\" InstallSlot=\"C0\" Device=\".0\" "
          "LocationX=\"-32.3069\" LocationY=\"-2.7995\" LocationZ=\"161.4423\" "
          "RotationX=\"0.589193\" RotationY=\" -0.595110\" "
          "RotationZ=\"179.875227\" />\n            <Sensor idx=\"9\" ID=\"3\" "
          "InstallSlot=\"C0\" Device=\".0\" LocationX=\"164.635700\" "
          "LocationY=\"-62.0490\" LocationZ=\"167.935200\" "
          "RotationX=\"-0.105822\" RotationY=\"5.222701\" "
          "RotationZ=\"-104.616938\" />\n  </SensorGroup>\n       "
          " <SensorGroup name=\"4\">\n   <Sensor idx=\"1\" "
          "ID=\"0\" InstallSlot=\"C0\" Device=\".0\" LocationX=\"185.2259\" "
          "LocationY=\"2.3111\" LocationZ=\"163.1456\" RotationX=\"0.223021\" "
          "RotationY=\"-0.643842\" RotationZ=\"0.378665\" />\n            "
          "<Sensor idx=\"1\" ID=\"1\" InstallSlot=\"C0\" Device=\".0\" "
          "LocationX=\"161.2965\" LocationY=\"63.4211\" LocationZ=\"168.9438\" "
          "RotationX=\"0.392487\" RotationY=\"4.220168\" "
          "RotationZ=\"104.439330\" />\n            <Sensor idx=\"1\" ID=\"2\" "
          "InstallSlot=\"C0\" Device=\".0\" LocationX=\"-32.3069\" "
          "LocationY=\"-2.7995\" LocationZ=\"161.4423\" RotationX=\"0.589193\" "
          "RotationY=\" -0.595110\" RotationZ=\"179.875227\" />\n            "
          "<Sensor idx=\"1\" ID=\"3\" InstallSlot=\"C0\" Device=\".0\" "
          "LocationX=\"164.635700\" LocationY=\"-62.0490\" "
          "LocationZ=\"167.935200\" RotationX=\"-0.105822\" "
          "RotationY=\"5.222701\" RotationZ=\"-104.616938\" />\n  "
          "</SensorGroup>\n        <SensorGroup name=\"5\" />\n        "
          "<SensorGroup name=\"6\" />\n        <SensorGroup name=\"7\" />\n     "
          "   <SensorGroup name=\"8\" />\n        <SensorGroup name=\"9\" />\n  "
          "      <SensorGroup name=\"10\" />\n        <SensorGroup name=\"11\" "
          "/>\n        <SensorGroup name=\"12\" />\n    </Catalog>\n    "
          "<Catalog name=\"SensorDefine\">\n        <!-- "
          "s -->\n        <Camera idx=\"0\" "
          "name=\"Camera\" Aperture=\"4\" Bloom=\"0.675\" "
          "ColorTemperature=\"6500\" CCD_Width=\"10\" Blur_Intensity=\"0\" "
          "CCD_Height=\"10\" Compensation=\"1\" DisplayMode=\"0\" "
          "Exquisite=\"0\" "
          "Distortion_Parameters=\"-0.5333680427825355,0.4577346209269804,-0."
          "4836076527200365,-0.0023929586859060227,0.0018364297357014352\" "
          "ExposureMode=\"0\" MotionBlur_Amount=\"0\" FOV_Horizontal=\"60\" "
          "Vignette_Intensity=\"0\" FOV_Vertical=\"36.28\" Focal_Length=\"10\" "
          "Frequency=\"25\" ISO=\"100\" IntrinsicParamType=\"0\" "
          "Intrinsic_Matrix=\"1945.1674168728503,0,946.1188960408923,0,1938."
          "137228006907,619.7048547473978,0,0,1\" LensFlares=\"0\" "
          "Noise_Intensity=\"0\" Res_Horizontal=\"1920\" Res_Vertical=\"1208\" "
          "ShutterSpeed=\"60\" Transmittance=\"98\" WhiteHint=\"0\" />\n        "
          "<Radar idx=\"1\" name=\"Radar\" ANTENNA_ANGLE_path1=\"\" "
          "Gt_dBi=\"15\" Fn_dB=\"12\" ANTENNA_ANGLE_path2=\"\" F0_GHz=\"77\" "
          "rcs=\"7.9\" Frequency=\"10\" Gr_dBi=\"15\" Ts_K=\"300\" L0_dB=\"10\" "
          "SNR_min_dB=\"18\" Pt_dBm=\"10\" R_m=\"50\" anne_tag=\"1\" "
          "delay=\"0\" hfov=\"20\" hwidth=\"3\" radar_angle=\"0\" tag=\"0\" "
          "vfov=\"10\" vwidth=\"3\" weather=\"0\" />\n        <TraditionalLidar "
          "FovStart=\"0\" idx=\"12\" name=\"TraditionalLidar\" "
          "Port=\"2368,2001\" FovEnd=\"360\" AngleDefinition=\"\" "
          "Attenuation=\"0\" ExtinctionCoe=\"0\" DrawPoint=\"true\" "
          "DrawRay=\"true\" Frequency=\"10\" IP=\"\" Model=\"HS128AT\" "
          "ReflectionType=\"0\" uLowerFov=\"-10\" uChannels=\"16\" "
          "uHorizontalResolution=\"1\" uRange=\"15000\" uUpperFov=\"10\" />\n   "
          "     <Truth idx=\"3\" drange=\"2000\" vfov=\"180\" name=\"Truth\" "
          "hfov=\"360\" />\n        <IMU idx=\"4\" name=\"IMU\" "
          "Quaternion=\"0,0,-0.70710678118,0.70710678118\" />\n        <GPS "
          "idx=\"5\" name=\"GPS\" Quaternion=\"0,0,0,1\" />\n        <Fisheye "
          "idx=\"6\" name=\"Fisheye\" Aperture=\"4\" Bloom=\"0.675\" "
          "DisplayMode=\"0\" Exquisite=\"0\" ColorTemperature=\"6500\" "
          "Blur_Intensity=\"0\" "
          "Distortion_Parameters=\"0.00298329,0.04234368,-0.00231007,-0."
          "0028992\" Compensation=\"1\" ExposureMode=\"0\" "
          "MotionBlur_Amount=\"0\" Frequency=\"50\" ISO=\"100\" "
          "IntrinsicParamType=\"0\" "
          "Intrinsic_Matrix=\"337.14139734,0,641.86019172,0,336.24194116,543."
          "23107903,0,0,1\" LensFlares=\"0\" Noise_Intensity=\"0\" "
          "Res_Horizontal=\"1200\" Res_Vertical=\"1080\" ShutterSpeed=\"60\" "
          "Transmittance=\"98\" Vignette_Intensity=\"0\" WhiteHint=\"0\" />\n   "
          "     <Semantic idx=\"7\" name=\"Semantic\" DisplayMode=\"0\" "
          "CCD_Width=\"10\" Blur_Intensity=\"0\" FOV_Horizontal=\"60\" "
          "Vignette_Intensity=\"0\" CCD_Height=\"10\" FOV_Vertical=\"36.28\" "
          "Distortion_Parameters=\"-0.5333680427825355,0.4577346209269804,-0."
          "4836076527200365,-0.0023929586859060227,0.0018364297357014352\" "
          "Focal_Length=\"10\" Frequency=\"25\" IntrinsicParamType=\"0\" "
          "Intrinsic_Matrix=\"1945.1674168728503,0,946.1188960408923,0,1938."
          "137228006907,619.7048547473978,0,0,1\" MotionBlur_Amount=\"0\" "
          "Noise_Intensity=\"0\" Res_Horizontal=\"1920\" Res_Vertical=\"1208\" "
          "/>\n        <Depth idx=\"8\" name=\"Depth\" DisplayMode=\"0\" "
          "CCD_Width=\"10\" Blur_Intensity=\"0\" FOV_Horizontal=\"60\" "
          "Vignette_Intensity=\"0\" CCD_Height=\"10\" FOV_Vertical=\"36.28\" "
          "Distortion_Parameters=\"-0.5333680427825355,0.4577346209269804,-0."
          "4836076527200365,-0.0023929586859060227,0.0018364297357014352\" "
          "Focal_Length=\"10\" Frequency=\"25\" IntrinsicParamType=\"0\" "
          "Intrinsic_Matrix=\"1945.1674168728503,0,946.1188960408923,0,1938."
          "137228006907,619.7048547473978,0,0,1\" MotionBlur_Amount=\"0\" "
          "Noise_Intensity=\"0\" Res_Horizontal=\"1920\" Res_Vertical=\"1208\" "
          "/>\n        <Ultrasonic IndirectDistance=\"0\" "
          "AttachmentType=\"None\" idx=\"9\" name=\"Ultrasonic\" "
          "FOV_Horizontal=\"60\" AttachmentRange=\"0.5\" Distance=\"200\" "
          "PulseMoment=\"0\" FOV_Vertical=\"30\" Frequency=\"40000\" "
          "NoiseFactor=\"5\" NoiseStd=\"5\" PulsePeriod=\"100\" Radius=\"0.01\" "
          "dBmin=\"-6\" />\n        <OBU v2x_broad_speed=\"1\" idx=\"10\" "
          "ConnectionDistance=\"300\" name=\"OBU\" v2x_delay_normal_mean=\"45\" "
          "Frequency=\"30\" v2x_bandwidth=\"10\" v2x_delay_fixed_time=\"45\" "
          "v2x_freq_channel=\"5.9\" v2x_delay_normal_deviation=\"5\" "
          "v2x_delay_type=\"0\" v2x_delay_uniform_max=\"45\" "
          "v2x_delay_uniform_min=\"40\" v2x_loss_burs_max=\"5\" "
          "v2x_loss_burs_min=\"1\" v2x_loss_burs_prob=\"1\" "
          "v2x_loss_rand_prob=\"1\" v2x_loss_type=\"0\" />\n    "
          "</Catalog>\n</TADSim>\n";
  test.close();

  // EXPECT_EQ(cameras.size(), 4);
}

/**
 * @brief Construct a new TEST object
 *
 */
TEST(CameraSensorTest, Fun) {
  CameraSensor camera(1);

  camera.setResolution(1920, 1208);
  Eigen::Matrix3d insmat = Eigen::Matrix3d::Identity();
  insmat(0, 2) = 946.1188960408923;
  insmat(1, 2) = 619.7048547473978;
  insmat(0, 0) = 1945.1674168728503;
  insmat(1, 1) = 1938.137228006907;
  camera.setIntrinsic(insmat);
  camera.setDistortion(-0.5333680427825355, 0.4577346209269804, -0.4836076527200365, -0.0023929586859060227,
                       0.0018364297357014352);
  camera.setFPS(30);
  camera.setRotation(0, 0, 0);
  camera.setPosition(Eigen::Vector3d(0, 0, 0));
  camera.setCarPosition(Eigen::Vector3d(0, 0, 0));
  camera.setCarRoatation(0, 0, 0);

  EXPECT_TRUE(camera.Init());
  EXPECT_EQ(camera.ID(), 1);
  EXPECT_EQ(camera.FPS(), 30);
  EXPECT_EQ(camera.W(), 1920);
  EXPECT_EQ(camera.H(), 1208);
  EXPECT_TRUE(camera.inFov(Eigen::Vector3d(30, 0, 0)));
  auto uv = camera.world2pixel(Eigen::Vector3d(30, 0, 0));
  EXPECT_TRUE(uv.x() > 946 && uv.x() < 947);
  EXPECT_TRUE(uv.y() > 619 && uv.y() < 620);
}
