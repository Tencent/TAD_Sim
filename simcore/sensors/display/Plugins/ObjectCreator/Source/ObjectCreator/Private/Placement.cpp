#include "Placement.h"
#include "FileHelpers.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/MessageDialog.h"
#include "EditorModeManager.h"
#include "EditorActorFolders.h"
#include "Runtime/Foliage/Public/InstancedFoliageActor.h"
#include "Runtime/Foliage/Public/InstancedFoliage.h"
#include "Runtime/Foliage/Public/FoliageType.h"
#include "Engine/StaticMesh.h"
#include "Misc/FeedbackContext.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/KismetMathLibrary.h"
#include "ScopedTransaction.h"
#include "ActorFactories/ActorFactory.h"
#include "AssetSelection.h"
#include <array>
#include <vector>
#include "earcut.hpp"
#include "Pole.h"
#include <functional>
#include "Runtime/Engine/Public/ObjectEditorUtils.h"
#include <set>
#include "EngineUtils.h"
#include "Mercator.h"

bool FPlacement::isInBox(const FVector& p)
{
    return p.X >= Box0.X && p.X < Box1.X && p.Y >= Box0.Y && p.Y < Box1.Y;
}
bool FPlacement::LoadData(const TArray<FString>& fnames)
{
    FString lname = GWorld->GetName();
    int tileX = INT_MAX, tileY = INT_MAX;
    int fi = 0;
    if (lname.FindChar('_', fi))
    {
        lname = lname.Right(lname.Len() - 1 - fi);
        if (lname.FindChar('_', fi))
        {
            tileX = FCString::Atoi(*lname.Left(fi));
            tileY = FCString::Atoi(*lname.Right(lname.Len() - 1 - fi));
        }
    }
    if (tileX == INT_MAX || tileY == INT_MAX)
    {
        FText Message = FText::FromString(TEXT("The level is not meractor sublevel: ") + lname);
        FMessageDialog::Open(EAppMsgType::Ok, Message);
        return false;
    }
    auto tcen = tilec(tileX, tileY);
    auto tlb = tileo(tileX, tileY);
    tlb.first -= tcen.first;
    tlb.second -= tcen.second;
    auto trt = tlb;
    double tsize = tilesize();
    trt.first += tsize;
    trt.second += tsize;
    Box0.X = tlb.first * 100;
    Box0.Y = -trt.second * 100;
    Box1.X = trt.first * 100;
    Box1.Y = -tlb.second * 100;

    for (const auto& fname : fnames)
    {
        FString jbuf;
        if (FFileHelper::LoadFileToString(jbuf, *fname))
        {
            TSharedPtr<FJsonObject> json;
            TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(jbuf);
            if (FJsonSerializer::Deserialize(Reader, json))
            {
                auto jasset = json->GetArrayField(TEXT("Lines"));
                for (const auto& jass : jasset)
                {
                    auto jassobj = jass->AsObject();
                    SLineData s;
                    s.type = TEXT("Lines_") + FString::FromInt(jassobj->GetIntegerField(TEXT("category_id")));

                    double height = 0;
                    jassobj->TryGetNumberField("height", height);
                    s.size.Z = -height;
                    s.id = jassobj->GetIntegerField(TEXT("id"));
                    auto jpoint = jassobj->GetArrayField(TEXT("points"));
                    bool isin = false;
                    for (const auto& jpt : jpoint)
                    {
                        auto jp = jpt->AsArray();
                        if (jp.Num() == 3)
                        {
                            FVector pos;
                            pos.X = jp[0]->AsNumber() - tcen.first;
                            pos.Y = -jp[1]->AsNumber() + tcen.second;
                            pos.Z = jp[2]->AsNumber();
                            pos *= 100;
                            isin |= isInBox(pos);
                            s.line.Add(pos);
                        }
                    }
                    if (isin)
                    {
                        LineArray.FindOrAdd(s.type).Add(s);
                    }
                }
                jasset = json->GetArrayField(TEXT("Lines_2D"));
                for (const auto& jass : jasset)
                {
                    auto jassobj = jass->AsObject();
                    SLineData s;
                    s.type = TEXT("Lines_2D_") + FString::FromInt(jassobj->GetIntegerField(TEXT("category_id")));

                    double height = 0;
                    jassobj->TryGetNumberField("height", height);
                    s.size.Z = -height;
                    s.id = jassobj->GetIntegerField(TEXT("id"));
                    auto jpoint = jassobj->GetArrayField(TEXT("points"));
                    bool isin = false;
                    for (const auto& jpt : jpoint)
                    {
                        auto jp = jpt->AsArray();
                        if (jp.Num() >= 2)
                        {
                            FVector pos;
                            pos.X = jp[0]->AsNumber() - tcen.first;
                            pos.Y = -jp[1]->AsNumber() + tcen.second;
                            pos.Z = 0;
                            pos *= 100;
                            isin |= isInBox(pos);
                            s.line.Add(pos);
                        }
                    }
                    if (isin)
                        LineArray.FindOrAdd(s.type).Add(s);
                }
                jasset = json->GetArrayField(TEXT("Tree"));
                for (const auto& jass : jasset)
                {
                    auto jassobj = jass->AsObject();
                    auto id = jassobj->GetIntegerField(TEXT("id"));
                    auto datatype = jassobj->GetStringField("data_type");
                    double height = 0, radius = 0;
                    jassobj->TryGetNumberField("height", height);
                    jassobj->TryGetNumberField("radius", radius);
                    height = FMath::Max(0.0, height);
                    radius = FMath::Max(0.0, radius);
                    auto jpointss = jassobj->GetArrayField(TEXT("points"));
                    TArray<TArray<FVector>> pts;
                    std::vector<std::pair<bool, std::vector<bool>>> isins;
                    for (const auto& jpts : jpointss)
                    {
                        TArray<FVector> line;
                        auto jps = jpts->AsArray();
                        bool isin = false;
                        std::vector<bool> ins;
                        for (const auto& jp : jps)
                        {
                            auto p = jp->AsArray();
                            if (p.Num() > 1)
                            {
                                FVector pos;
                                pos.X = p[0]->AsNumber() - tcen.first;
                                pos.Y = -p[1]->AsNumber() + tcen.second;
                                pos.Z = p.Num() > 2 ? p[2]->AsNumber() : 0;
                                pos *= 100;
                                line.Add(pos);
                                bool in = isInBox(pos);
                                isin |= in;
                                ins.push_back(in);
                            }
                        }
                        if (line.Num() > 0)
                        {
                            pts.Add(line);
                            isins.push_back(std::make_pair(isin, ins));
                        }
                    }
                    if (pts.Num() == 0)
                    {
                        auto jpos = jassobj->GetArrayField(TEXT("pos"));

                        if (jpos.Num() >= 2)
                        {
                            FVector pos;
                            pos.X = jpos[0]->AsNumber() - tcen.first;
                            pos.Y = -jpos[1]->AsNumber() + tcen.second;
                            pos.Z = jpos.Num() > 2 ? jpos[2]->AsNumber() : 0;
                            pos *= 100;
                            bool isin = isInBox(pos);
                            pts.SetNum(1);
                            pts[0].Add(pos);
                            std::vector<bool> ins;
                            ins.push_back(isin);
                            isins.push_back(std::make_pair(isin, ins));
                            datatype = "point";
                        }

                        if (pts.Num() == 0)
                            continue;
                    }
                    auto type = TEXT("Tree_") + FString::FromInt(jassobj->GetIntegerField(TEXT("category_id")));

                    if (datatype == "point")
                    {
                        double yaw = 0;
                        jassobj->TryGetNumberField("yaw", yaw);
                        for (int i = 0; i < pts.Num(); ++i)
                        {
                            const auto& pt = pts[i];
                            for (int j = 0; j < pt.Num(); ++j)
                            {
                                if (!isins[i].second[j])
                                {
                                    continue;
                                }
                                const auto& p = pt[j];
                                SPosData s;
                                s.id = id;
                                s.type = type;
                                s.pos = p;
                                s.size = -FVector(radius * 200, radius * 200, height * 100);
                                s.rot.Yaw = -FMath::RadiansToDegrees(yaw);

                                PosArray.FindOrAdd(s.type).Add(s);
                            }
                        }
                    }
                    else if (datatype == "line")
                    {
                        for (int i = 0; i < pts.Num(); ++i)
                        {
                            if (!isins[i].first)
                            {
                                continue;
                            }
                            const auto& pt = pts[i];
                            SLineData s;
                            s.type = type;
                            s.line = pt;
                            s.id = id;
                            s.size = -FVector(radius * 200, radius * 200, height * 100);
                            LineArray.FindOrAdd(s.type).Add(s);
                        }
                    }
                    else if (datatype == "polygon")
                    {
                        if (isins[0].first)
                        {
                            SPolygonData s;
                            s.type = type;
                            s.id = id;
                            s.size = -FVector(radius * 200, radius * 200, height * 100);
                            s.outline = pts[0];
                            for (int i = 1; i < pts.Num(); i++)
                            {
                                s.holes.Add(pts[i]);
                            }
                            PolygonArray.FindOrAdd(s.type).Add(s);
                        }
                    }
                }
                jasset = json->GetArrayField(TEXT("Polygon"));
                for (const auto& jass : jasset)
                {
                    auto jassobj = jass->AsObject();
                    auto id = jassobj->GetIntegerField(TEXT("id"));
                    auto jpointss = jassobj->GetArrayField(TEXT("points"));
                    TArray<TArray<FVector>> pts;
                    bool isin = false;

                    if (jpointss.Num() > 0 && jpointss[0]->AsArray().Num() > 0 &&
                        jpointss[0]->AsArray()[0]->AsArray().Num() > 0)
                    {
                        for (const auto& jpts : jpointss)
                        {
                            TArray<FVector> line;
                            auto jps = jpts->AsArray();
                            for (const auto& jp : jps)
                            {
                                auto p = jp->AsArray();
                                if (p.Num() > 1)
                                {
                                    FVector pos;
                                    pos.X = p[0]->AsNumber() - tcen.first;
                                    pos.Y = -p[1]->AsNumber() + tcen.second;
                                    pos.Z = p.Num() > 2 ? p[2]->AsNumber() : 0;
                                    pos *= 100;
                                    line.Add(pos);
                                    isin |= isInBox(pos);
                                }
                            }
                            if (line.Num() > 0)
                            {
                                pts.Add(line);
                            }
                        }
                    }
                    else
                    {
                        TArray<FVector> line;
                        for (const auto& jp : jpointss)
                        {
                            auto p = jp->AsArray();
                            if (p.Num() > 1)
                            {
                                FVector pos;
                                pos.X = p[0]->AsNumber() - tcen.first;
                                pos.Y = -p[1]->AsNumber() + tcen.second;
                                pos.Z = p.Num() > 2 ? p[2]->AsNumber() : 0;
                                pos *= 100;
                                line.Add(pos);
                                isin |= isInBox(pos);
                            }
                        }
                        if (line.Num() > 0)
                        {
                            pts.Add(line);
                        }
                    }
                    if (isin)
                    {
                        auto type = TEXT("Polygon_") + FString::FromInt(jassobj->GetIntegerField(TEXT("category_id")));

                        SPolygonData s;
                        s.type = type;
                        s.id = id;
                        s.outline = pts[0];
                        for (int i = 1; i < pts.Num(); i++)
                        {
                            s.holes.Add(pts[i]);
                        }
                        PolygonArray.FindOrAdd(s.type).Add(s);
                    }
                }
                jasset = json->GetArrayField(TEXT("Arrows"));
                for (const auto& jass : jasset)
                {
                    auto jassobj = jass->AsObject();
                    SPosData s;
                    s.type = TEXT("Arrows_") + FString::FromInt(jassobj->GetIntegerField(TEXT("category_id")));

                    s.id = jassobj->GetIntegerField(TEXT("id"));
                    auto jpoint = jassobj->GetArrayField(TEXT("pos"));
                    bool isin = false;
                    if (jpoint.Num() == 3)
                    {
                        s.pos.X = jpoint[0]->AsNumber() - tcen.first;
                        s.pos.Y = -jpoint[1]->AsNumber() + tcen.second;
                        s.pos.Z = jpoint[2]->AsNumber();
                        s.pos *= 100;
                    }
                    if (isInBox(s.pos))
                    {
                        auto jrpy = jassobj->GetArrayField(TEXT("rpy"));
                        if (jrpy.Num() == 3)
                        {
                            s.rot.Yaw = -FMath::RadiansToDegrees(jrpy[2]->AsNumber());
                            s.rot.Pitch = FMath::RadiansToDegrees(jrpy[1]->AsNumber());
                            s.rot.Roll = -FMath::RadiansToDegrees(jrpy[0]->AsNumber());
                        }
                        PosArray.FindOrAdd(s.type).Add(s);
                    }
                }
                jasset = json->GetArrayField(TEXT("V2X"));
                for (const auto& jass : jasset)
                {
                    auto jassobj = jass->AsObject();
                    SPosData s;
                    s.type = TEXT("V2X_") + FString::FromInt(jassobj->GetIntegerField(TEXT("category_id")));

                    s.id = jassobj->GetIntegerField(TEXT("id"));
                    auto jpoint = jassobj->GetArrayField(TEXT("pos"));
                    if (jpoint.Num() >= 2)
                    {
                        s.pos.X = jpoint[0]->AsNumber() - tcen.first;
                        s.pos.Y = -jpoint[1]->AsNumber() + tcen.second;
                        if (jpoint.Num() > 2)
                            s.pos.Z = jpoint[2]->AsNumber();
                        s.pos *= 100;
                    }

                    int32 lid = 0;
                    jassobj->TryGetNumberField(TEXT("link_to"), lid);
                    s.pid = FString::FromInt(lid);
                    if (lid > 0 && pole)
                    {
                        if (!pole->Has(s.pid))
                        {
                            continue;
                        }
                        pole->Pin(s.pos, s.rot.Yaw, s.pid, FVector(30, 200, 120), 200);
                    }
                    else if (!isInBox(s.pos))
                        continue;

                    auto jrpy = jassobj->GetArrayField(TEXT("rpy"));
                    if (jrpy.Num() == 3)
                    {
                        s.rot.Yaw = -FMath::RadiansToDegrees(jrpy[2]->AsNumber());
                        s.rot.Pitch = FMath::RadiansToDegrees(jrpy[1]->AsNumber());
                        s.rot.Roll = -FMath::RadiansToDegrees(jrpy[0]->AsNumber());
                    }

                    const TSharedPtr<FJsonObject>* userdata = 0;
                    if (jassobj->TryGetObjectField(TEXT("userdata"), userdata) && userdata && userdata->Get())
                    {
                        for (auto m : userdata->Get()->Values)
                        {
                            s.userdata.FindOrAdd(m.Key) = m.Value->AsString();
                        }
                    }
                    const TSharedPtr<FJsonObject>* deviceinfo = 0;
                    if (jassobj->TryGetObjectField(TEXT("deviceinfo"), deviceinfo) && deviceinfo && deviceinfo->Get())
                    {
                        for (auto m : deviceinfo->Get()->Values)
                        {
                            s.deviceinfo.FindOrAdd(m.Key) = m.Value->AsString();
                        }
                    }
                    PosArray.FindOrAdd(s.type).Add(s);
                }
                jasset = json->GetArrayField(TEXT("TrafficLight"));
                for (const auto& jass : jasset)
                {
                    auto jassobj = jass->AsObject();
                    SPosData s;
                    s.type = TEXT("TrafficLight_") + FString::FromInt(jassobj->GetIntegerField(TEXT("category_id")));

                    s.id = jassobj->GetIntegerField(TEXT("id"));
                    auto jpoint = jassobj->GetArrayField(TEXT("pos"));
                    if (jpoint.Num() == 3)
                    {
                        s.pos.X = jpoint[0]->AsNumber() - tcen.first;
                        s.pos.Y = -jpoint[1]->AsNumber() + tcen.second;
                        s.pos.Z = jpoint[2]->AsNumber();
                        s.pos *= 100;
                    }
                    int32 lid = 0;
                    jassobj->TryGetNumberField(TEXT("link_to"), lid);
                    s.pid = FString::FromInt(lid);
                    if (lid > 0 && pole)
                    {
                        if (!pole->Has(s.pid))
                        {
                            continue;
                        }
                        pole->Pin(s.pos, s.rot.Yaw, s.pid, FVector(100, 300, 200), 100);
                    }
                    else if (!isInBox(s.pos))
                        continue;

                    auto jrpy = jassobj->GetArrayField(TEXT("rpy"));
                    if (jrpy.Num() == 3)
                    {
                        s.rot.Yaw = -FMath::RadiansToDegrees(jrpy[2]->AsNumber());
                        s.rot.Pitch = FMath::RadiansToDegrees(jrpy[1]->AsNumber());
                        s.rot.Roll = -FMath::RadiansToDegrees(jrpy[0]->AsNumber());
                    }
                    PosArray.FindOrAdd(s.type).Add(s);
                }
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Can`t read json: %s"), *Reader->GetErrorMessage());
                FText Message = FText::FromString(Reader->GetErrorMessage());
                FMessageDialog::Open(EAppMsgType::Ok, Message);
            }
        }
    }
    return true;
}
bool FPlacement::LoadConfig()
{
    PosArray.Empty();
    LineArray.Empty();
    PolygonArray.Empty();
    FString jpath = FPaths::ProjectConfigDir() + TEXT("asset.json");

    FString jbuf;
    if (FFileHelper::LoadFileToString(jbuf, *jpath))
    {
        TSharedPtr<FJsonObject> json;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(jbuf);
        if (FJsonSerializer::Deserialize(Reader, json))
        {
            if (json->GetStringField(TEXT("version")) == TEXT("0.1.0"))
            {
                auto jasset = json->GetArrayField(TEXT("asset"));
                for (const auto& jass : jasset)
                {
                    auto jassobj = jass->AsObject();
                    FString name = jassobj->GetStringField(TEXT("name"));
                    if (name.IsEmpty())
                    {
                        continue;
                    }
                    Config cfg;
                    jassobj->TryGetBoolField(TEXT("foliage"), cfg.Foliaged);

                    jassobj->TryGetBoolField(TEXT("foliage"), cfg.Foliaged);
                    jassobj->TryGetBoolField(TEXT("pin_terrain"), cfg.PinTerrain);
                    jassobj->TryGetNumberField(TEXT("pin_radius"), cfg.PinRadius);
                    jassobj->TryGetNumberField(TEXT("turnover"), cfg.Turnover);
                    jassobj->TryGetNumberField(TEXT("rand_angle"), cfg.RandAngle);
                    jassobj->TryGetNumberField(TEXT("rand_rot"), cfg.RandRot);
                    jassobj->TryGetNumberField(TEXT("rand_size"), cfg.RandSize);
                    jassobj->TryGetNumberField(TEXT("line_len"), cfg.LineLen);
                    jassobj->TryGetBoolField(TEXT("pos_top"), cfg.PosTop);
                    jassobj->TryGetNumberField(TEXT("pin_road"), cfg.pinRoad);
                    jassobj->TryGetNumberField(TEXT("ply_rscale"), cfg.PlyDensity);
                    jassobj->TryGetNumberField(TEXT("offsetX"), cfg.offset[0]);
                    jassobj->TryGetNumberField(TEXT("offsetY"), cfg.offset[1]);
                    jassobj->TryGetNumberField(TEXT("offsetZ"), cfg.offset[2]);
                    jassobj->TryGetNumberField(TEXT("scaleX"), cfg.scale[0]);
                    jassobj->TryGetNumberField(TEXT("scaleY"), cfg.scale[1]);
                    jassobj->TryGetNumberField(TEXT("scaleZ"), cfg.scale[2]);
                    jassobj->TryGetBoolField(TEXT("nobuilding"), cfg.detect_building);
                    const TArray<TSharedPtr<FJsonValue>>* jassetpaths;
                    jassobj->TryGetArrayField(TEXT("asset_path"), jassetpaths);
                    if (jassetpaths)
                        for (const auto& jasspath : *jassetpaths)
                        {
                            cfg.AssetPath.Add(jasspath->AsString());
                        }

                    AssetConfigs.FindOrAdd(name) = cfg;
                }
            }
            return true;
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Can`t read json: %s"), *Reader->GetErrorMessage());
            FText Message = FText::FromString(Reader->GetErrorMessage());
            FMessageDialog::Open(EAppMsgType::Ok, Message);
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("Read asset config falid."));
    FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("Read asset config falid."));
    return false;
}

void FPlacement::CreateAndPlace(const TArray<FString>& path)
{
    if (!LoadConfig())
        return;

    if (!LoadData(path))
    {
        FText Message = FText::FromString(TEXT("Load data faild."));
        FMessageDialog::Open(EAppMsgType::Ok, Message);
        return;
    }
    SpawnAssets();
}

// Rand between [min,max]
inline float randFloat(float min, float max)
{
    return min + FMath::FRand() * (max - min);
}

bool FPlacement::SpawnAssets()
{
    TMap<FString, AActor*> existNames;
    for (TActorIterator<AActor> ActorItr(GWorld); ActorItr; ++ActorItr)
    {
        FString path = ActorItr->GetFolderPath().ToString();
        FString name = ActorItr->GetName();
        existNames.Add(path / name) = *ActorItr;
    }

    for (const auto& p : PosArray)
    {
        const auto config = AssetConfigs.Find(p.Key);
        if (!config)
        {
            UE_LOG(LogTemp, Warning, TEXT("Find config falid: %s"), *p.Key);
            if (EAppReturnType::Cancel ==
                FMessageDialog::Open(EAppMsgType::OkCancel, FText::FromString(TEXT("Find config falid: ") + p.Key)))
                return false;
            continue;
        }
        if (config->AssetPath.Num() == 0)
        {
            UE_LOG(LogTemp, Warning, TEXT("Config null: %s"), *p.Key);
            if (EAppReturnType::Cancel ==
                FMessageDialog::Open(EAppMsgType::OkCancel, FText::FromString(TEXT("Config null falid: ") + p.Key)))
                return false;
            continue;
        }
        TArray<FAssetData> InAssets;
        for (const auto& apath : config->AssetPath)
        {
            UObject* Asset = LoadObject<UObject>(nullptr, *apath);
            if (Asset)
            {
                InAssets.Add(FAssetData(Asset));
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Cannot load: %s"), *apath);
                if (EAppReturnType::Cancel ==
                    FMessageDialog::Open(EAppMsgType::OkCancel, FText::FromString(TEXT("Cannot load: ") + apath)))
                    return false;
                continue;
            }
        }
        TArray<SPosData> poss = p.Value;
        if (config->PinTerrain)
        {
            for (auto& p1 : poss)
            {
                UWorld* World = GWorld;
                FVector Start = p1.pos + FVector::UpVector * config->PinRadius;
                FVector End = p1.pos - FVector::UpVector * config->PinRadius * 2;

                FHitResult Result;
                bool bHit =
                    World->LineTraceSingleByChannel(Result, Start, End, ECollisionChannel::ECC_GameTraceChannel2);
                if (bHit)
                {
                    p1.pos = Result.ImpactPoint;
                    p1.pos.Z += 2;
                }
            }
        }

        if (!config->Foliaged)
        {
            bool cancel = false;
            UWorld* World = GWorld;
            ULevel* DesiredLevel = GWorld->GetCurrentLevel();
            FString NewFolderName = WorldDir;
            FActorFolders::Get().CreateFolder(*GWorld, *NewFolderName);
            NewFolderName += TEXT("/Point");
            FActorFolders::Get().CreateFolder(*World, *NewFolderName);
            NewFolderName += TEXT("/") + p.Key;
            FActorFolders::Get().CreateFolder(*World, *NewFolderName);
            SpawnActorAsset(NewFolderName, p.Key, InAssets, *config, poss, existNames, &cancel);
            if (cancel)
                break;
        }
        else
            SpawnFoliageAsset(InAssets, *config, poss);
    }
    for (const auto& p : LineArray)
    {
        const auto config = AssetConfigs.Find(p.Key);
        if (!config)
        {
            UE_LOG(LogTemp, Warning, TEXT("Find config falid: %s"), *p.Key);
            if (EAppReturnType::Cancel ==
                FMessageDialog::Open(EAppMsgType::OkCancel, FText::FromString(TEXT("LineConfig find falid: ") + p.Key)))
                return false;
            continue;
        }
        if (config->AssetPath.Num() == 0)
        {
            UE_LOG(LogTemp, Warning, TEXT("Config null: %s"), *p.Key);
            if (EAppReturnType::Cancel ==
                FMessageDialog::Open(EAppMsgType::OkCancel, FText::FromString(TEXT("LineConfig null falid: ") + p.Key)))
                return false;
            continue;
        }
        int cc = 0;
        FString NewFolderName = WorldDir;
        if (!config->Foliaged)
        {
            UWorld* World = GWorld;
            ULevel* DesiredLevel = GWorld->GetCurrentLevel();
            FActorFolders::Get().CreateFolder(*GWorld, *NewFolderName);
            NewFolderName += TEXT("/Line");
            FActorFolders::Get().CreateFolder(*GWorld, *NewFolderName);
            NewFolderName += TEXT("/") + p.Key;
            FActorFolders::Get().CreateFolder(*GWorld, *NewFolderName);
        }
        for (const auto& line : p.Value)
        {
            int32 fi = FMath::Rand() % config->AssetPath.Num();
            UObject* InAsset = LoadObject<UObject>(nullptr, *config->AssetPath[fi]);
            if (!InAsset)
            {
                UE_LOG(LogTemp, Warning, TEXT("Cannot load: %s"), *config->AssetPath[fi]);
                if (EAppReturnType::Cancel ==
                    FMessageDialog::Open(
                        EAppMsgType::OkCancel, FText::FromString(TEXT("Line cannot load: ") + config->AssetPath[fi])))
                    return false;
                continue;
            }
            TArray<FAssetData> Assetdatas;
            Assetdatas.Add(InAsset);
            double llen = config->LineLen;
            if (llen < 1e-6)
            {
                UStaticMesh* sm = Cast<UStaticMesh>(InAsset);
                AStaticMeshActor* sm1 = Cast<AStaticMeshActor>(InAsset);
                UBlueprint* bp = Cast<UBlueprint>(InAsset);
                if (sm)
                {
                    FVector s = sm->GetBoundingBox().GetSize();
                    llen = FMath::Abs(FRotator(0, config->Turnover, 0).RotateVector(s).X * config->scale[0]);
                }
                else if (sm1 && sm1->GetStaticMeshComponent() && sm1->GetStaticMeshComponent()->GetStaticMesh())
                {
                    FVector s = sm1->GetStaticMeshComponent()->GetStaticMesh()->GetBoundingBox().GetSize();
                    llen = FMath::Abs(FRotator(0, config->Turnover, 0).RotateVector(s).X * config->scale[0]);
                }
                else if (bp)
                {
                    AStaticMeshActor* actor = Cast<AStaticMeshActor>(bp->GeneratedClass->GetDefaultObject());
                    if (actor)
                    {
                        FVector s = actor->GetStaticMeshComponent()->GetStaticMesh()->GetBoundingBox().GetSize();
                        llen = FMath::Abs(FRotator(0, config->Turnover, 0).RotateVector(s).X * config->scale[0]);
                    }
                    else
                        continue;
                }
                else
                {
                    continue;
                }
            }
            TArray<SPosData> poss;
            if (Line2Pos(*config, line, poss, llen))
            {
                if (config->Foliaged)
                {
                    SpawnFoliageAsset(Assetdatas, *config, poss);
                }
                else
                {
                    FString actorname = NewFolderName + TEXT("/") + FString::FromInt(line.id);
                    FActorFolders::Get().CreateFolder(*GWorld, *actorname);
                    bool cancel = false;
                    SpawnActorAsset(actorname, p.Key + TEXT("_") + FString::FromInt(line.id), Assetdatas, *config, poss,
                        existNames, &cancel);
                    if (cancel)
                        break;
                }
            }
        }
    }
    for (const auto& p : PolygonArray)
    {
        const auto config = AssetConfigs.Find(p.Key);
        if (!config)
        {
            UE_LOG(LogTemp, Warning, TEXT("Find polyconfig falid: %s"), *p.Key);
            if (EAppReturnType::Cancel ==
                FMessageDialog::Open(EAppMsgType::OkCancel, FText::FromString(TEXT("PolyConfig find falid: ") + p.Key)))
                return false;
            continue;
        }
        if (config->AssetPath.Num() == 0)
        {
            UE_LOG(LogTemp, Warning, TEXT("Config null: %s"), *p.Key);
            if (EAppReturnType::Cancel ==
                FMessageDialog::Open(EAppMsgType::OkCancel, FText::FromString(TEXT("PolyConfig null falid: ") + p.Key)))
                return false;
            continue;
        }
        int cc = 0;
        FString NewFolderName = TEXT("Polygon");
        if (!config->Foliaged)
        {
            UWorld* World = GWorld;
            ULevel* DesiredLevel = GWorld->GetCurrentLevel();
            FActorFolders::Get().CreateFolder(*GWorld, *NewFolderName);
            NewFolderName += TEXT("/") + p.Key;
            FActorFolders::Get().CreateFolder(*GWorld, *NewFolderName);
        }
        for (const auto& ply : p.Value)
        {
            TArray<float> areas;
            TArray<FAssetData> Assetdatas;
            for (const auto& apath : config->AssetPath)
            {
                UObject* Asset = LoadObject<UObject>(nullptr, *apath);
                if (!Asset)
                {
                    UE_LOG(LogTemp, Warning, TEXT("Cannot load: %s"), *apath);
                    if (EAppReturnType::Cancel == FMessageDialog::Open(EAppMsgType::OkCancel,
                                                      FText::FromString(TEXT("Poly cannot load: ") + apath)))
                        return false;
                    continue;
                }
                float area = 0;
                UStaticMesh* sm = Cast<UStaticMesh>(Asset);
                AStaticMeshActor* sm1 = Cast<AStaticMeshActor>(Asset);
                UBlueprint* bp = Cast<UBlueprint>(Asset);
                UFoliageType* flg = Cast<UFoliageType>(Asset);
                if (sm)
                {
                    FVector s = sm->GetBoundingBox().GetSize();
                    area = FMath::Max(s.X * config->scale[0], s.Y * config->scale[1]);
                }
                else if (sm1 && sm1->GetStaticMeshComponent() && sm1->GetStaticMeshComponent()->GetStaticMesh())
                {
                    FVector s = sm1->GetStaticMeshComponent()->GetStaticMesh()->GetBoundingBox().GetSize();
                    area = FMath::Max(s.X * config->scale[0], s.Y * config->scale[1]);
                }
                else if (bp)
                {
                    AStaticMeshActor* actor = Cast<AStaticMeshActor>(bp->GeneratedClass->GetDefaultObject());
                    if (actor)
                    {
                        FVector s = actor->GetStaticMeshComponent()->GetStaticMesh()->GetBoundingBox().GetSize();
                        area = FMath::Max(s.X * config->scale[0], s.Y * config->scale[1]);
                    }
                    else
                        continue;
                }
                else if (flg)
                {
                    UStaticMesh* sm_ = Cast<UStaticMesh>(flg->GetSource());
                    if (sm_)
                    {
                        FVector s = sm_->GetBoundingBox().GetSize();
                        area = FMath::Max(s.X * config->scale[0], s.Y * config->scale[1]);
                    }
                }
                else
                {
                    continue;
                }
                if (area > 0.0001f)
                {
                    areas.Add(area * config->PlyDensity);
                    Assetdatas.Add(Asset);
                }
            }
            TArray<SPosData> poss;
            if (Polgyon2Pos(*config, ply, poss, areas))
            {
                if (config->Foliaged)
                    SpawnFoliageAsset(Assetdatas, *config, poss);
                else
                {
                    FString actorname = NewFolderName + TEXT("/") + FString::FromInt(ply.id);
                    FActorFolders::Get().CreateFolder(*GWorld, *actorname);
                    bool cancel = false;
                    SpawnActorAsset(actorname, p.Key + TEXT("_") + FString::FromInt(ply.id), Assetdatas, *config, poss,
                        existNames, &cancel);

                    if (cancel)
                        break;
                }
            }
        }
    }

    return true;
}

bool FPlacement::SpawnFoliageAsset(const TArray<FAssetData>& InAssets, const Config& cfg, const TArray<SPosData>& poss)
{
    UWorld* World = GWorld;
    TArray<UFoliageType*> FoliageTypes;
    for (const auto& InAsset : InAssets)
    {
        UFoliageType* FoliageType = NULL;
        UObject* Asset = LoadObject<UObject>(NULL, *InAsset.ObjectPath.ToString());
        UStaticMesh* StaticMesh = Cast<UStaticMesh>(Asset);
        if (StaticMesh)
        {
            {
                const FScopedTransaction Transaction(
                    NSLOCTEXT("UnrealEd", "FoliageMode_AddTypeTransaction", "Add Foliage Type"));

                AInstancedFoliageActor* IFA =
                    AInstancedFoliageActor::GetInstancedFoliageActorForCurrentLevel(World, true);
                FoliageType = IFA->GetLocalFoliageTypeForSource(StaticMesh);
                if (!FoliageType)
                {
                    IFA->AddMesh(StaticMesh, &FoliageType);
                }
            }

            // If there is multiple levels for this world, save the foliage directly as an asset, so user will be able
            // to paint over all levels by default
            if (World->GetStreamingLevels().Num() > 0)
            {
                /*UFoliageType* TypeSaved = FFoliageEditUtility::SaveFoliageTypeObject(FoliageType);
                if (TypeSaved != nullptr && TypeToSave != FoliageType)
                {
                    ReplaceSettingsObject(FoliageType, TypeSaved);
                }
                if (TypeSaved != nullptr)
                {
                    FoliageType = TypeSaved;
                }*/
            }
        }
        else
        {
            const FScopedTransaction Transaction(
                NSLOCTEXT("UnrealEd", "FoliageMode_AddTypeTransaction", "Add Foliage Type"));

            FoliageType = Cast<UFoliageType>(Asset);
            if (FoliageType)
            {
                AInstancedFoliageActor* IFA =
                    AInstancedFoliageActor::GetInstancedFoliageActorForCurrentLevel(GWorld, true);
                FoliageType = IFA->AddFoliageType(FoliageType);
            }
        }

        // if (FoliageType)
        //{
        //     UWorld* World = GEditor->GetEditorWorldContext().World();
        //     ULevel* CurrentLevel = World->GetCurrentLevel();
        //     const int32 NumLevels = World->GetNumLevels();
        //     for (int32 LevelIdx = 0; LevelIdx < NumLevels; ++LevelIdx)
        //     {
        //         ULevel* Level = World->GetLevel(LevelIdx);
        //         if (Level && Level->bIsVisible)
        //         {
        //             AInstancedFoliageActor* IFA = AInstancedFoliageActor::GetInstancedFoliageActorForLevel(Level);
        //             if (IFA)
        //             {
        //                 for (auto& MeshPair : IFA->FoliageMeshes)
        //                 {
        //                     if (MeshPair.Key == nullptr)    //if asset has already been deleted we can't paint
        //                     {
        //                         continue;
        //                     }
        //                     // Non-shared objects can be painted only into their own level
        //                     // Assets can be painted everywhere
        //                     if (!MeshPair.Key->IsAsset() && MeshPair.Key->GetOutermost() !=
        //                     CurrentLevel->GetOutermost())
        //                     {
        //                         continue;
        //                     }
        //                     FoliageMesh = MakeShareable(new FFoliageMeshUIInfo(MeshPair.Key));
        //                     int32 PlacedInstanceCount = MeshPair.Value->GetPlacedInstanceCount();
        //                     FoliageMesh->InstanceCountTotal += PlacedInstanceCount;
        //                     if (Level == World->GetCurrentLevel())
        //                     {
        //                         FoliageMesh->InstanceCountCurrentLevel += PlacedInstanceCount;
        //                     }
        //                 }
        //             }
        //         }
        //     }
        // }

        if (FoliageType)
            FoliageTypes.Add(FoliageType);
    }
    if (FoliageTypes.Num() == 0)
    {
        FText Message = FText::FromString(TEXT("FoliageType is null"));
        FMessageDialog::Open(EAppMsgType::Ok, Message);
        return false;
    }
    AInstancedFoliageActor* IFA = AInstancedFoliageActor::GetInstancedFoliageActorForCurrentLevel(World, true);
    if (UPrimitiveComponent* PrimitiveComponent = IFA->FindComponentByClass<UPrimitiveComponent>())
    {
        PrimitiveComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        PrimitiveComponent->Mobility = EComponentMobility::Static;
    }

    TArray<FFoliageInfo*> MeshInfos;
    for (const auto& FoliageType : FoliageTypes)
    {
        MeshInfos.Add(IFA->FindOrAddMesh(FoliageType));
    }

    auto getMeshSize = [&](UObject* InAsset)
    {
        UStaticMesh* sm = Cast<UStaticMesh>(InAsset);
        FVector s(0, 0, 0);
        if (sm)
        {
            s = sm->GetBoundingBox().GetSize();
            s = FRotator(0, cfg.Turnover, 0).RotateVector(s);
        }
        s.X *= cfg.scale[0];
        s.Y *= cfg.scale[1];
        s.Z *= cfg.scale[2];
        return s;
    };

    if (cfg.detect_building)
        for (const auto& ps : poss)
        {
            FVector pos = ps.pos;
            {
                FVector Start = pos + FVector::UpVector * 100000;
                FVector End = pos - FVector::UpVector * 100000;

                FHitResult Result;
                bool bHit =
                    World->LineTraceSingleByChannel(Result, Start, End, ECollisionChannel::ECC_GameTraceChannel3);
                if (bHit)
                {
                    continue;
                }
            }
            FRotator rot = ps.rot;
            FVector size = ps.size;

            int32 fi = FMath::Rand() % MeshInfos.Num();

            if (size.X < 0 || size.Y < 0 || size.Z < 0)
            {
                if (FoliageTypes.Num() > 1)
                {
                    float mins = FLT_MAX;
                    for (int32 i = 0; i < FoliageTypes.Num(); i++)
                    {
                        FVector s = getMeshSize(FoliageTypes[i]->GetSource());
                        if (s.Z < 1e-4)
                        {
                            UE_LOG(LogTemp, Warning, TEXT("Cannot get the foliage size: %d"), i);
                            if (EAppReturnType::Cancel ==
                                FMessageDialog::Open(EAppMsgType::OkCancel,
                                    FText::FromString(TEXT("Cannot get the flliage size: ") + FString::FromInt(i))))
                                return false;
                            continue;
                        }
                        float zs = -size.Z / s.Z;
                        if (zs < 1.f)
                        {
                            zs = 1.0f / zs;
                        }
                        if (zs < mins)
                        {
                            mins = zs;
                            fi = i;
                        }
                    }
                }

                FVector s = getMeshSize(FoliageTypes[fi]->GetSource());

                if (size.X < 0 && s.X > 1e-4)
                    size.X /= -s.X;
                else
                    size.X = 1;
                if (size.Y < 0 && s.Y > 1e-4)
                    size.Y /= -s.Y;
                else
                    size.Y = 1;
                if (size.Z < 0 && s.Z > 1e-4)
                    size.Z /= -s.Z;
                else
                    size.Z = 1;
            }

            pos += rot.RotateVector(FVector(cfg.offset[0], cfg.offset[1], cfg.offset[2]));

            FFoliageInstance instance;
            instance.Location = pos;
            if (cfg.RandRot > 1e-2f)
            {
                float angle = randFloat(-cfg.RandRot, cfg.RandRot);
                rot = UKismetMathLibrary::ComposeRotators(rot, FRotator(0, angle, 0));
            }
            if (cfg.RandAngle > 1e-2f)
            {
                float angle = randFloat(-cfg.RandAngle, cfg.RandAngle);
                rot = UKismetMathLibrary::ComposeRotators(rot, FRotator(angle, 0, 0));
            }
            FRotator to(0, cfg.Turnover, 0);
            rot = FRotator(rot.Quaternion() * to.Quaternion());
            instance.Rotation = rot;
            // instance.PreAlignRotation;
            if (cfg.RandSize > 0.01f)
            {
                float s = randFloat(1.f / (cfg.RandSize + 1), cfg.RandSize + 1);
                size *= s;
            }

            size.X *= cfg.scale[0];
            size.Y *= cfg.scale[1];
            size.Z *= cfg.scale[2];
            instance.DrawScale3D = size;
            instance.ZOffset = 0.f;
            // instance.Flags;
            // Normalize the position

            MeshInfos[fi]->AddInstance(IFA, FoliageTypes[fi], instance);
        }
    return true;
}

bool FPlacement::SpawnActorAsset(const FString& Folder, const FString& Name, const TArray<FAssetData>& InAssets,
    const Config& cfg, const TArray<SPosData>& poss, const TMap<FString, AActor*>& existNames, bool* cancel)
{
    if (cancel)
    {
        *cancel = false;
    }
    UWorld* World = GWorld;
    ULevel* DesiredLevel = GWorld->GetCurrentLevel();

    TArray<UActorFactory*> Factorys;
    TArray<UObject*> InAssets2;
    for (const auto& InAsset : InAssets)
    {
        UObject* Asset = LoadObject<UObject>(NULL, *InAsset.ObjectPath.ToString());
        UActorFactory* Factory = FActorFactoryAssetProxy::GetFactoryForAssetObject(Asset);
        if (!Factory)
        {
            FText Message = FText::FromString(TEXT("Object cannot spawn"));
            FMessageDialog::Open(EAppMsgType::Ok, Message);
            continue;
        }
        Factorys.Add(Factory);
        InAssets2.Add(Asset);
    }
    if (0 == Factorys.Num())
        return false;

    GWarn->BeginSlowTask(FText::FromString(TEXT("Add_Point_Actor")), true, true);

    auto getMeshSize = [&](UObject* InAsset)
    {
        FVector s(0, 0, 0);
        UStaticMesh* sm = Cast<UStaticMesh>(InAsset);
        AStaticMeshActor* sm1 = Cast<AStaticMeshActor>(InAsset);
        UBlueprint* bp = Cast<UBlueprint>(InAsset);
        if (sm)
        {
            s = FRotator(0, cfg.Turnover, 0).RotateVector(sm->GetBoundingBox().GetSize());
        }
        else if (sm1 && sm1->GetStaticMeshComponent() && sm1->GetStaticMeshComponent()->GetStaticMesh())
        {
            s = FRotator(0, cfg.Turnover, 0)
                    .RotateVector(sm1->GetStaticMeshComponent()->GetStaticMesh()->GetBoundingBox().GetSize());
        }
        else if (bp)
        {
            AStaticMeshActor* actor = Cast<AStaticMeshActor>(bp->GeneratedClass->GetDefaultObject());
            if (actor)
            {
                s = FRotator(0, cfg.Turnover, 0)
                        .RotateVector(actor->GetStaticMeshComponent()->GetStaticMesh()->GetBoundingBox().GetSize());
            }
        }
        s.X *= cfg.scale[0];
        s.Y *= cfg.scale[1];
        s.Z *= cfg.scale[2];
        return s;
    };

    int32 cc = 0;
    TMap<int, int> addNames;
    for (const auto& pa : poss)
    {
        FVector pos = pa.pos;
        int nametail = 0;
        if (auto fd = addNames.Find(pa.id))
        {
            nametail = ++(*fd);
        }
        else
        {
            addNames.Add(pa.id) = 0;
        }

        FString actorname = Name + TEXT("_") + FString::FromInt(pa.id) + TEXT("_") + FString::FromInt(nametail);
        FString fullname = Folder / actorname;
        if (auto obj = existNames.Find(fullname))
        {
            AActor* actor = *obj;
            // traffic light
            if (actor && actor->GetClass() && actor->GetClass()->GetSuperClass() &&
                actor->GetClass()->GetSuperClass()->GetName() == TEXT("TrafficLight"))
            {
                if (pa.userdata.Find("signaler_id"))
                {
                    FStrProperty* prot =
                        CastField<FStrProperty>(actor->GetClass()->GetSuperClass()->FindPropertyByName("JunctionID"));
                    if (prot)
                    {
                        uint8* value = prot->ContainerPtrToValuePtr<uint8>(actor);
                        prot->SetPropertyValue(value, pa.userdata["signaler_id"]);
                    }
                }
                if (pa.userdata.Find("heading"))
                {
                    FStrProperty* prot =
                        CastField<FStrProperty>(actor->GetClass()->GetSuperClass()->FindPropertyByName("HeadingID"));
                    if (prot)
                    {
                        uint8* value = prot->ContainerPtrToValuePtr<uint8>(actor);
                        prot->SetPropertyValue(value, pa.userdata["heading"]);
                    }
                }
                if (pa.userdata.Find("turn_k"))
                {
                    FStrProperty* prot =
                        CastField<FStrProperty>(actor->GetClass()->GetSuperClass()->FindPropertyByName("TurnID"));
                    if (prot)
                    {
                        uint8* value = prot->ContainerPtrToValuePtr<uint8>(actor);
                        prot->SetPropertyValue(value, pa.userdata["turn_k"]);
                    }
                }
                FStrProperty* prot =
                    CastField<FStrProperty>(actor->GetClass()->GetSuperClass()->FindPropertyByName("StyleID"));
                if (prot)
                {
                    uint8* value = prot->ContainerPtrToValuePtr<uint8>(actor);
                    FString tpid = Name.Right(3);
                    if (tpid == "100" || tpid == "110" || tpid == "121")
                    {
                        prot->SetPropertyValue(value, "0");
                    }
                    else if (tpid == "101" || tpid == "111" || tpid == "122")
                    {
                        prot->SetPropertyValue(value, "1");
                    }
                    else if (tpid == "102" || tpid == "112" || tpid == "123")
                    {
                        prot->SetPropertyValue(value, "3");
                    }
                    else if (tpid == "103" || tpid == "113" || tpid == "124")
                    {
                        prot->SetPropertyValue(value, "2");
                    }
                    else if (tpid == "104" || tpid == "114" || tpid == "125")
                    {
                        prot->SetPropertyValue(value, "5");
                    }
                    else if (tpid == "131" || tpid == "132")
                    {
                        prot->SetPropertyValue(value, "6");
                    }
                }
            }

            continue;
            // actorname = MakeUniqueObjectName(DesiredLevel, InAssets2[fi]->GetClass(), *(actorname +
            // TEXT("_"))).ToString();
        }

        if (cfg.detect_building)
        {
            FVector Start = pos + FVector::UpVector * 100000;
            FVector End = pos - FVector::UpVector * 100000;

            FHitResult Result;
            bool bHit = World->LineTraceSingleByChannel(Result, Start, End, ECollisionChannel::ECC_GameTraceChannel3);
            if (bHit)
            {
                continue;
            }
        }

        FRotator rot = pa.rot;
        FVector size = pa.size;

        int32 fi = FMath::Rand() % Factorys.Num();
        if (size.X < 0 || size.Y < 0 || size.Z < 0)
        {
            if (Factorys.Num() > 1)
            {
                float mins = FLT_MAX;
                for (int32 i = 0; i < Factorys.Num(); i++)
                {
                    FVector s = getMeshSize(Factorys[i]);
                    if (s.Z < 1e-4)
                    {
                        continue;
                    }
                    float zs = -size.Z / s.Z;
                    if (zs < 1.f)
                    {
                        zs = 1.0f / zs;
                    }
                    if (zs < mins)
                    {
                        mins = zs;
                        fi = i;
                    }
                }
            }

            auto InAsset = InAssets2[fi];
            FVector s = getMeshSize(InAsset);

            if (size.X < 0 && s.X > 1e-4)
                size.X /= -s.X;
            else
                size.X = 1;
            if (size.Y < 0 && s.Y > 1e-4)
                size.Y /= -s.Y;
            else
                size.Y = 1;
            if (size.Z < 0 && s.Z > 1e-4)
                size.Z /= -s.Z;
            else
                size.Z = 1;
        }
        pos += rot.RotateVector(FVector(cfg.offset[0], cfg.offset[1], cfg.offset[2]));
        if (cfg.RandRot > 1e-2f)
        {
            float angle = randFloat(-cfg.RandRot, cfg.RandRot);
            rot = UKismetMathLibrary::ComposeRotators(rot, FRotator(0, angle, 0));
        }
        if (cfg.RandAngle > 1e-2f)
        {
            float angle = randFloat(-cfg.RandAngle, cfg.RandAngle);
            rot = UKismetMathLibrary::ComposeRotators(rot, FRotator(angle, 0, 0));
        }
        // instance.PreAlignRotation;
        if (cfg.RandSize > 0.01f)
        {
            float s = randFloat(1.f / (cfg.RandSize + 1), cfg.RandSize + 1);
            size *= s;
        }
        size.X *= cfg.scale[0];
        size.Y *= cfg.scale[1];
        size.Z *= cfg.scale[2];
        FRotator to(0, cfg.Turnover, 0);
        rot = FRotator(rot.Quaternion() * to.Quaternion());

        // AActor *actor = FActorFactoryAssetProxy::AddActorForAsset(InAsset, false, RF_Transactional, af);
        FTransform ActorTransform(rot, pos);
        FActorSpawnParameters SpawnParams;
        SpawnParams.ObjectFlags = RF_Transactional;
        SpawnParams.Name = *actorname;
        SpawnParams.NameMode = FActorSpawnParameters::ESpawnActorNameMode::Requested;
        AActor* actor = Factorys[fi]->CreateActor(InAssets2[fi], DesiredLevel, ActorTransform, SpawnParams);
        if (actor)
        {
            if (UPrimitiveComponent* PrimitiveComponent = actor->FindComponentByClass<UPrimitiveComponent>())
            {
                PrimitiveComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                PrimitiveComponent->Mobility = EComponentMobility::Static;
            }
            // traffic light
            if (actor->GetClass() && actor->GetClass()->GetSuperClass() &&
                actor->GetClass()->GetSuperClass()->GetName() == TEXT("TrafficLight"))
            {
                if (pa.userdata.Find("signaler_id"))
                {
                    FStrProperty* prot =
                        CastField<FStrProperty>(actor->GetClass()->GetSuperClass()->FindPropertyByName("JunctionID"));
                    if (prot)
                    {
                        uint8* value = prot->ContainerPtrToValuePtr<uint8>(actor);
                        prot->SetPropertyValue(value, pa.userdata["signaler_id"]);
                    }
                }
                if (pa.userdata.Find("heading"))
                {
                    FStrProperty* prot =
                        CastField<FStrProperty>(actor->GetClass()->GetSuperClass()->FindPropertyByName("HeadingID"));
                    if (prot)
                    {
                        uint8* value = prot->ContainerPtrToValuePtr<uint8>(actor);
                        prot->SetPropertyValue(value, pa.userdata["heading"]);
                    }
                }
                if (pa.userdata.Find("turn_k"))
                {
                    FStrProperty* prot =
                        CastField<FStrProperty>(actor->GetClass()->GetSuperClass()->FindPropertyByName("TurnID"));
                    if (prot)
                    {
                        uint8* value = prot->ContainerPtrToValuePtr<uint8>(actor);
                        prot->SetPropertyValue(value, pa.userdata["turn_k"]);
                    }
                }
                FStrProperty* prot =
                    CastField<FStrProperty>(actor->GetClass()->GetSuperClass()->FindPropertyByName("StyleID"));
                if (prot)
                {
                    uint8* value = prot->ContainerPtrToValuePtr<uint8>(actor);
                    FString tpid = Name.Right(3);
                    if (tpid == "100" || tpid == "110" || tpid == "121")
                    {
                        prot->SetPropertyValue(value, "0");
                    }
                    else if (tpid == "101" || tpid == "111" || tpid == "122")
                    {
                        prot->SetPropertyValue(value, "1");
                    }
                    else if (tpid == "102" || tpid == "112" || tpid == "123")
                    {
                        prot->SetPropertyValue(value, "3");
                    }
                    else if (tpid == "103" || tpid == "113" || tpid == "124")
                    {
                        prot->SetPropertyValue(value, "2");
                    }
                    else if (tpid == "104" || tpid == "114" || tpid == "125")
                    {
                        prot->SetPropertyValue(value, "5");
                    }
                    else if (tpid == "131" || tpid == "132")
                    {
                        prot->SetPropertyValue(value, "6");
                    }
                }
            }

            int sidx = 0;
            actor->SetActorScale3D(size);
            actor->SetActorLabel(actorname);
            actor->GetRootComponent()->Mobility = EComponentMobility::Static;
            actor->SetFolderPath_Recursively(*Folder);
            actor->InvalidateLightingCache();
            actor->PostEditChange();
            actor->MarkPackageDirty();
        }

        GWarn->UpdateProgress(cc++, poss.Num());

        if (GWarn->ReceivedUserCancel())
        {
            GWarn->EndSlowTask();
            if (cancel)
            {
                *cancel = true;
            }
            return true;
        }
    }

    GWarn->EndSlowTask();
    return true;
}

bool FPlacement::Line2Pos(const Config& cfg, const SLineData& line, TArray<SPosData>& pos, float Al)
{
    pos.SetNum(0);
    if (line.line.Num() < 2)
    {
        return false;
    }

    TArray<float> terrain_z;
    TArray<FVector> samedis_line;
    {
        FVector p0 = line.line[0];
        samedis_line.Add(p0);

        for (int i = 1; i < line.line.Num(); i++)
        {
            FVector p = line.line[i];
            float dis = FVector::Distance(p, p0);
            if (dis < Al)
            {
                continue;
            }
            FVector n = p - p0;
            n.Normalize();
            FVector np = p0 + n * Al;
            samedis_line.Add(np);
            i--;
            p0 = np;
        }
        if (FVector::Distance(p0, line.line.Last()) > 1)
        {
            samedis_line.Add(line.line.Last());
        }

        float dh0 = FLT_MAX;
        for (const auto& p : samedis_line)
        {
            float Z = FLT_MAX;
            float dh = cfg.PosTop ? (dh0 == FLT_MAX ? cfg.PinRadius : dh0) : 0;
            FVector Start = p + FVector::UpVector * cfg.PinRadius;
            FVector End = p - FVector::UpVector * (cfg.PinRadius + dh);

            FHitResult Result;
            bool bHit = GWorld->LineTraceSingleByChannel(Result, Start, End, ECollisionChannel::ECC_GameTraceChannel2);
            if (cfg.pinRoad > 0.001)
            {
                FVector rightn = p - p0;
                Swap(rightn.X, rightn.Y);
                rightn.X = -rightn.X;
                rightn.Z = 0;
                rightn.Normalize();
                Start = p + FVector::UpVector * cfg.PinRadius + rightn * cfg.pinRoad;
                End = p - FVector::UpVector * (cfg.PinRadius + dh) + rightn * cfg.pinRoad;
                FHitResult Result2;
                bool bHit2 =
                    GWorld->LineTraceSingleByChannel(Result2, Start, End, ECollisionChannel::ECC_GameTraceChannel2);
                if (bHit2)
                {
                    if (bHit)
                    {
                        if ((Result.ImpactPoint.Z - Result2.ImpactPoint.Z) < -10)
                        {
                            Result.ImpactPoint.Z = Result2.ImpactPoint.Z;
                        }
                    }
                    else
                    {
                        Result.ImpactPoint = Result2.ImpactPoint - rightn * cfg.pinRoad;
                        bHit = bHit2;
                    }
                }
                Start = p + FVector::UpVector * cfg.PinRadius - rightn * cfg.pinRoad;
                End = p - FVector::UpVector * (cfg.PinRadius + dh) - rightn * cfg.pinRoad;
                bHit2 = GWorld->LineTraceSingleByChannel(Result2, Start, End, ECollisionChannel::ECC_GameTraceChannel2);
                if (bHit2)
                {
                    if ((Result.ImpactPoint.Z - Result2.ImpactPoint.Z) < -10)
                    {
                        Result.ImpactPoint.Z = Result2.ImpactPoint.Z;
                    }
                }
            }
            if (bHit)
            {
                if (cfg.PosTop && dh0 == FLT_MAX)
                    dh0 = std::max(10.f, p.Z - Result.ImpactPoint.Z);
                Z = Result.ImpactPoint.Z;
            }

            terrain_z.Add(Z);
        }
    }
    {
        TArray<float> terrain_zB = terrain_z;
        for (int i = 0; i < terrain_zB.Num(); i++)
        {
            if (terrain_zB[i] != FLT_MAX)
            {
                continue;
            }
            float a = i > 0 ? terrain_zB[i - 1] : FLT_MAX;
            float b = i < (terrain_zB.Num() - 1) ? terrain_zB[i + 1] : FLT_MAX;
            if (a != FLT_MAX && b != FLT_MAX)
            {
                terrain_z[i] = (a + b) * 0.5;
            }
            else if (a != FLT_MAX)
            {
                terrain_z[i] = a;
            }
            else if (b != FLT_MAX)
            {
                terrain_z[i] = b;
            }
        }
    }
    for (int i = 1; i < samedis_line.Num(); i++)
    {
        if (terrain_z[i] == FLT_MAX || terrain_z[i - 1] == FLT_MAX)
        {
            continue;
        }
        SPosData sp;

        FVector p0 = samedis_line[i - 1];
        FVector p = samedis_line[i];
        if (cfg.PosTop)
        {
            float dh0 = p0.Z - terrain_z[i - 1];
            float dh = p.Z - terrain_z[i];
            sp.size.Z = -FMath::Max(dh0, dh);
        }
        if (cfg.PinTerrain || cfg.PosTop)
        {
            p0.Z = terrain_z[i - 1];
            p.Z = terrain_z[i];
        }
        float dis = FVector::Distance(p, p0);

        sp.pos = (p + p0) * 0.5;
        sp.rot.Pitch = FMath::RadiansToDegrees(FMath::Asin((p.Z - p0.Z) / dis));
        sp.rot.Yaw = FMath::RadiansToDegrees(FMath::Atan2(p.Y - p0.Y, p.X - p0.X));
        sp.size.X = dis / Al;

        pos.Add(sp);
    }

    return true;
}

bool FPlacement::Polgyon2Pos(
    const Config& cfg, const SPolygonData& ply, TArray<SPosData>& pos, const TArray<float>& area)
{
    if (ply.outline.Num() < 2 || area.Num() == 0)
    {
        return false;
    }
    double sumarea = 0;
    for (const auto& a : area)
    {
        sumarea += a;
    }
    if (sumarea < 0.001)
        return false;
    sumarea /= area.Num();
    TArray<FVector> rawPots;
    using Point = std::array<float, 2>;
    std::vector<std::vector<Point>> polygon(1);
    polygon.back().resize(ply.outline.Num());
    for (int i = 0; i < ply.outline.Num(); i++)
    {
        polygon.back().at(i) = {ply.outline[i].X, ply.outline[i].Y};
        rawPots.Add(ply.outline[i]);
    }
    for (const auto& hole : ply.holes)
    {
        if (hole.Num() == 0)
            continue;
        polygon.push_back(std::vector<Point>());
        polygon.back().resize(hole.Num());
        for (int i = 0; i < hole.Num(); i++)
        {
            polygon.back().at(i) = {hole[i].X, hole[i].Y};
            rawPots.Add(hole[i]);
        }
    }
    std::vector<int> mPositionsIndex;
    mPositionsIndex = mapbox::earcut<int>(polygon);
    for (size_t i = 0, is = mPositionsIndex.size() / 3; i < is; i++)
    {
        const FVector& p1 = rawPots[mPositionsIndex[i * 3]];
        const FVector& p2 = rawPots[mPositionsIndex[i * 3 + 1]];
        const FVector& p3 = rawPots[mPositionsIndex[i * 3 + 2]];
        double tarea = (p1.X * (p2.Y - p3.Y) + p2.X * (p3.Y - p1.Y) + p3.X * (p1.Y - p2.Y)) * 0.5;    // ;
        tarea /= sumarea * sumarea;
        int32 ptn = FMath::CeilToInt(tarea);
        int32 rdt = FMath::CeilToInt(1.0 / tarea);

        for (int j = 0; j < ptn; j++)
        {
            if (i != is / 2 && FMath::RandRange(0, rdt - 1) != 0)
            {
                continue;
            }

            float r1 = FMath::Sqrt(FMath::RandRange(0.f, 1.f));
            float r2 = FMath::RandRange(0.f, 1.f);
            float m1 = 1 - r1;
            float m2 = r1 * (1 - r2);
            float m3 = r2 * r1;

            FVector p = (m1 * p1) + (m2 * p2) + (m3 * p3);
            if (!isInBox(p))
                continue;
            SPosData sp;
            sp.id = ply.id;
            sp.pos = p;
            sp.rot = FRotator(0);

            if (cfg.PinTerrain)
            {
                UWorld* World = GWorld;
                FVector Start = p + FVector::UpVector * cfg.PinRadius;
                FVector End = p - FVector::UpVector * (cfg.PinRadius);

                FHitResult Result;
                bool bHit =
                    World->LineTraceSingleByChannel(Result, Start, End, ECollisionChannel::ECC_GameTraceChannel2);
                if (bHit)
                {
                    p = Result.ImpactPoint;
                }
            }

            sp.pos = p;

            pos.Add(sp);
        }
    }
    if (pos.Num() == 0)
    {
        double sx = 0, sy = 0, sz = 0;
        for (const auto& p : ply.outline)
        {
            sx += p.X;
            sy += p.Y;
            sz += p.Z;
        }
        SPosData sp;
        sp.id = pos.Num();
        sp.pos = FVector(sx / ply.outline.Num(), sy / ply.outline.Num(), sz / ply.outline.Num());
        if (isInBox(sp.pos))
        {
            if (cfg.PinTerrain)
            {
                UWorld* World = GWorld;
                FVector Start = sp.pos + FVector::UpVector * cfg.PinRadius;
                FVector End = sp.pos - FVector::UpVector * (cfg.PinRadius);

                FHitResult Result;
                bool bHit =
                    World->LineTraceSingleByChannel(Result, Start, End, ECollisionChannel::ECC_GameTraceChannel2);
                if (bHit)
                {
                    sp.pos = Result.ImpactPoint;
                }
            }
            sp.rot = FRotator(0);
            pos.Add(sp);
        }
    }

    return true;
}
