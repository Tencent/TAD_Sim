// Fill out your copyright notice in the Description page of Project Settings.

#include "FovFilter.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"
#include "XmlFile.h"
#include "DisplayGameInstance.h"
#include "HadmapManager.h"
#include "CommandLine.h"

void Age::YearEnd()
{
    _data = _tmp;
    _tmp.Empty();
}
int Age::operator()(int id)
{
    auto& na = _tmp.Add(id);
    na = 1;
    auto* a = _data.Find(id);
    if (a)
    {
        na += *a;
    }
    return na;
}

bool FovFilter::Init(const FovFilterConfig& config_, hadmapue4::HadmapManager* hadmapHandle_)
{
    config = config_;
    hadmapHandle = hadmapHandle_;
    age[0] = Age();
    age[1] = Age();
    age[2] = Age();
    return true;
}
void FovFilter::Update(const FSimUpdateIn& simIn, FSimUpdateOut& simOuts)
{
    FVector egoPs(0);
    hadmapHandle->LonLatToLocal(simIn.egoData.begin().Value().position().x(),
        simIn.egoData.begin().Value().position().y(), simIn.egoData.begin().Value().position().z(), egoPs);
    egoPs += config.installLocation;
    FRotator egoRt = UKismetMathLibrary::ComposeRotators(
        config.installRotation, FRotator((float) (-simIn.egoData.begin().Value().rpy().y() * 180 / PI),
                                    (float) (-simIn.egoData.begin().Value().rpy().z() * 180 / PI - 90),
                                    (float) (simIn.egoData.begin().Value().rpy().x() * 180 / PI)));
    egoRt = egoRt.GetInverse();

    ///*~ Traffic ~*/
    //{
    //        for (auto &Elem : simIn.trafficData.cars())
    //        {
    //            FVector objPs(0);
    //            hadmapHandle->LonLatToLocal(Elem.x(), Elem.y(), simIn.egoData.position().z(), objPs);
    //            if (IsInView(egoPs, egoRt, objPs))
    //            {
    //                auto& newdata = *simOuts.trafficData.add_cars();
    //                newdata = Elem;
    //                newdata.set_age(age[0](Elem.id()));
    //            }
    //        }
    //        age[0].YearEnd();
    //}
    //
    ///* Pedestrian */
    //{
    //
    //    for (auto &Elem : simIn.trafficData.dynamicobstacles())
    //    {
    //        FVector objPs(0);
    //        hadmapHandle->LonLatToLocal(Elem.x(), Elem.y(), simIn.egoData.position().z(), objPs);
    //        if (IsInView(egoPs, egoRt, objPs))
    //        {
    //            auto& newdata = *simOuts.trafficData.add_dynamicobstacles();
    //            newdata = Elem;
    //            newdata.set_age(age[1](Elem.id()));
    //        }
    //    }
    //    age[1].YearEnd();

    //}
    //
    ///* Obstacle */
    //{

    //    for (auto &Elem : simIn.trafficData.staticobstacles())
    //    {
    //        FVector objPs(0);
    //        hadmapHandle->LonLatToLocal(Elem.x(), Elem.y(), simIn.egoData.position().z(), objPs);
    //        if (IsInView(egoPs, egoRt, objPs))
    //        {
    //            auto& newdata = *simOuts.trafficData.add_staticobstacles();
    //            newdata = Elem;
    //            newdata.set_age(age[2](Elem.id()));
    //        }
    //    }
    //    age[2].YearEnd();

    //}
    //
    // simOuts.topic_trafficData = "TRAFFIC";
    // simOuts.topic_trafficData = "ME_SENSOR";
    // simOuts.topic_trafficData += std::to_string(config.id);
}

bool FovFilter::IsInView(const FVector& loc, const FRotator& rrt, const FVector& object)
{
    FVector ActorToSensor = rrt.RotateVector(object - loc);
    float OriginDistance = FVector::DotProduct(ActorToSensor, ActorToSensor);
    if (OriginDistance > config.dis_range * config.dis_range)
    {
        return false;
    }
    float angle_h = FMath::Atan2(FMath::Abs(ActorToSensor.Y), ActorToSensor.X);
    float angle_v = FMath::Atan2(FMath::Abs(ActorToSensor.Z), FMath::Abs(ActorToSensor.X));
    if (angle_h <= FMath::DegreesToRadians(config.fov_Horizontal * 0.5f) &&
        angle_v <= FMath::DegreesToRadians(config.fov_Vertical * 0.5f))
    {
        return true;
    }
    return false;
}

bool FovFilterManager::Init(const FString& configFilePath, hadmapue4::HadmapManager* hadmapHandle_)
{
    filters.Empty();
    FXmlFile xmlfile(configFilePath);
    if (xmlfile.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("FovFilter: Begin read XmlFile."));
        FXmlNode* Sensor = xmlfile.GetRootNode();
        if (Sensor)
        {
            // ViewTruth
            FXmlNode* ViewTruth = Sensor->FindChildNode(TEXT("Truth"));
            if (ViewTruth)
            {
                // is Active?
                if (FCString::Atoi(*ViewTruth->GetAttribute(TEXT("Active"))))
                {
                    // FXmlNode* Config = Camera->FindChildNode(TEXT("Config"));
                    TArray<FXmlNode*> ConfigArry = ViewTruth->GetChildrenNodes();
                    for (auto& Config : ConfigArry)
                    {
                        if (Config)
                        {
                            FString PropStr = Config->GetAttribute(TEXT("Enabled"));
                            if (PropStr == TEXT("true"))
                            {
                                FovFilterConfig NewViewTruthConfig;

                                // TypeName
                                NewViewTruthConfig.typeName = TEXT("Truth");

                                // ID
                                // GetPropValue(Config, TEXT(""), NewViewTruthConfig.id);
                                FString ID = Config->GetAttribute(TEXT("ID"));
                                if (!ID.IsEmpty())
                                {
                                    NewViewTruthConfig.id = FCString::Atoi(*ID);
                                }
                                else
                                {
                                    UE_LOG(LogTemp, Warning, TEXT("Truth: Config ID is empty!"));
                                    continue;
                                }

                                FString Device = Config->GetAttribute(TEXT("Device"));
                                if (!Device.IsEmpty())
                                {
                                    NewViewTruthConfig.device = Device;
                                }
                                else
                                {
                                    NewViewTruthConfig.device = TEXT(".0");
                                    UE_LOG(LogTemp, Warning, TEXT("SensorManger: Config Device is empty!"));
                                }
                                FString device;
                                if (!FParse::Value(FCommandLine::Get(), TEXT("-device="), device))
                                {
                                    device = TEXT("0");
                                }
                                FString LeftStr;
                                FString RightStr;
                                device.Split(TEXT("."), &LeftStr, &RightStr);
                                if (RightStr.IsEmpty())
                                {
                                    device = LeftStr + TEXT(".0");
                                }

                                if (NewViewTruthConfig.device != device)
                                {
                                    continue;
                                }

                                // Location
                                FString LocationX = Config->GetAttribute(TEXT("LocationX"));
                                FString LocationY = Config->GetAttribute(TEXT("LocationY"));
                                FString LocationZ = Config->GetAttribute(TEXT("LocationZ"));
                                if (!LocationX.IsEmpty() && !LocationY.IsEmpty() && !LocationZ.IsEmpty())
                                {
                                    NewViewTruthConfig.installLocation.X = FCString::Atof(*LocationX);
                                    NewViewTruthConfig.installLocation.Y = FCString::Atof(*LocationY);
                                    NewViewTruthConfig.installLocation.Z = FCString::Atof(*LocationZ);
                                }
                                else
                                {
                                    UE_LOG(LogTemp, Warning, TEXT("Truth: Config Location is incomplete or empty!"));
                                    continue;
                                }

                                // Rotation
                                FString RotationX = Config->GetAttribute(TEXT("RotationX"));
                                FString RotationY = Config->GetAttribute(TEXT("RotationY"));
                                FString RotationZ = Config->GetAttribute(TEXT("RotationZ"));
                                if (!RotationX.IsEmpty() && !RotationY.IsEmpty() && !RotationZ.IsEmpty())
                                {
                                    NewViewTruthConfig.installRotation.Roll = FCString::Atof(*RotationX);
                                    NewViewTruthConfig.installRotation.Pitch = FCString::Atof(*RotationY);
                                    NewViewTruthConfig.installRotation.Yaw = FCString::Atof(*RotationZ);
                                }
                                else
                                {
                                    UE_LOG(LogTemp, Warning, TEXT("Truth: Config Rotation is incomplete or empty!"));
                                    continue;
                                }

                                // FOV
                                FString FOV_Horizontal = Config->GetAttribute(TEXT("hfov"));
                                FString FOV_Vertical = Config->GetAttribute(TEXT("vfov"));
                                if (!FOV_Horizontal.IsEmpty() && !FOV_Vertical.IsEmpty())
                                {
                                    NewViewTruthConfig.fov_Horizontal = FCString::Atof(*FOV_Horizontal);
                                    NewViewTruthConfig.fov_Vertical = FCString::Atof(*FOV_Vertical);
                                }
                                else
                                {
                                    UE_LOG(LogTemp, Warning, TEXT("Truth: Config fov is empty!"));
                                    continue;
                                }

                                // Frequency
                                FString Drange = Config->GetAttribute(TEXT("drange"));
                                if (!Drange.IsEmpty())
                                {
                                    NewViewTruthConfig.dis_range = FCString::Atof(*Drange);
                                }
                                else
                                {
                                    UE_LOG(LogTemp, Warning, TEXT("Truth: Config Drange is empty!"));
                                    continue;
                                }
                                FovFilter filter;
                                if (filter.Init(NewViewTruthConfig, hadmapHandle_))
                                {
                                    UE_LOG(LogTemp, Log, TEXT("Truth: ID=%d is succeed."), NewViewTruthConfig.id);
                                    filters.Add(filter);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return filters.Num() > 0;
}

void FovFilterManager::Update(const FSimUpdateIn& simIn, TArray<FSimUpdateOut>& simOuts)
{
    simOuts.Empty();
    simOuts.SetNum(filters.Num());
    for (int i = 0; i < filters.Num(); ++i)
    {
        filters[i].Update(simIn, simOuts[i]);
    }
}
