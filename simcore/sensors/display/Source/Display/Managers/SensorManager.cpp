// Fill out your copyright notice in the Description page of Project Settings.

#include "SensorManager.h"
#include "Engine/World.h"
#include "XmlFile.h"
#include "Framework/DisplayPlayerController.h"
#include "Framework/DisplayGameInstance.h"
#include "Objects/Sensors/SensorFactory.h"
#include "Objects/Sensors/CameraSensors/CameraSensor.h"
#include "Objects/Sensors/LidarSensors/TLidarSensor.h"
#include "Objects/Sensors/CameraSensors/FisheyeSensor.h"
#include "Objects/Sensors/CameraSensors/DepthSensor.h"
#include "Kismet/KismetMathLibrary.h"
#include <thread>
#include "scene.pb.h"

DEFINE_LOG_CATEGORY_STATIC(SimLogSensorManager, Log, All);

void GetPropValue(const TMap<FString, FString>& params, const FString& para, double& value)
{
    if (params.Find(para))
    {
        value = FCString::Atod(**params.Find(para));
    }
};

void GetPropValue(const TMap<FString, FString>& params, const FString& para, float& value)
{
    if (params.Find(para))
    {
        value = FCString::Atof(**params.Find(para));
    }
};

void GetPropValue(const TMap<FString, FString>& params, const FString& para, int& value)
{
    if (params.Find(para))
    {
        value = FCString::Atoi(**params.Find(para));
    }
};
void GetPropValue(const TMap<FString, FString>& params, const FString& para, FString& value)
{
    if (params.Find(para))
    {
        value = *params.Find(para);
    }
};
void GetPropValue(const TMap<FString, FString>& params, const FString& para, bool& value)
{
    if (params.Find(para))
    {
        value = true;
        auto v = params.Find(para)->ToLower();
        if (v == TEXT("false") || v == TEXT("0") || v == TEXT("close") || v == TEXT("disable"))
            value = false;
    }
};

ASensorManager::ASensorManager()
{
}

void ASensorManager::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(SimLogSensorManager, Log, TEXT("Spawn SensorManager!"));
}

FSensorManagerConfig ASensorManager::ParseSensorString(const std::string& buffer, int64 EgoId)
{
    UE_LOG(SimLogSensorManager, Log, TEXT("Parse config from pb"));
    FSensorManagerConfig SensormanagerConfig;
    if (buffer.empty())
    {
        UE_LOG(SimLogSensorManager, Log, TEXT("pb is empty"));
        return SensormanagerConfig;
    }
    sim_msg::Scene scene;
    if (!scene.ParseFromString(buffer))
    {
        UE_LOG(SimLogSensorManager, Warning, TEXT("ParseFromString faild."));
        return SensormanagerConfig;
    }

    FString BaseSavePath;
    if (GConfig->GetString(TEXT("Sensor"), TEXT("SavePath"), BaseSavePath, GGameIni))
    {
        bool absPath = false;
#ifdef WIN32
        absPath = BaseSavePath.Find(TEXT(":")) > INDEX_NONE;
#else
        absPath = BaseSavePath.Len() > 0 && BaseSavePath[0] == TEXT('/');
#endif    // WIN32

        if (!absPath)
            BaseSavePath = FPaths::ProjectSavedDir() + BaseSavePath;
        BaseSavePath += TEXT("Ego_") + FString::FromInt(EgoId + 1) + TEXT("/");
        FString FileName = TEXT("SensorData");
        FileName += TEXT("_");
        FString Date;
        FDateTime datetime;
        UObject* parent = nullptr;
        if (UKismetMathLibrary::Now().ExportTextItem(Date, datetime, parent, 1, parent))
        {
            FileName += Date;
            FileName += TEXT("/");
        }
        BaseSavePath += FileName;
        UE_LOG(LogTemp, Log, TEXT("SensorManger: savePath is: %s."), *BaseSavePath);
    }

    FString device = TEXT("0");
    {
        if (!FParse::Value(FCommandLine::Get(), TEXT("-device="), device))
        {
            device = TEXT("0");
        }
        int idx = 0;
        if (device.FindChar('.', idx))
        {
            device = device.Mid(idx + 1);
        }
    }

    TArray<FUltrasonicConfig> all_ultrasonicArry;

    UE_LOG(LogTemp, Log, TEXT("SensorManger: device is: %s."), *device);

    // 目前主车id都为0，EgoId先用主车索引代替
    int32 FindEgoIndex = EgoId;
    /* for (int32 i = 0; i < scene.egos().size(); i++)
     {
         if (EgoId == scene.egos(i).id())
         {
             FindEgoIndex = i;
             break;
         }
     }*/
    if (FindEgoIndex < 0)
    {
        UE_LOG(SimLogSensorManager, Error, TEXT("Find ego index failed"));
        return SensormanagerConfig;
    }
    else
    {
        UE_LOG(SimLogSensorManager, Log, TEXT("Find ego index %d, Id %ld"), FindEgoIndex, EgoId);
    }

    UE_LOG(
        LogTemp, Log, TEXT("SensorManger: sensors count: %d."), scene.egos(FindEgoIndex).sensor_group().sensors_size());

    for (const auto& sensor : scene.egos(FindEgoIndex).sensor_group().sensors())
    {
        if (sensor.type() == sim_msg::SENSOR_TYPE_ULTRASONIC)
        {
            FUltrasonicConfig NewConfig;
            NewConfig.id = sensor.extrinsic().id();
            NewConfig.installSlot = ANSI_TO_TCHAR(sensor.extrinsic().installslot().c_str());
            NewConfig.installLocation.X = sensor.extrinsic().locationx();
            NewConfig.installLocation.Y = sensor.extrinsic().locationy();
            NewConfig.installLocation.Z = sensor.extrinsic().locationz();
            NewConfig.installRotation.Roll = sensor.extrinsic().rotationx();
            NewConfig.installRotation.Pitch = sensor.extrinsic().rotationy();
            NewConfig.installRotation.Yaw = sensor.extrinsic().rotationz();

            // Coordinate transform
            CoordinateTransform_RightHandToLeftHand(NewConfig.installLocation, NewConfig.installRotation);

            NewConfig.targetType = ETrafficType::ST_Ego;
            NewConfig.targetId = EgoId;
            // TypeName
            NewConfig.typeName = TEXT("Ultrasonic");
            FString attachtype;

            TMap<FString, FString> Config;
            for (const auto& ss : sensor.intrinsic().params())
            {
                Config.Add(ANSI_TO_TCHAR(ss.first.c_str())) = ANSI_TO_TCHAR(ss.second.c_str());
            }

            GetPropValue(Config, FString(TEXT("FOV_Horizontal")), NewConfig.azimuth_fov);
            GetPropValue(Config, FString(TEXT("FOV_Vertical")), NewConfig.elevation_fov);
            GetPropValue(Config, FString(TEXT("Distance")), NewConfig.scan_distance);

            // new
            GetPropValue(Config, FString(TEXT("FovHorizional")), NewConfig.azimuth_fov);
            GetPropValue(Config, FString(TEXT("FovVertial")), NewConfig.elevation_fov);
            GetPropValue(Config, FString(TEXT("IndirectDistance")), NewConfig.openIndirectDistance);
            GetPropValue(Config, FString(TEXT("PulseMoment")), NewConfig.timePulse0);
            GetPropValue(Config, FString(TEXT("PulsePeriod")), NewConfig.timePulsePeriod);
            GetPropValue(Config, FString(TEXT("PollTurn")), NewConfig.pollTurn);

            all_ultrasonicArry.Add(NewConfig);
        }
    }

    for (const auto& sensor : scene.egos(FindEgoIndex).sensor_group().sensors())
    {
        FSensorConfig Base;
        Base.device = ANSI_TO_TCHAR(sensor.extrinsic().device().c_str());
        {
            int idx = 0;
            if (Base.device.FindChar('.', idx))
            {
                Base.device = Base.device.Mid(idx + 1);
            }
        }
        if (Base.device != device)
        {
            UE_LOG(LogTemp, Log, TEXT("SensorManger: config skip: deivce=%s, id=%d"), *Base.device, Base.id);
            continue;
        }
        Base.id = sensor.extrinsic().id();
        Base.installSlot = ANSI_TO_TCHAR(sensor.extrinsic().installslot().c_str());
        Base.installLocation.X = sensor.extrinsic().locationx();
        Base.installLocation.Y = sensor.extrinsic().locationy();
        Base.installLocation.Z = sensor.extrinsic().locationz();
        Base.installRotation.Roll = sensor.extrinsic().rotationx();
        Base.installRotation.Pitch = sensor.extrinsic().rotationy();
        Base.installRotation.Yaw = sensor.extrinsic().rotationz();
        UE_LOG(LogTemp, Log, TEXT("SensorManger: sensor[%d] ~s device is %s"), Base.id, *Base.device);
        UE_LOG(LogTemp, Log, TEXT("SensorManger: sensor[%d] ~s install pos is %s, %s"), Base.id,
            *Base.installLocation.ToString(), *Base.installRotation.ToString());

        // Coordinate transform
        CoordinateTransform_RightHandToLeftHand(Base.installLocation, Base.installRotation);

        Base.targetType = ETrafficType::ST_Ego;
        Base.targetId = 0;

        TMap<FString, FString> Config;
        for (const auto& ss : sensor.intrinsic().params())
        {
            UE_LOG(LogTemp, Log, TEXT("SensorManger: %s: %s"), ANSI_TO_TCHAR(ss.first.c_str()),
                ANSI_TO_TCHAR(ss.second.c_str()));
            Config.Add(ANSI_TO_TCHAR(ss.first.c_str())) = ANSI_TO_TCHAR(ss.second.c_str());
        }

        if (sensor.type() == sim_msg::SENSOR_TYPE_CAMERA)
        {
            FCameraConfig NewConfig;
            *(FSensorConfig*) &NewConfig = Base;
            NewConfig.typeName = TEXT("Camera");
            NewConfig.targetId = EgoId;
            FString Intrinsic_Matrix = TEXT("");
            FString distortion_Parameters = TEXT("");

            // old
            GetPropValue(Config, FString(TEXT("CCD_Width")), NewConfig.ccd_Width);              ////---
            GetPropValue(Config, FString(TEXT("CCD_Height")), NewConfig.ccd_Height);            ////---
            GetPropValue(Config, FString(TEXT("Focal_Length")), NewConfig.focal_Length);        ////---
            GetPropValue(Config, FString(TEXT("FOV_Horizontal")), NewConfig.fov_Horizontal);    ////---
            GetPropValue(Config, FString(TEXT("FOV_Vertical")), NewConfig.fov_Vertical);        ////---
            GetPropValue(Config, FString(TEXT("Res_Horizontal")), NewConfig.res_Horizontal);    ////---
            GetPropValue(Config, FString(TEXT("Res_Vertical")), NewConfig.res_Vertical);        ////---
            GetPropValue(Config, FString(TEXT("IntrinsicParamType")), NewConfig.paraType);
            GetPropValue(Config, FString(TEXT("Intrinsic_Matrix")), Intrinsic_Matrix);              ////---
            GetPropValue(Config, FString(TEXT("Distortion_Parameters")), distortion_Parameters);    /////----
            GetPropValue(Config, FString(TEXT("Blur_Intensity")), NewConfig.blur_Intensity);
            GetPropValue(Config, FString(TEXT("MotionBlur_Amount")), NewConfig.motionBlur_Amount);
            GetPropValue(Config, FString(TEXT("Noise_Intensity")), NewConfig.noise_Intensity);
            GetPropValue(Config, FString(TEXT("Vignette_Intensity")), NewConfig.vignette_Intensity);
            GetPropValue(Config, FString(TEXT("DisplayMode")), NewConfig.color_gray);
            FString dm;
            GetPropValue(Config, FString(TEXT("DisplayMode")), dm);
            if (dm == TEXT("Gray"))
            {
                NewConfig.color_gray = 1;
            }

            GetPropValue(Config, FString(TEXT("Frequency")), NewConfig.frequency);
            GetPropValue(Config, FString(TEXT("CcdWidth")), NewConfig.ccd_Width);
            GetPropValue(Config, FString(TEXT("CcdHeight")), NewConfig.ccd_Height);
            GetPropValue(Config, FString(TEXT("CcdFocal")), NewConfig.focal_Length);
            GetPropValue(Config, FString(TEXT("FovHorizonal")), NewConfig.fov_Horizontal);
            GetPropValue(Config, FString(TEXT("FovVertial")), NewConfig.fov_Vertical);
            GetPropValue(Config, FString(TEXT("ResHorizonal")), NewConfig.res_Horizontal);
            GetPropValue(Config, FString(TEXT("ResVertial")), NewConfig.res_Vertical);
            GetPropValue(Config, FString(TEXT("Vignette")), NewConfig.vignette_Intensity);
            GetPropValue(Config, FString(TEXT("GrainIntensity")), NewConfig.noise_Intensity);
            GetPropValue(Config, FString(TEXT("MotionBlur")), NewConfig.motionBlur_Amount);
            GetPropValue(Config, FString(TEXT("LensFlares")), NewConfig.LensFlares);
            GetPropValue(Config, FString(TEXT("Blur")), NewConfig.blur_Intensity);
            GetPropValue(Config, FString(TEXT("Exquisite")), NewConfig.Exquisite);
            GetPropValue(Config, FString(TEXT("Bloom")), NewConfig.Bloom);
            GetPropValue(Config, FString(TEXT("Compensation")), NewConfig.Compensation);
            GetPropValue(Config, FString(TEXT("ShutterSpeed")), NewConfig.ShutterSpeed);
            GetPropValue(Config, FString(TEXT("ISO")), NewConfig.ISO);
            GetPropValue(Config, FString(TEXT("Aperture")), NewConfig.Aperture);
            GetPropValue(Config, FString(TEXT("ColorTemperature")), NewConfig.ColorTemperature);
            GetPropValue(Config, FString(TEXT("WhiteHint")), NewConfig.WhiteHint);
            GetPropValue(Config, FString(TEXT("Transmittance")), NewConfig.Transmittance);
            GetPropValue(Config, FString(TEXT("ExposureMode")), NewConfig.Exposure);
            GetPropValue(Config, FString(TEXT("ColorMode")), NewConfig.color_gray);
            GetPropValue(Config, FString(TEXT("IntrinsicType")), NewConfig.paraType);
            GetPropValue(Config, FString(TEXT("IntrinsicMat")), Intrinsic_Matrix);
            GetPropValue(Config, FString(TEXT("Distortion")), distortion_Parameters);

            if (!Intrinsic_Matrix.IsEmpty())
            {
                Intrinsic_Matrix = Intrinsic_Matrix.Replace(*FString(" "), *FString(""));    // Remove space
                FString LeftStr;
                FString RightStr;
                while (Intrinsic_Matrix.Split(",", &LeftStr, &RightStr))
                {
                    NewConfig.intrinsic_Matrix.Add(FCString::Atof(*LeftStr));
                    Intrinsic_Matrix = RightStr;
                }
                NewConfig.intrinsic_Matrix.Add(FCString::Atof(*Intrinsic_Matrix));
            }
            if (!distortion_Parameters.IsEmpty())
            {
                distortion_Parameters = distortion_Parameters.Replace(*FString(" "), *FString(""));    // Remove space
                FString LeftStr;
                FString RightStr;
                while (distortion_Parameters.Split(",", &LeftStr, &RightStr))
                {
                    NewConfig.distortion_Parameters.Add(FCString::Atof(*LeftStr));
                    distortion_Parameters = RightStr;
                }
                NewConfig.distortion_Parameters.Add(FCString::Atof(*distortion_Parameters));
            }
            FString savestring;
            GConfig->GetString(TEXT("Sensor"), TEXT("CameraSaved"), savestring, GGameIni);
            if (!BaseSavePath.IsEmpty() && savestring == TEXT("true"))
                NewConfig.savePath = BaseSavePath + TEXT("CameraData/") + TEXT("Camera_") +
                                     FString::FromInt(NewConfig.id) +
                                     TEXT("/");    // TODO: set save path in a standardized way
            SensormanagerConfig.cameraArry.Add(NewConfig);
        }
        else if (sensor.type() == sim_msg::SENSOR_TYPE_FISHEYE)
        {
            FFisheyeConfig NewConfig;
            *(FSensorConfig*) &NewConfig = Base;
            NewConfig.typeName = TEXT("Fisheye");
            NewConfig.targetId = EgoId;
            FString Intrinsic_Matrix = TEXT("");
            FString distortion_Parameters = TEXT("");

            // old
            GetPropValue(Config, FString(TEXT("Res_Horizontal")), NewConfig.res_Horizontal);
            GetPropValue(Config, FString(TEXT("Res_Vertical")), NewConfig.res_Vertical);
            GetPropValue(Config, FString(TEXT("Blur_Intensity")), NewConfig.blur_Intensity);
            GetPropValue(Config, FString(TEXT("MotionBlur_Amount")), NewConfig.motionBlur_Amount);
            GetPropValue(Config, FString(TEXT("Noise_Intensity")), NewConfig.noise_Intensity);
            GetPropValue(Config, FString(TEXT("Vignette_Intensity")), NewConfig.vignette_Intensity);
            GetPropValue(Config, FString(TEXT("Intrinsic_Matrix")), Intrinsic_Matrix);              ////---
            GetPropValue(Config, FString(TEXT("Distortion_Parameters")), distortion_Parameters);    /////----
            GetPropValue(Config, FString(TEXT("DisplayMode")), NewConfig.color_gray);
            FString dm;
            GetPropValue(Config, FString(TEXT("DisplayMode")), dm);
            if (dm == TEXT("Gray"))
            {
                NewConfig.color_gray = 1;
            }

            GetPropValue(Config, FString(TEXT("Frequency")), NewConfig.frequency);
            GetPropValue(Config, FString(TEXT("ResHorizonal")), NewConfig.res_Horizontal);
            GetPropValue(Config, FString(TEXT("ResVertial")), NewConfig.res_Vertical);
            GetPropValue(Config, FString(TEXT("Vignette")), NewConfig.vignette_Intensity);
            GetPropValue(Config, FString(TEXT("GrainIntensity")), NewConfig.noise_Intensity);
            GetPropValue(Config, FString(TEXT("MotionBlur")), NewConfig.motionBlur_Amount);
            GetPropValue(Config, FString(TEXT("LensFlares")), NewConfig.LensFlares);
            GetPropValue(Config, FString(TEXT("Blur")), NewConfig.blur_Intensity);
            GetPropValue(Config, FString(TEXT("Exquisite")), NewConfig.Exquisite);
            GetPropValue(Config, FString(TEXT("Bloom")), NewConfig.Bloom);
            GetPropValue(Config, FString(TEXT("Compensation")), NewConfig.Compensation);
            GetPropValue(Config, FString(TEXT("ShutterSpeed")), NewConfig.ShutterSpeed);
            GetPropValue(Config, FString(TEXT("ISO")), NewConfig.ISO);
            GetPropValue(Config, FString(TEXT("Aperture")), NewConfig.Aperture);
            GetPropValue(Config, FString(TEXT("ColorTemperature")), NewConfig.ColorTemperature);
            GetPropValue(Config, FString(TEXT("WhiteHint")), NewConfig.WhiteHint);
            GetPropValue(Config, FString(TEXT("Transmittance")), NewConfig.Transmittance);
            GetPropValue(Config, FString(TEXT("ExposureMode")), NewConfig.Exposure);
            GetPropValue(Config, FString(TEXT("ColorMode")), NewConfig.color_gray);
            GetPropValue(Config, FString(TEXT("IntrinsicMat")), Intrinsic_Matrix);
            GetPropValue(Config, FString(TEXT("Distortion")), distortion_Parameters);

            if (!Intrinsic_Matrix.IsEmpty())
            {
                Intrinsic_Matrix = Intrinsic_Matrix.Replace(*FString(" "), *FString(""));    // Remove space
                FString LeftStr;
                FString RightStr;
                while (Intrinsic_Matrix.Split(",", &LeftStr, &RightStr))
                {
                    NewConfig.intrinsic_Matrix.Add(FCString::Atof(*LeftStr));
                    Intrinsic_Matrix = RightStr;
                }
                NewConfig.intrinsic_Matrix.Add(FCString::Atof(*Intrinsic_Matrix));
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("SensorManger: Config LabelList is empty!"));
            }

            if (!distortion_Parameters.IsEmpty())
            {
                distortion_Parameters = distortion_Parameters.Replace(*FString(" "), *FString(""));    // Remove space
                FString LeftStr;
                FString RightStr;
                while (distortion_Parameters.Split(",", &LeftStr, &RightStr))
                {
                    if (!LeftStr.IsEmpty())
                    {
                        NewConfig.distortion_Parameters.Add(FCString::Atof(*LeftStr));
                    }
                    distortion_Parameters = RightStr;
                }
                NewConfig.distortion_Parameters.Add(FCString::Atof(*distortion_Parameters));
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("SensorManger: Config LabelList is empty!"));
            }

            // GetPropValue(Config, FString(TEXT("SaveData")), NewCameraConfig.bSaveData);
            // Save Path
            FString savestring;
            GConfig->GetString(TEXT("Sensor"), TEXT("FisheyeSaved"), savestring, GGameIni);
            if (!BaseSavePath.IsEmpty() && savestring == TEXT("true"))
                NewConfig.savePath = BaseSavePath + TEXT("FisheyeData/") + TEXT("Fisheye_") +
                                     FString::FromInt(NewConfig.id) +
                                     TEXT("/");    // TODO: set save path in a standardized way
            SensormanagerConfig.fisheyeArry.Add(NewConfig);
        }
        else if (sensor.type() == sim_msg::SENSOR_TYPE_SEMANTIC)
        {
            FCameraConfig NewConfig;
            *(FSensorConfig*) &NewConfig = Base;
            NewConfig.typeName = TEXT("Semantic");
            NewConfig.targetId = EgoId;
            FString Intrinsic_Matrix = TEXT("");
            FString distortion_Parameters = TEXT("");

            // old
            GetPropValue(Config, FString(TEXT("CCD_Width")), NewConfig.ccd_Width);              ////---
            GetPropValue(Config, FString(TEXT("CCD_Height")), NewConfig.ccd_Height);            ////---
            GetPropValue(Config, FString(TEXT("Focal_Length")), NewConfig.focal_Length);        ////---
            GetPropValue(Config, FString(TEXT("FOV_Horizontal")), NewConfig.fov_Horizontal);    ////---
            GetPropValue(Config, FString(TEXT("FOV_Vertical")), NewConfig.fov_Vertical);        ////---
            GetPropValue(Config, FString(TEXT("Res_Horizontal")), NewConfig.res_Horizontal);    ////---
            GetPropValue(Config, FString(TEXT("Res_Vertical")), NewConfig.res_Vertical);        ////---
            GetPropValue(Config, FString(TEXT("IntrinsicParamType")), NewConfig.paraType);
            GetPropValue(Config, FString(TEXT("Intrinsic_Matrix")), Intrinsic_Matrix);              ////---
            GetPropValue(Config, FString(TEXT("Distortion_Parameters")), distortion_Parameters);    /////----

            GetPropValue(Config, FString(TEXT("Frequency")), NewConfig.frequency);
            GetPropValue(Config, FString(TEXT("CcdWidth")), NewConfig.ccd_Width);
            GetPropValue(Config, FString(TEXT("CcdHeight")), NewConfig.ccd_Height);
            GetPropValue(Config, FString(TEXT("CcdFocal")), NewConfig.focal_Length);
            GetPropValue(Config, FString(TEXT("FovHorizonal")), NewConfig.fov_Horizontal);
            GetPropValue(Config, FString(TEXT("FovVertial")), NewConfig.fov_Vertical);
            GetPropValue(Config, FString(TEXT("ResHorizonal")), NewConfig.res_Horizontal);
            GetPropValue(Config, FString(TEXT("ResVertial")), NewConfig.res_Vertical);
            GetPropValue(Config, FString(TEXT("IntrinsicType")), NewConfig.paraType);
            GetPropValue(Config, FString(TEXT("IntrinsicMat")), Intrinsic_Matrix);
            GetPropValue(Config, FString(TEXT("Distortion")), distortion_Parameters);

            if (!Intrinsic_Matrix.IsEmpty())
            {
                Intrinsic_Matrix = Intrinsic_Matrix.Replace(*FString(" "), *FString(""));    // Remove space
                FString LeftStr;
                FString RightStr;
                while (Intrinsic_Matrix.Split(",", &LeftStr, &RightStr))
                {
                    NewConfig.intrinsic_Matrix.Add(FCString::Atof(*LeftStr));
                    Intrinsic_Matrix = RightStr;
                }
                NewConfig.intrinsic_Matrix.Add(FCString::Atof(*Intrinsic_Matrix));
            }
            if (!distortion_Parameters.IsEmpty())
            {
                distortion_Parameters = distortion_Parameters.Replace(*FString(" "), *FString(""));    // Remove space
                FString LeftStr;
                FString RightStr;
                while (distortion_Parameters.Split(",", &LeftStr, &RightStr))
                {
                    NewConfig.distortion_Parameters.Add(FCString::Atof(*LeftStr));
                    distortion_Parameters = RightStr;
                }
                NewConfig.distortion_Parameters.Add(FCString::Atof(*distortion_Parameters));
            }

            FString savestring;
            GConfig->GetString(TEXT("Sensor"), TEXT("SemanticSaved"), savestring, GGameIni);
            if (!BaseSavePath.IsEmpty() && savestring == TEXT("true"))
                NewConfig.savePath = BaseSavePath + TEXT("SemanticData/") + TEXT("Semantic_") +
                                     FString::FromInt(NewConfig.id) +
                                     TEXT("/");    // TODO: set save path in a standardized way
            SensormanagerConfig.semanticArry.Add(NewConfig);
        }
        else if (sensor.type() == sim_msg::SENSOR_TYPE_DEPTH)
        {
            FCameraConfig NewConfig;
            *(FSensorConfig*) &NewConfig = Base;
            NewConfig.typeName = TEXT("Depth");
            NewConfig.targetId = EgoId;
            FString Intrinsic_Matrix = TEXT("");
            FString distortion_Parameters = TEXT("");

            // old
            GetPropValue(Config, FString(TEXT("CCD_Width")), NewConfig.ccd_Width);              ////---
            GetPropValue(Config, FString(TEXT("CCD_Height")), NewConfig.ccd_Height);            ////---
            GetPropValue(Config, FString(TEXT("Focal_Length")), NewConfig.focal_Length);        ////---
            GetPropValue(Config, FString(TEXT("FOV_Horizontal")), NewConfig.fov_Horizontal);    ////---
            GetPropValue(Config, FString(TEXT("FOV_Vertical")), NewConfig.fov_Vertical);        ////---
            GetPropValue(Config, FString(TEXT("Res_Horizontal")), NewConfig.res_Horizontal);    ////---
            GetPropValue(Config, FString(TEXT("Res_Vertical")), NewConfig.res_Vertical);        ////---
            GetPropValue(Config, FString(TEXT("IntrinsicParamType")), NewConfig.paraType);
            GetPropValue(Config, FString(TEXT("Intrinsic_Matrix")), Intrinsic_Matrix);              ////---
            GetPropValue(Config, FString(TEXT("Distortion_Parameters")), distortion_Parameters);    /////----

            GetPropValue(Config, FString(TEXT("Frequency")), NewConfig.frequency);
            GetPropValue(Config, FString(TEXT("CcdWidth")), NewConfig.ccd_Width);
            GetPropValue(Config, FString(TEXT("CcdHeight")), NewConfig.ccd_Height);
            GetPropValue(Config, FString(TEXT("CcdFocal")), NewConfig.focal_Length);
            GetPropValue(Config, FString(TEXT("FovHorizonal")), NewConfig.fov_Horizontal);
            GetPropValue(Config, FString(TEXT("FovVertial")), NewConfig.fov_Vertical);
            GetPropValue(Config, FString(TEXT("ResHorizonal")), NewConfig.res_Horizontal);
            GetPropValue(Config, FString(TEXT("ResVertial")), NewConfig.res_Vertical);
            GetPropValue(Config, FString(TEXT("IntrinsicType")), NewConfig.paraType);
            GetPropValue(Config, FString(TEXT("IntrinsicMat")), Intrinsic_Matrix);
            GetPropValue(Config, FString(TEXT("Distortion")), distortion_Parameters);

            if (!Intrinsic_Matrix.IsEmpty())
            {
                Intrinsic_Matrix = Intrinsic_Matrix.Replace(*FString(" "), *FString(""));    // Remove space
                FString LeftStr;
                FString RightStr;
                while (Intrinsic_Matrix.Split(",", &LeftStr, &RightStr))
                {
                    NewConfig.intrinsic_Matrix.Add(FCString::Atof(*LeftStr));
                    Intrinsic_Matrix = RightStr;
                }
                NewConfig.intrinsic_Matrix.Add(FCString::Atof(*Intrinsic_Matrix));
            }
            if (!distortion_Parameters.IsEmpty())
            {
                distortion_Parameters = distortion_Parameters.Replace(*FString(" "), *FString(""));    // Remove space
                FString LeftStr;
                FString RightStr;
                while (distortion_Parameters.Split(",", &LeftStr, &RightStr))
                {
                    NewConfig.distortion_Parameters.Add(FCString::Atof(*LeftStr));
                    distortion_Parameters = RightStr;
                }
                NewConfig.distortion_Parameters.Add(FCString::Atof(*distortion_Parameters));
            }

            FString savestring;
            GConfig->GetString(TEXT("Sensor"), TEXT("DepthSaved"), savestring, GGameIni);
            if (!BaseSavePath.IsEmpty() && savestring == TEXT("true"))
                NewConfig.savePath = BaseSavePath + TEXT("DepthData/") + TEXT("Depth_") +
                                     FString::FromInt(NewConfig.id) +
                                     TEXT("/");    // TODO: set save path in a standardized way
            SensormanagerConfig.depthArry.Add(NewConfig);
        }
        else if (sensor.type() == sim_msg::SENSOR_TYPE_TRADITIONAL_LIDAR)
        {
            FLidarConfig NewConfig;
            *(FSensorConfig*) &NewConfig = Base;
            NewConfig.typeName = TEXT("Lidar");
            NewConfig.targetId = EgoId;

            // old
            GetPropValue(Config, FString(TEXT("Frequency")), NewConfig.frequency);
            GetPropValue(Config, FString(TEXT("Model")), NewConfig.model);
            GetPropValue(Config, FString(TEXT("uChannels")), NewConfig.channels);
            GetPropValue(Config, FString(TEXT("uRange")), NewConfig.range);
            GetPropValue(Config, FString(TEXT("uHorizontalResolution")), NewConfig.horizontalResolution);
            GetPropValue(Config, FString(TEXT("uUpperFov")), NewConfig.upperFovLimit);
            GetPropValue(Config, FString(TEXT("uLowerFov")), NewConfig.lowerFovLimit);
            GetPropValue(Config, FString(TEXT("DrawPoint")), NewConfig.bDrawPoint);

            // new
            GetPropValue(Config, FString(TEXT("Type")), NewConfig.model);
            GetPropValue(Config, FString(TEXT("IP")), NewConfig.ip);
            GetPropValue(Config, FString(TEXT("Port")), NewConfig.port);
            GetPropValue(Config, FString(TEXT("Attenuation")), NewConfig.Attenuation);
            GetPropValue(Config, FString(TEXT("AngleDefinition")), NewConfig.AngleDefinition);
            GetPropValue(Config, FString(TEXT("ExtinctionCoe")), NewConfig.ExtinctionCoe);
            GetPropValue(Config, FString(TEXT("RayNum")), NewConfig.channels);
            GetPropValue(Config, FString(TEXT("Radius")), NewConfig.range);
            GetPropValue(Config, FString(TEXT("ReflectionType")), NewConfig.ReflectionType);
            GetPropValue(Config, FString(TEXT("HorzionalRes")), NewConfig.horizontalResolution);
            GetPropValue(Config, FString(TEXT("FovUp")), NewConfig.upperFovLimit);
            GetPropValue(Config, FString(TEXT("FovDown")), NewConfig.lowerFovLimit);
            GetPropValue(Config, FString(TEXT("FovStart")), NewConfig.FovStart);
            GetPropValue(Config, FString(TEXT("FovEnd")), NewConfig.FovEnd);

            NewConfig.cfgDir = FPaths::ProjectUserDir() + TEXT("XMLFiles/LidarConfig");
            if (!FPaths::DirectoryExists(NewConfig.cfgDir))
            {
                NewConfig.cfgDir = FPaths::ProjectDir() + TEXT("XMLFiles/LidarConfig");
            }
            FString savestring;
            GConfig->GetString(TEXT("Sensor"), TEXT("LidarSaved"), savestring, GGameIni);
            if (!BaseSavePath.IsEmpty() && savestring == TEXT("true"))
                NewConfig.savePath =
                    BaseSavePath + TEXT("LidarhData/") + TEXT("Lidar_") + FString::FromInt(NewConfig.id) + TEXT("/");
            SensormanagerConfig.lidarArry.Add(NewConfig);
        }
        else if (sensor.type() == sim_msg::SENSOR_TYPE_ULTRASONIC)
        {
            FUltrasonicConfig NewConfig;
            *(FSensorConfig*) &NewConfig = Base;
            NewConfig.typeName = TEXT("Ultrasonic");
            NewConfig.targetId = EgoId;
            FString attachtype;

            GetPropValue(Config, FString(TEXT("FOV_Horizontal")), NewConfig.azimuth_fov);
            GetPropValue(Config, FString(TEXT("FOV_Vertical")), NewConfig.elevation_fov);
            GetPropValue(Config, FString(TEXT("Frequency")), NewConfig.fc);
            GetPropValue(Config, FString(TEXT("dBmin")), NewConfig.dB_min);
            GetPropValue(Config, FString(TEXT("Radius")), NewConfig.radius);
            GetPropValue(Config, FString(TEXT("RayDivisionHor")), NewConfig.ray_division_hor);
            GetPropValue(Config, FString(TEXT("RayDivisionVer")), NewConfig.ray_division_ele);
            GetPropValue(Config, FString(TEXT("Distance")), NewConfig.scan_distance);
            GetPropValue(Config, FString(TEXT("NoiseFactor")), NewConfig.noise_factor);
            GetPropValue(Config, FString(TEXT("NoiseStd")), NewConfig.noise_std);
            GetPropValue(Config, FString(TEXT("AttachmentType")), attachtype);
            GetPropValue(Config, FString(TEXT("AttachmentRange")), NewConfig.attachment_range);
            GetPropValue(Config, FString(TEXT("DebugFrustum")), NewConfig.debug_frustum);
            GetPropValue(Config, FString(TEXT("DebugReturn")), NewConfig.debug_rescan);
            GetPropValue(Config, FString(TEXT("DebugScan")), NewConfig.debug_scan);

            // new
            GetPropValue(Config, FString(TEXT("FovHorizional")), NewConfig.azimuth_fov);
            GetPropValue(Config, FString(TEXT("FovVertial")), NewConfig.elevation_fov);
            GetPropValue(Config, FString(TEXT("IndirectDistance")), NewConfig.openIndirectDistance);
            GetPropValue(Config, FString(TEXT("PulseMoment")), NewConfig.timePulse0);
            GetPropValue(Config, FString(TEXT("PulsePeriod")), NewConfig.timePulsePeriod);
            GetPropValue(Config, FString(TEXT("PollTurn")), NewConfig.pollTurn);

            NewConfig.attachment_type = EAttachmentTypeEnum::AT_None;
            if (attachtype == TEXT("Water"))
            {
                NewConfig.attachment_type = EAttachmentTypeEnum::AT_Water;
            }
            else if (attachtype == TEXT("Mud"))
            {
                NewConfig.attachment_type = EAttachmentTypeEnum::AT_Mud;
            }

            NewConfig.cfgDir = FPaths::ProjectUserDir() + TEXT("XMLFiles/UssConfig");
            if (!FPaths::DirectoryExists(NewConfig.cfgDir))
            {
                NewConfig.cfgDir = FPaths::ProjectDir() + TEXT("XMLFiles/UssConfig");
            }
            FString savestring;
            GConfig->GetString(TEXT("Sensor"), TEXT("UltrasonicSaved"), savestring, GGameIni);
            if (!BaseSavePath.IsEmpty() && savestring == TEXT("true"))
            {
                NewConfig.savePath = BaseSavePath + TEXT("UltrasonicData/") + TEXT("Ultrasonic_") +
                                     FString::FromInt(NewConfig.id) + TEXT("/");
                NewConfig.bSaveData = true;
            }
            NewConfig.all_ultrasonicArry = all_ultrasonicArry;
            SensormanagerConfig.ultrasonicArry.Add(NewConfig);
        }
    }

    return SensormanagerConfig;
}
void ASensorManager::CoordinateTransform_RightHandToLeftHand(FVector& _Location, FRotator& _Rotation)
{
    _Location.Y = _Location.Y * (-1.f);
    _Rotation.Pitch = _Rotation.Pitch * (-1.f);
    _Rotation.Yaw = _Rotation.Yaw * (-1.f);
}

void ASensorManager::Init(const FManagerConfig& Config)
{
    const FSensorManagerConfig* SensorConfig = Cast_Data<const FSensorManagerConfig>(Config);
    if (!SensorConfig)
    {
        UE_LOG(SimLogSensorManager, Error, TEXT("Can Not Get SensorConfig!"));
        return;
    }

#ifdef WIN32

#else
    if (rosNode == nullptr)
    {
        // FActorSpawnParameters spawnParams;
        // spawnParams.Name = FName(TEXT("RosSensor"));
        // rosNode = GetWorld()->SpawnActor<AROS2Node>(spawnParams);
        // rosNode->Namespace = TEXT("DisplaySensor");
        // rosNode->Init();
    }

#endif

    // Lidar
    for (auto& Elem : SensorConfig->lidarArry)
    {
        ASensorActor* NewSensor =
            ASensorFactory::SpawnSensor<ATLidarSensor>(GetWorld(), ATLidarSensor::StaticClass(), Elem);

        if (NewSensor)
        {
            ISimActorInterface* InstalledSimActor = NewSensor->Install(Elem);
            if (InstalledSimActor)
            {
                TMap<FString, ISensorInterface*>* ExistSenorMap = sensorMap.Find(InstalledSimActor);
                if (ExistSenorMap)
                {
                    FString SensorName;
                    SensorName = Elem.typeName;
                    SensorName += FString("_") + FString::FromInt(Elem.id);
                    if (!ExistSenorMap->Find(SensorName))
                    {
                        ExistSenorMap->Add(SensorName, NewSensor);
                    }
                    else
                    {
                        NewSensor->Destroy(TEXT("Already Exist!"));
                    }
                }
                else
                {
                    TMap<FString, ISensorInterface*> NewSenorMap;
                    FString SensorName;
                    SensorName = Elem.typeName;
                    SensorName += FString("_") + FString::FromInt(Elem.id);
                    NewSenorMap.Add(SensorName, NewSensor);
                    sensorMap.Add(InstalledSimActor, NewSenorMap);
                }
            }
            else
            {
                NewSensor->Destroy(TEXT("Cant Find Target!"));
            }
        }
    }
    // Camera
    for (auto& Elem : SensorConfig->cameraArry)
    {
        ACameraSensor* CameraSensor =
            ASensorFactory::SpawnSensor<ACameraSensor>(GetWorld(), ACameraSensor::StaticClass(), Elem);
        if (CameraSensor)
        {
            ISimActorInterface* InstalledSimActor = CameraSensor->Install(Elem);
            if (InstalledSimActor)
            {
                TMap<FString, ISensorInterface*>* ExistSenorMap = sensorMap.Find(InstalledSimActor);
                if (ExistSenorMap)
                {
                    FString SensorName;
                    SensorName = Elem.typeName;
                    SensorName += FString("_") + FString::FromInt(Elem.id);
                    if (!ExistSenorMap->Find(SensorName))
                    {
                        ExistSenorMap->Add(SensorName, CameraSensor);
                    }
                    else
                    {
                        CameraSensor->Destroy(TEXT("Already Exist!"));
                    }
                }
                else
                {
                    TMap<FString, ISensorInterface*> NewSenorMap;
                    FString SensorName;
                    SensorName = Elem.typeName;
                    SensorName += FString("_") + FString::FromInt(Elem.id);
                    NewSenorMap.Add(SensorName, CameraSensor);
                    sensorMap.Add(InstalledSimActor, NewSenorMap);
                }
            }
            else    // road sensor
            {
                TMap<FString, ISensorInterface*>* ExistSenorMap = sensorMap.Find(0);
                if (ExistSenorMap)
                {
                    FString SensorName;
                    SensorName = Elem.typeName;
                    SensorName += FString("_") + FString::FromInt(Elem.id);
                    if (!ExistSenorMap->Find(SensorName))
                    {
                        ExistSenorMap->Add(SensorName, CameraSensor);
                    }
                    else
                    {
                        CameraSensor->Destroy(TEXT("Already Exist!"));
                    }
                }
                else
                {
                    TMap<FString, ISensorInterface*> NewSenorMap;
                    FString SensorName;
                    SensorName = Elem.typeName;
                    SensorName += FString("_") + FString::FromInt(Elem.id);
                    NewSenorMap.Add(SensorName, CameraSensor);
                    sensorMap.Add(0, NewSenorMap);
                }
            }
        }
    }

    // senmantic
    for (auto& Elem : SensorConfig->semanticArry)
    {
        ASemanticCamera* SemanticCameraSensor =
            ASensorFactory::SpawnSensor<ASemanticCamera>(GetWorld(), ASemanticCamera::StaticClass(), Elem);
        if (SemanticCameraSensor)
        {
            ISimActorInterface* InstalledSimActor = SemanticCameraSensor->Install(Elem);
            if (InstalledSimActor)
            {
                TMap<FString, ISensorInterface*>* ExistSenorMap = sensorMap.Find(InstalledSimActor);
                if (ExistSenorMap)
                {
                    FString SensorName;
                    SensorName = Elem.typeName;
                    SensorName += FString("_") + FString::FromInt(Elem.id);
                    if (!ExistSenorMap->Find(SensorName))
                    {
                        ExistSenorMap->Add(SensorName, SemanticCameraSensor);
                    }
                    else
                    {
                        SemanticCameraSensor->Destroy(TEXT("Already Exist!"));
                    }
                }
                else
                {
                    TMap<FString, ISensorInterface*> NewSenorMap;
                    FString SensorName;
                    SensorName = Elem.typeName;
                    SensorName += FString("_") + FString::FromInt(Elem.id);
                    NewSenorMap.Add(SensorName, SemanticCameraSensor);
                    sensorMap.Add(InstalledSimActor, NewSenorMap);
                }
            }
            else
            {
                SemanticCameraSensor->Destroy(TEXT("Cant Find Target!"));
            }
        }
    }

    // depth
    for (auto& Elem : SensorConfig->depthArry)
    {
        ADepthSensor* CameraSensor =
            ASensorFactory::SpawnSensor<ADepthSensor>(GetWorld(), ADepthSensor::StaticClass(), Elem);
        if (CameraSensor)
        {
            ISimActorInterface* InstalledSimActor = CameraSensor->Install(Elem);
            if (InstalledSimActor)
            {
                TMap<FString, ISensorInterface*>* ExistSenorMap = sensorMap.Find(InstalledSimActor);
                if (ExistSenorMap)
                {
                    FString SensorName;
                    SensorName = Elem.typeName;
                    SensorName += FString("_") + FString::FromInt(Elem.id);
                    if (!ExistSenorMap->Find(SensorName))
                    {
                        ExistSenorMap->Add(SensorName, CameraSensor);
                    }
                    else
                    {
                        CameraSensor->Destroy(TEXT("Already Exist!"));
                    }
                }
                else
                {
                    TMap<FString, ISensorInterface*> NewSenorMap;
                    FString SensorName;
                    SensorName = Elem.typeName;
                    SensorName += FString("_") + FString::FromInt(Elem.id);
                    NewSenorMap.Add(SensorName, CameraSensor);
                    sensorMap.Add(InstalledSimActor, NewSenorMap);
                }
            }
            else
            {
                CameraSensor->Destroy(TEXT("Cant Find Target!"));
            }
        }
    }

    // Fisheye
    for (auto& Elem : SensorConfig->fisheyeArry)
    {
        AFisheyeSensor* FisheyeSensor =
            ASensorFactory::SpawnSensor<AFisheyeSensor>(GetWorld(), AFisheyeSensor::StaticClass(), Elem);
        if (FisheyeSensor)
        {
            ISimActorInterface* InstalledSimActor = FisheyeSensor->Install(Elem);
            if (InstalledSimActor)
            {
                TMap<FString, ISensorInterface*>* ExistSenorMap = sensorMap.Find(InstalledSimActor);
                if (ExistSenorMap)
                {
                    FString SensorName;
                    SensorName = Elem.typeName;
                    SensorName += FString("_") + FString::FromInt(Elem.id);
                    if (!ExistSenorMap->Find(SensorName))
                    {
                        ExistSenorMap->Add(SensorName, FisheyeSensor);
                    }
                    else
                    {
                        FisheyeSensor->Destroy(TEXT("Already Exist!"));
                    }
                }
                else
                {
                    TMap<FString, ISensorInterface*> NewSenorMap;
                    FString SensorName;
                    SensorName = Elem.typeName;
                    SensorName += FString("_") + FString::FromInt(Elem.id);
                    NewSenorMap.Add(SensorName, FisheyeSensor);
                    sensorMap.Add(InstalledSimActor, NewSenorMap);
                }
            }
            else
            {
                FisheyeSensor->Destroy(TEXT("Cant Find Target!"));
            }
        }
    }

    ////ViewTruth
    // for (auto &Elem : SensorConfig->viewTruthArry)
    //{
    //     ASensorActor* NewSensor = ASensorFactory::SpawnSensor<AViewTruth>(GetWorld(), AViewTruth::StaticClass(),
    //     Elem); if (NewSensor)
    //     {
    //         ISimActorInterface* InstalledSimActor = NewSensor->Install(Elem);
    //         if (InstalledSimActor)
    //         {
    //             TMap<FString, ISensorInterface*>* ExistSenorMap = sensorMap.Find(InstalledSimActor);
    //             if (ExistSenorMap)
    //             {
    //                 FString SensorName;
    //                 SensorName = Elem.typeName;
    //                 SensorName += FString("_") + FString::FromInt(Elem.id);
    //                 if (!ExistSenorMap->Find(SensorName))
    //                 {
    //                     ExistSenorMap->Add(SensorName, NewSensor);
    //                 }
    //                 else
    //                 {
    //                     NewSensor->Destroy(TEXT("Already Exist!"));
    //                 }
    //             }
    //             else
    //             {
    //                 TMap<FString, ISensorInterface*> NewSenorMap;
    //                 FString SensorName;
    //                 SensorName = Elem.typeName;
    //                 SensorName += FString("_") + FString::FromInt(Elem.id);
    //                 NewSenorMap.Add(SensorName, NewSensor);
    //                 sensorMap.Add(InstalledSimActor, NewSenorMap);
    //             }
    //         }
    //         else
    //         {
    //             NewSensor->Destroy(TEXT("Cant Find Target!"));
    //         }
    //     }
    // }

    // Ultrasonic
    for (auto& Elem : SensorConfig->ultrasonicArry)
    {
        AUltrasonicSensor* sensor =
            ASensorFactory::SpawnSensor<AUltrasonicSensor>(GetWorld(), AUltrasonicSensor::StaticClass(), Elem);
        if (sensor)
        {
            ISimActorInterface* InstalledSimActor = sensor->Install(Elem);
            if (InstalledSimActor)
            {
                TMap<FString, ISensorInterface*>* ExistSenorMap = sensorMap.Find(InstalledSimActor);
                if (ExistSenorMap)
                {
                    FString SensorName;
                    SensorName = Elem.typeName;
                    SensorName += FString("_") + FString::FromInt(Elem.id);
                    if (!ExistSenorMap->Find(SensorName))
                    {
                        ExistSenorMap->Add(SensorName, sensor);
                    }
                    else
                    {
                        sensor->Destroy(TEXT("Already Exist!"));
                    }
                }
                else
                {
                    TMap<FString, ISensorInterface*> NewSenorMap;
                    FString SensorName;
                    SensorName = Elem.typeName;
                    SensorName += FString("_") + FString::FromInt(Elem.id);
                    NewSenorMap.Add(SensorName, sensor);
                    sensorMap.Add(InstalledSimActor, NewSenorMap);
                }
            }
            else
            {
                sensor->Destroy(TEXT("Cant Find Target!"));
            }
        }
    }
}

void updateinstance(ISensorInterface* ls, double stamp)
{
    FSensorInput SensorIn;
    FSensorOutput SensorOut;

    SensorIn.timeStamp = stamp;
    ls->Update(SensorIn, SensorOut);
}
void ASensorManager::Update(const FManagerIn& _Input, FManagerOut& _Output)
{
    // UE_LOG(LogTemp, Log, TEXT("SensorManger: update"));
    const FSensorManagerIn* SensorManagerIn = Cast_Data<const FSensorManagerIn>(_Input);
    FSensorManagerOut* SensorManagerOut = Cast_Data<FSensorManagerOut>(_Output);
    SensorManagerOut->outArray.SetNum(0);
    for (auto& Elem : sensorMap)
    {
        // if (Elem.Key)
        //{
        //     //std::vector<std::thread> thrs;
        //     //for (auto &Sensor : Elem.Value)
        //     //{
        //     //    thrs.push_back(std::thread(updateinstance, Sensor.Value, SensorManagerIn->timeStamp));
        //     //    //thrs.push_back(std::move(t1));
        //     //}
        //     //for (std::thread & th : thrs)
        //     //{
        //     //    // If thread Object is Joinable then Join that thread.
        //     //    if (th.joinable())
        //     //        th.join();
        //     //}
        // }
        for (auto& Sensor : Elem.Value)
        {
            FSensorInput SensorIn;
            FSensorOutput SensorOut;
            SensorIn.timeStamp = SensorManagerIn->timeStamp;
            SensorIn.timeStamp_ego = SensorManagerIn->timeStamp_ego;
            SensorIn.timeStamp_tail = SensorManagerIn->timeStamp_tail;
            SensorOut.id = Sensor.Value->configBase.id;
            SensorOut.type = Sensor.Value->configBase.typeName;
            SensorOut.timeStamp = SensorIn.timeStamp;
            Sensor.Value->Update(SensorIn, SensorOut);
            SensorManagerOut->outArray.Add(SensorOut);
        }
    }
}
