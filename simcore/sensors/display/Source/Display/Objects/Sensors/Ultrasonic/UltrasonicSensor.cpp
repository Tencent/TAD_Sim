// Fill out your copyright notice in the Description page of Project Settings.

#include "UltrasonicSensor.h"
#include <thread>
#include <fstream>
#include <sstream>
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Framework/SaveDataThread.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Managers/EnvManager.h"
#include "Framework/DisplayPlayerController.h"
#include "FovDepthBuffer.h"
#include "Framework/DisplayGameInstance.h"
#include "Framework/SaveDataThread.h"
#include "Framework/DisplayGameStateBase.h"
#include "BoostMath.h"
#include <random>
#include "SimMsg/osi_datarecording.pb.h"
#include <google/protobuf/util/json_util.h>

AUltrasonicSensor::AUltrasonicSensor(const FObjectInitializer& ObjectInitializer)
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = false;
    root = CreateDefaultSubobject<USceneComponent>(FName(TEXT("root")));
    RootComponent = root;
}

void AUltrasonicSensor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // 保存所有的数据
    if (config.bSaveData)
        Save();
    Super::EndPlay(EndPlayReason);
}

// 间接距离计算
float AUltrasonicSensor::CalIndirectDistance(const FDetectLine& domain, double timestamp)
{
    if (!config.openIndirectDistance)
    {
        return config.scan_distance;
    }
    timestamp -= config.timePulse0;
    const float speed = sonic_v * 0.001f;    // voice speed, m/ms
    double t0 = FMath::Floor(timestamp / config.timePulsePeriod) * config.timePulsePeriod;
    double td = domain.length * 2 / speed;
    FVector point = domain.length * 100 * domain.lineNor.Vector();
    double mindis = config.scan_distance;
    // 计算所有的超声波传播路径，并计算最短距离
    for (const auto& cfg : config.all_ultrasonicArry)
    {
        if (!cfg.openIndirectDistance)
        {
            continue;
        }
        if (rs10[cfg.id] == 0 || rs10[cfg.id] == 3)
        {
            continue;
        }
        FTransform rt1;
        rt1.SetLocation(cfg.installLocation);
        rt1.SetRotation(cfg.installRotation.Quaternion());
        FVector pp = rt1.TransformPosition(point);
        float dis = pp.Size();
        if (dis > cfg.scan_distance * 100)
        {
            continue;
        }
        double h = FMath::RadiansToDegrees(FMath::Abs(2.0 * FMath::Atan2(pp.Y, pp.X)));
        double v = FMath::RadiansToDegrees(FMath::Abs(2.0 * FMath::Atan2(pp.Z, pp.X)));
        if (h < cfg.azimuth_fov && v < cfg.elevation_fov)
        {
            float td2 = (dis * 0.01 + domain.length) / speed;
            double t2 = FMath::CeilToDouble((t0 - cfg.timePulse0 - td2) / cfg.timePulsePeriod) * cfg.timePulsePeriod +
                        cfg.timePulse0 + td2;
            t2 -= t0 + 0.1;
            if (t2 > 0 && t2 < td)
            {
                mindis = FMath::Min(mindis, speed * t2 * 0.5);
            }
        }
    }
    return mindis;
}

bool AUltrasonicSensor::Init(const FSensorConfig& _Config)
{
    // 初始化传感器
    if (!ASensorActor::Init(_Config))
        return false;
    // 获取配置参数
    config = *Cast_Sim<const FUltrasonicConfig>(_Config);
    // 加载雷达纹理映射
    LoadStencilMap(config.cfgDir);
    // 加载CEO数据
    loadCeoData(config.cfgDir);
    // 计算方向图
    int hn = FMath::Max(3, FMath::CeilToInt(config.azimuth_fov / FMath::Max(config.ray_division_hor, 0.0001f)));
    int en = FMath::Max(3, FMath::CeilToInt(config.elevation_fov / FMath::Max(config.ray_division_ele, 0.0001f)));
    float step_y = config.azimuth_fov / hn;
    float step_z = FMath::Min(config.elevation_fov, config.azimuth_fov) / en;
    // 设置声学参数
    float lambda = sonic_v / config.fc;
    float sonic_k = PI * 2 / lambda;
    float sonic_ka = sonic_k * config.radius;

    float fud = FMath::Pow(10, -FMath::Abs(config.dB_min) * 0.1f);
    float beamwidth = FMath::DegreesToRadians(config.azimuth_fov) * 0.5;
    for (float i = 0.02; i < HALF_PI; i += 0.02)
    {
        float kasita = sonic_ka * FMath::Sin(i);
        float p = ueboost::bessel_j(1, kasita) * 2 / kasita;
        if (p < fud)
        {
            beamwidth = i - 0.01;
            break;
        }
    }
    beamwidth /= FMath::DegreesToRadians(config.azimuth_fov) * 0.5;
    beamwidth = FMath::Min(beamwidth, 1.f);

    float distance = config.scan_distance;
    if (config.attachment_type == EAttachmentTypeEnum::AT_Water)
    {
        distance *= 1.f - FMath::Pow(FMath::Clamp(config.attachment_range, 0.f, 1.f), 3) * 0.1;
    }
    else if (config.attachment_type == EAttachmentTypeEnum::AT_Mud)
    {
        distance *= 1.f - FMath::Pow(FMath::Clamp(config.attachment_range, 0.f, 1.f), 2) * 0.5;
    }
    // 获取深度测试方法
    FString method = "depth";
    GConfig->GetString(TEXT("Sensor"), TEXT("UltrasonicMethod"), method, GGameIni);
    method = method.ToLower();
    if (method == TEXT("raycast"))
    {
        UE_LOG(LogTemp, Log, TEXT("Ultrasonic method is Raycast"));
    }
    else if (method == TEXT("depth"))
    {
        UE_LOG(LogTemp, Log, TEXT("Ultrasonic method is Depth"));
        // 创建深度相机
        FovViewConfiguration camCfg;
        camCfg.position = FVector(0);
        camCfg.rotator = FRotator(0, 0, 0);
        camCfg.hfov = FMath::Max(25.f, FMath::Min(150.f, 1.1f * config.azimuth_fov));
        camCfg.vfov = FMath::Max(25.f, FMath::Min(150.f, 1.1f * config.elevation_fov));
        camCfg.w = FMath::Max(hn * 4, 50);
        camCfg.h = FMath::Max(en * 4, 50);

        UE_LOG(LogTemp, Log, TEXT("depth image w=%d, h=%d"), camCfg.w, camCfg.h);
        depthBuffer = GetWorld()->SpawnActor<AFovDepthBuffer>();
        depthBuffer->SetOwner(this);
        depthBuffer->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
        if (!depthBuffer->SetCamera(camCfg))
        {
            UE_LOG(LogTemp, Log, TEXT("Ultrasonic cannot install depth camera."));
            return false;
        }
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Ultrasonic method is unknow"));
        return false;
    }

    // 生成检测线
    for (int i = 0; i <= en; i++)
    {
        float pitch = -config.elevation_fov * 0.5 + i * step_z;
        for (int j = 0; j <= hn; j++)
        {
            float yaw = -config.azimuth_fov * 0.5 + j * step_y;
            FRotator r(pitch, yaw, 0);

            FDetectLine dl;
            dl.lineNor = r;
            float ang = FMath::Acos(r.Vector().X) * beamwidth;

            float kasita = sonic_ka * FMath::Sin(ang);
            if (FMath::Abs(kasita) < 1e-6)
                dl.length = distance;
            else
            {
                float p = ueboost::bessel_j(1, kasita) * 2 / kasita;
                dl.length = distance * p;
            }

            // UE_LOG(LogTemp, Warning, TEXT("dl length: %f"), dl.length);

            detectRays.Add(dl);
        }
    }
    // 设置范围点
    rangePt[0] = FRotator(-config.elevation_fov * 0.5, -config.azimuth_fov * 0.5, 0);
    rangePt[1] = FRotator(config.elevation_fov * 0.5, -config.azimuth_fov * 0.5, 0);
    rangePt[2] = FRotator(config.elevation_fov * 0.5, config.azimuth_fov * 0.5, 0);
    rangePt[3] = FRotator(-config.elevation_fov * 0.5, config.azimuth_fov * 0.5, 0);
    // 如果开启间接距离测距功能
    if (config.openIndirectDistance)
    {
        config.pollTurn.ToLowerInline();
        config.timePulsePeriod = FMath::Max(config.timePulsePeriod, 0.001f);
        TArray<FUltrasonicConfig> other_ultrasonicArry;
        FTransform rt0;
        rt0.SetLocation(config.installLocation);
        rt0.SetRotation(config.installRotation.Quaternion());
        for (const auto& cfg : config.all_ultrasonicArry)
        {
            if (cfg.id == config.id)
            {
                continue;
            }
            /// remove cannot crossed
            bool isin = false;
            FTransform rt1;
            rt1.SetLocation(cfg.installLocation);
            rt1.SetRotation(cfg.installRotation.Quaternion());
            rt1 = rt1.Inverse() * rt0;
            for (const auto& ray : detectRays)
            {
                for (int i = 1, is = FMath::CeilToInt(ray.length / 0.3f); i <= is; i++)
                {
                    FVector point = (ray.length * i * 100 / is) * ray.lineNor.Vector();
                    FVector pp = rt1.TransformPosition(point);
                    float dis = pp.Size();
                    if (dis > cfg.scan_distance * 100)
                    {
                        continue;
                    }
                    double h = FMath::RadiansToDegrees(FMath::Abs(2.0 * FMath::Atan2(pp.Y, pp.X)));
                    double v = FMath::RadiansToDegrees(FMath::Abs(2.0 * FMath::Atan2(pp.Z, pp.X)));
                    if (h < cfg.azimuth_fov && v < cfg.elevation_fov)
                    {
                        isin = true;
                        break;
                    }
                }
                if (isin)
                {
                    break;
                }
            }
            if (!isin)
            {
                continue;
            }

            FUltrasonicConfig newcfg = cfg;
            newcfg.installLocation = rt1.GetLocation();
            newcfg.installRotation = rt1.GetRotation().Rotator();
            newcfg.timePulse0 -= config.timePulse0;
            newcfg.timePulsePeriod = FMath::Max(cfg.timePulsePeriod, 0.001f);
            newcfg.pollTurn.ToLowerInline();
            other_ultrasonicArry.Add(newcfg);
            rs10.FindOrAdd(cfg.id) = 0;
        }
        config.all_ultrasonicArry = other_ultrasonicArry;

        UE_LOG(LogTemp, Log, TEXT("USS %d has %d Indirect"), config.id, config.all_ultrasonicArry.Num());
    }

    GConfig->GetBool(TEXT("Sensor"), TEXT("PublicMsg"), public_msg, GGameIni);
    return true;
}

void AUltrasonicSensor::Update(const FSensorInput& _Input, FSensorOutput& _Output)
{
    const FUltrasonicInput* Input = Cast_Sim<const FUltrasonicInput>(_Input);
    // FUltrasonicOutput* Output = Cast_Sim<FUltrasonicOutput>(_Output);

    if (config.openIndirectDistance)
    {
        if (Input->timeStamp < config.timePulse0 || (Input->timeStamp - timeStamp) < config.timePulsePeriod)
        {
            return;
        }
    }
    timeStamp = Input->timeStamp;

    FRotator Direct;
    float Distance = config.scan_distance;
    float Distance2 = config.scan_distance;
    uint32 tag_c = 0, tag_c2 = 0, tag_t = 0, tag_t2 = 0;
    // Output->distance = 99999;
    //  如果开启间接距离测距功能
    if (config.openIndirectDistance && !config.pollTurn.IsEmpty())
    {
        int cur = FMath::FloorToInt((Input->timeStamp - config.timePulse0) / config.timePulsePeriod);
        auto t = config.pollTurn[cur % config.pollTurn.Len()];
        if (t == TCHAR('0') || t == TCHAR('s'))
        {
            return;
        }
        rs10.FindOrAdd(config.id) = t == TCHAR('1') ? 1 : 3;
        for (auto& cfg : config.all_ultrasonicArry)
        {
            if (cfg.openIndirectDistance && !cfg.pollTurn.IsEmpty())
            {
                cur = FMath::FloorToInt((Input->timeStamp - cfg.timePulse0) / cfg.timePulsePeriod);
                t = cfg.pollTurn[cur % cfg.pollTurn.Len()];
                if (t == TCHAR('0') || t == TCHAR('r'))
                {
                    continue;
                }
                switch (t)
                {
                    case TCHAR('1'):
                        rs10.FindOrAdd(cfg.id) = 1;
                        break;
                    case TCHAR('s'):
                        rs10.FindOrAdd(cfg.id) = 2;
                        break;
                    case TCHAR('r'):
                        rs10.FindOrAdd(cfg.id) = 3;
                        break;
                    default:
                        rs10.FindOrAdd(cfg.id) = 0;
                        break;
                }
            }
        }
    }

    if (detectRays.Num() == 0)
    {
        return;
    }
    double tmp_sita = 1;
    if (!envManager)
    {
        auto gamestate = GetWorld()->GetGameState<ADisplayGameStateBase>();
        if (gamestate && gamestate->syncSystem && gamestate->syncSystem->envManager)
        {
            envManager = gamestate->syncSystem->envManager;
        }
    }
    if (envManager)
    {
        FSimEnvOriginInfo weather;
        envManager->GetCurrentEnvData().GetEnvOriginInfo(weather);
        double v = 331.5 + 0.6 * weather.Temperature;

        tmp_sita = sonic_v / v;
    }

    const float cross_limit = 0.2f;

    FString method = "depth";
    GConfig->GetString(TEXT("Sensor"), TEXT("UltrasonicMethod"), method, GGameIni);
    method = method.ToLower();
    // 如果是射线方法
    if (method == TEXT("raycast"))
    {
        // 计算射线起点和终点
        FVector Loc = GetActorLocation();
        FRotator Rot = GetActorRotation();
        FHitResult HitInfo(ForceInit);
        FCollisionQueryParams TraceParams = FCollisionQueryParams(FName(TEXT("Laser_Trace")));
        TraceParams.bTraceComplex = true;
        TraceParams.bReturnPhysicalMaterial = false;

        // 调试输出
        if (config.debug_frustum)
        {
            FVector kp[4];
            for (int i = 0; i < 4; i++)
            {
                FRotator A = UKismetMathLibrary::ComposeRotators(rangePt[i], Rot);
                kp[i] = Loc + A.Vector() * config.scan_distance * 100;
            }
            for (int i = 0, j = 3; i < 4; i++, j = (j + 1) % 4)
            {
                DrawDebugLine(GetWorld(), kp[i], kp[j], FColor(255, 0, 255), false, -1, 0, 2);
                DrawDebugLine(GetWorld(), Loc, kp[i], FColor(255, 0, 255), false, -1, 0, 2);
            }
            DrawDebugLine(GetWorld(), kp[0], kp[2], FColor(255, 0, 255), false, -1, 0, 2);
            DrawDebugLine(GetWorld(), kp[1], kp[3], FColor(255, 0, 255), false, -1, 0, 2);
        }
        int best = 0;
        // 射线扫描
        for (int32 i = 0; i < detectRays.Num(); ++i)
        {
            const auto& dl = detectRays[i];
            if (dl.length < 1e-4)
            {
                continue;
            }

            FRotator ResultRot = UKismetMathLibrary::ComposeRotators(dl.lineNor, Rot);

            FVector EndTrace = Loc + dl.length * 100 * ResultRot.Vector();
            GetWorld()->LineTraceSingleByChannel(HitInfo, Loc, EndTrace, ECollisionChannel::ECC_Visibility, TraceParams,
                FCollisionResponseParams::DefaultResponseParam);
            if (config.debug_scan)
            {
                DrawDebugLine(GetWorld(), Loc, EndTrace, FColor(0, 0, 255), false, -1, 0, 1);
            }

            if (!HitInfo.bBlockingHit)
            {
                continue;
            }
            // 获取当前回波距离

            float d = HitInfo.Distance * 0.01f;
            if (d < 0.001 || d > dl.length)
            {
                continue;
            }
            float nm = FMath::Abs(FVector::DotProduct(ResultRot.Vector(), HitInfo.ImpactNormal));
            nm *= 1.f - d / dl.length;
            if (nm < cross_limit)
            {
                continue;
            }
            // 计算二次回波距离（间接距离
            auto ndl = dl;
            ndl.length = d;
            float d2 = CalIndirectDistance(ndl, Input->timeStamp);
            uint32 u1 = 0, u2 = 0;
            auto actor = HitInfo.Actor;
            if (actor.IsValid() && actor->Tags.Num() > 0)
            {
                u1 = FCString::Atoi(*actor->Tags[0].ToString());
                u2 = 0;
                if (actor->Tags.Num() > 1)
                {
                    u2 = FCString::Atoi(*actor->Tags[1].ToString());
                }
                if (actor->Tags.Num() > 1 && actor->Tags[0].ToString().Len() < actor->Tags[1].ToString().Len())
                {
                    std::swap(u1, u2);
                }
            }

            if (Distance > d)
            {
                best = i;
                Distance = d;
                tag_c = u1;
                tag_t = u2;
            }
            if (Distance2 > d2)
            {
                Distance2 = d2;
                tag_c2 = u1;
                tag_t2 = u2;
            }

            if (config.debug_rescan)
            {
                DrawDebugLine(GetWorld(), Loc, EndTrace, FColor(0, 255, 0), false, -1, 0, 1);
            }
        }
        Direct = detectRays[best].lineNor;
    }
    // 如果是深度方法
    else if (method == TEXT("depth"))
    {
        TArray<FColor> bitMap;
        if (depthBuffer->GetCaptureImage(bitMap))
        {
            /* test
            IImageWrapperModule& ImageWrapperModule =
            FModuleManager::LoadModuleChecked<IImageWrapperModule>("ImageWrapper");
            TSharedPtr<IImageWrapper> ImageWrapper =
            ImageWrapperModule.CreateImageWrapper(EImageFormat::JPEG);
            if (ImageWrapper->SetRaw(bitMap.GetData(), sizeof(FColor) * depthBuffer->fovCfg.w *
                depthBuffer->fovCfg.h, depthBuffer->fovCfg.w, depthBuffer->fovCfg.h, ERGBFormat::BGRA, 8))
            {
                FString savePathWithName = config.savePath +
                    TEXT("Ultrasonic_Data_") + FString::FromInt(config.id) +
                    TEXT(".jpg");

                GetGameInstance()->GetSaveDataHandle()->SaveJPG(ImageWrapper->GetCompressed(85),
                    savePathWithName);
            } //*/
            int best = 0;
            // 距离最小值
            for (int32 i = 0; i < detectRays.Num(); ++i)
            {
                const auto& dl = detectRays[i];
                if (dl.length < 1e-4)
                {
                    continue;
                }
                // 直接距离
                float d = 0, nm = 0;
                int tag = 0;
                ;
                if (!depthBuffer->GetDepth(bitMap, dl.lineNor.Pitch, dl.lineNor.Yaw, d, nm, &tag))
                    continue;
                d *= 0.01f;
                if (d < 0.001 || d > dl.length)
                {
                    continue;
                }
                nm *= 1.f - d / dl.length;
                if (nm < cross_limit)
                {
                    continue;
                }
                if (3 == rs10[config.id])
                    d = dl.length;
                // 间接距离
                auto ndl = dl;
                ndl.length = d;
                float d2 = CalIndirectDistance(ndl, Input->timeStamp);
                uint32 u1 = 0, u2 = 0;
                if (stencilMap.find(tag) != stencilMap.end())
                {
                    u1 = stencilMap.at(tag).first;
                    u2 = stencilMap.at(tag).second;
                }
                if (Distance > d)
                {
                    best = i;
                    Distance = d;
                    tag_c = u1;
                    tag_t = u2;
                }
                if (Distance2 > d2)
                {
                    Distance2 = d2;
                    tag_c2 = u1;
                    tag_t2 = u2;
                }
            }
            Direct = detectRays[best].lineNor;
        }
    }
    else
    {
        return;
    }
    bool suc1 = Distance < config.scan_distance, suc2 = Distance2 < config.scan_distance;
    if (suc1)
    {
        Distance *= tmp_sita;
    }
    if (suc2)
    {
        Distance2 *= tmp_sita;
    }

    // 添加噪声
    if ((suc1 || suc2) && config.noise_factor > 1e-2 && config.noise_std > 1e-2)
    {
        //  白噪声
        std::random_device rd{};
        std::mt19937 gen{rd()};
        std::normal_distribution<> d{0, FMath::Max(0.1, config.noise_std * 0.1)};
        float angle = FMath::RadiansToDegrees(FMath::Acos(Direct.Vector().X));
        float delta = -0.00038f + Distance * 0.000217f * config.noise_factor + 0.00008 * angle;

        if (suc1)
        {
            double gass = d(gen);
            Distance += delta * gass;
            Distance += DeltaMaterial(tag_c, tag_t, Distance);
        }
        if (suc2)
        {
            double gass = d(gen);
            Distance2 += delta * gass;
            Distance2 += DeltaMaterial(tag_c, tag_t, Distance2);
        }
    }
    Distance = FMath::Clamp(Distance, 0.f, config.scan_distance);
    Distance2 = FMath::Clamp(Distance2, 0.f, config.scan_distance);
    if (Distance > Distance2)
    {
        std::swap(Distance, Distance2);
    }
    // 广播数据
    if (public_msg)
    {
        osi3::UltrasonicDetectionData sensor_feature;
        sensor_feature.mutable_header()->mutable_sensor_id()->set_value(config.id);
        sensor_feature.mutable_header()->mutable_measurement_time()->set_seconds(std::floor(Input->timeStamp * 0.001));
        sensor_feature.mutable_header()->mutable_measurement_time()->set_nanos(
            (((int64) Input->timeStamp) % 1000) * 1000000);
        if (suc1)
        {
            auto detection = sensor_feature.add_detection();
            // detection->mutable_object_id()->set_value(id);
            detection->set_distance(Distance);
        }
        if (suc2)
        {
            auto detection = sensor_feature.add_detection();
            detection->set_distance(Distance2);
        }
        sensor_feature.SerializeToString(&_Output.serialize_string);

        // std::string json;
        // google::protobuf::util::MessageToJsonString(sensor_feature, &json);
        // FString savePath =TEXT("e:/test/test.json");
        // Cast<UDisplayGameInstance>(GetWorld()->GetGameInstance())->
        // GetSaveDataHandle()->SaveString(ANSI_TO_TCHAR(json.c_str()),
        // savePath);
    }

    // 落盘
    if (config.bSaveData)
    {
        FUltrasonicOutput NewOutput;
        NewOutput.distance = Distance;
        NewOutput.distance2 = Distance2;
        NewOutput.timeStamp = Input->timeStamp;
        dataArry.Add(NewOutput);
    }
}

bool AUltrasonicSensor::Save()
{
    if (dataArry.Num() == 0 /*|| !FPaths::DirectoryExists(config.savePath)*/)
    {
        return false;
    }
    // 文本方式追加
    FString savePathWithName = config.savePath + TEXT("Ultrasonic_Data_") + FString::FromInt(config.id) + TEXT(".csv");
    FString StringData;
    if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*savePathWithName))
    {
        StringData += "timeStamp, Distance;";
        StringData += LINE_TERMINATOR;
    }

    for (auto& Elem : dataArry)
    {
        // StringData += FString::SanitizeFloat(Elem.id);
        // StringData += ",";
        StringData += FString::SanitizeFloat(Elem.timeStamp);
        StringData += ",";
        StringData += FString::SanitizeFloat(Elem.distance);
        StringData += ",";
        StringData += FString::SanitizeFloat(Elem.distance2);
        StringData += LINE_TERMINATOR;
    }
    dataArry.Empty();
    // return FFileHelper::SaveStringToFile(StringData, *savePathWithName, FFileHelper::EEncodingOptions::AutoDetect,
    // &IFileManager::Get(), EFileWrite::FILEWRITE_Append);
    return GetDisplayInstance()->GetSaveDataHandle()->SaveString(StringData, savePathWithName);
}

// 加载材质类型
bool AUltrasonicSensor::LoadStencilMap(const FString& dir)
{
    TArray<FString> txt;
    if (!FFileHelper::LoadFileToStringArray(txt, *(dir + TEXT("/stencil.dat"))))
    {
        UE_LOG(LogTemp, Log, TEXT("Cannot load stencil.dat"));
        return false;
    }
    for (auto tt : txt)
    {
        TArray<int64> bufs;
        FString LeftStr;
        FString RightStr;
        while (tt.Split(" ", &LeftStr, &RightStr))
        {
            if (!LeftStr.IsEmpty())
                bufs.Add(FCString::Atoi64(*LeftStr));
            tt = RightStr;
        }
        bufs.Add(FCString::Atoi64(*tt));
        int s = bufs[0];
        int c = bufs[1];
        int t = bufs[2];
        stencilMap[s] = std::make_pair(c, t);
    }
    return true;
}

// 加载材料数据
bool AUltrasonicSensor::loadCeoData(const FString& dir)
{
    TArray<FString> txt;
    if (!FFileHelper::LoadFileToStringArray(txt, *(dir + TEXT("/delta.dat"))))
    {
        UE_LOG(LogTemp, Log, TEXT("Cannot load delta.dat"));
        return false;
    }
    deltaCoe.clear();
    std::vector<size_t> tags;
    for (auto tt : txt)
    {
        int64 tag = 0;
        TArray<float> bufs;
        FString LeftStr;
        FString RightStr;
        if (tt.Split(" ", &LeftStr, &RightStr))
        {
            if (!LeftStr.IsEmpty())
                tag = FCString::Atoi64(*LeftStr);
            tt = RightStr;
        }
        while (tt.Split(" ", &LeftStr, &RightStr))
        {
            if (!LeftStr.IsEmpty())
                bufs.Add(FCString::Atof(*LeftStr));
            tt = RightStr;
        }
        bufs.Add(FCString::Atof(*tt));
        if (tag > 0 && bufs.Num() == 3)
        {
            DeltaCoe rd;
            rd.a = bufs[0];
            rd.b = bufs[1];
            rd.c = bufs[2];
            deltaCoe[tag] = rd;
            tags.push_back(tag);
        }
    }
    for (auto tag : tags)
    {
        size_t tag_c = tag / 100;
        size_t tag_t = tag % 100;

        size_t tag_c3 = tag_c - (tag_c % 10000);
        size_t tag_c2 = tag_c - (tag_c % 10000000);

        tag_c3 *= 100;
        tag_c2 *= 100;
        tag_c *= 100;
        if (deltaCoe.find(tag_c3 + tag_t) == deltaCoe.end())
        {
            deltaCoe[tag_c3 + tag_t] = deltaCoe[tag];
        }
        if (deltaCoe.find(tag_c2 + tag_t) == deltaCoe.end())
        {
            deltaCoe[tag_c2 + tag_t] = deltaCoe[tag_c3 + tag_t];
        }

        if (deltaCoe.find(tag_c) == deltaCoe.end())
        {
            deltaCoe[tag_c] = deltaCoe[tag];
        }
        if (deltaCoe.find(tag_c3) == deltaCoe.end())
        {
            deltaCoe[tag_c3] = deltaCoe[tag_c];
        }
        if (deltaCoe.find(tag_c2) == deltaCoe.end())
        {
            deltaCoe[tag_c2] = deltaCoe[tag_c3];
        }
    }
    return true;
}

// 获取当前材质
bool AUltrasonicSensor::GetCeoData(unsigned int tag_c, unsigned int tag_t, float& a, float& b, float& c)
{
    if (deltaCoe.empty())
    {
        return false;
    }
    size_t tag = ((size_t) tag_c) * 100 + tag_t;
    auto fd = deltaCoe.find(tag);
    if (fd == deltaCoe.end())
    {
        tag -= size_t(tag_c % 10000) * 100;
        fd = deltaCoe.find(tag);
        if (fd == deltaCoe.end())
        {
            tag -= size_t(tag_c % 10000000) * 100;
            fd = deltaCoe.find(tag);
            if (fd == deltaCoe.end())
            {
                tag = ((size_t) tag_c) * 100;
                fd = deltaCoe.find(tag);
                if (fd == deltaCoe.end())
                {
                    tag -= tag % 1000000ull;
                    fd = deltaCoe.find(tag);
                    if (fd == deltaCoe.end())
                    {
                        tag -= tag % 1000000000ull;
                        fd = deltaCoe.find(tag);
                    }
                }
            }
        }

        if (fd == deltaCoe.end())
        {
            return false;
        }
        tag = ((size_t) tag_c) * 100 + tag_t;
        deltaCoe[tag] = fd->second;
    }
    a = fd->second.a;
    b = fd->second.b;
    c = fd->second.c;
    return true;
}

bool AUltrasonicSensor::GetCeoData(int tag, float& a, float& b, float& c)
{
    if (stencilMap.find(tag) == stencilMap.end())
    {
        return false;
    }
    return GetCeoData(stencilMap.at(tag).first, stencilMap.at(tag).second, a, b, c);
}

// 材质响应
// https://doc.weixin.qq.com/doc/w3_AKsA-waDACcF2rCv7OMSDWcHBAB49?scode=AJEAIQdfAAoSLkvajrAKsA-waDACc
float AUltrasonicSensor::DeltaMaterial(unsigned int tag_c, unsigned int tag_t, float d)
{
    float a = 0, b = 0, c = 0;
    if (!GetCeoData(tag_c, tag_t, a, b, c))
    {
        return 0;
    }
    return (a * d * d + b * d + c) * 0.01f;
}
