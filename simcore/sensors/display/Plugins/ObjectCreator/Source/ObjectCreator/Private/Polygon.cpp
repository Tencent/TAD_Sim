#include "Polygon.h"
#include "MeshDescription.h"
// #include "StaticMeshAttributes.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/ObjectLibrary.h"
#include "MeshAttributes.h"
#include "UObject/Package.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Misc/FileHelper.h"
#include "AssetRegistryModule.h"
#include "Materials/MaterialInstance.h"
#include "EngineUtils.h"
#include "Engine/Texture.h"
#include "Engine/StreamableManager.h"
#include "Factories/TextureFactory.h"
#include "IAssetTools.h"
#include "AssetToolsModule.h"
#include "ObjectTools.h"
#include "Misc/Paths.h"
#include "FileHelpers.h"
#include "Engine/StaticMeshActor.h"
#include "UObject/UObjectGlobals.h"
#include "Misc/Paths.h"
#include "Misc/MessageDialog.h"
#include "EditorActorFolders.h"
#include "AssetSelection.h"
#include "Internationalization/Text.h"
#include "UObject/PackageReload.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Developer/DesktopPlatform/Public/IDesktopPlatform.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"
#include "Misc/FeedbackContext.h"
#include "Pole.h"
#include "Polygon.h"

TArray<FVector> Polygon::ShrinkPolygon(const TArray<FVector>& ply, double slen)
{
    TArray<FVector> nply;

    return nply;
}

TArray<FVector> Polygon::PolygonSkeleton(const TArray<FVector>& ply)
{
    TArray<FVector> line;

    return line;
}

TArray<FVector> Polygon::SmoothPolygon(const TArray<FVector>& ply)
{
    TArray<FVector> nply;

    return nply;
}
Polygon::RefLine Polygon::BuildRefline(const TArray<FVector>& line)
{
    RefLine rline;
    rline.line = line;

    return rline;
}

void Polygon::CreateAndPlace(const FString& fname)
{
    if (!LoadConfig())
        return;

    if (!LoadData(fname))
    {
        FText Message = FText::FromString(TEXT("Load data faild."));
        FMessageDialog::Open(EAppMsgType::Ok, Message);
        return;
    }

    for (const auto& ply : plys)
    {
        if (config.Find(ply.type) == nullptr)
        {
            continue;
        }
    }
}

bool Polygon::LoadData(const FString& fname)
{
    FString jbuf;
    if (FFileHelper::LoadFileToString(jbuf, *fname))
    {
        TSharedPtr<FJsonObject> json;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(jbuf);
        if (FJsonSerializer::Deserialize(Reader, json))
        {
            int32 num = 0;
            auto jasset = json->GetArrayField(TEXT("Polygon"));
            for (const auto& jass : jasset)
            {
                auto jassobj = jass->AsObject();
                Ply ply;

                ply.type = FString::FromInt(jassobj->GetIntegerField(TEXT("category_id")));
                ply.id = FString::FromInt(jassobj->GetIntegerField(TEXT("id")));

                auto jpoints = jassobj->GetArrayField(TEXT("points"));
                if (jpoints.Num() < 3)
                {
                    UE_LOG(LogTemp, Warning, TEXT("Can`t read pos: %d"), jassobj->GetIntegerField(TEXT("id")));
                    continue;
                }
                for (size_t i = 0; i < jpoints.Num(); i++)
                {
                    auto jpoint = jpoints[i]->AsArray();
                    if (jpoint.Num() > 1)
                    {
                        FVector pos;
                        pos.X = -jpoint[1]->AsNumber();
                        pos.Y = -jpoint[0]->AsNumber();
                        if (jpoint.Num() > 2)
                        {
                            pos.Z = jpoint[2]->AsNumber();
                        }
                        else
                        {
                            pos.Z = FLT_MAX;
                        }
                        ply.plypos.Add(pos);
                    }
                }
                if (ply.plypos.Num() > 2)
                    plys.Add(ply);
            }
            UE_LOG(LogTemp, Warning, TEXT("Load polygon: %d"), plys.Num());
            return true;
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Can`t read json: %s"), *Reader->GetErrorMessage());
        }
    }

    return false;
}

bool Polygon::LoadConfig()
{
    FString jpath = FPaths::ProjectConfigDir() + TEXT("polygon.json");

    FString jbuf;
    if (FFileHelper::LoadFileToString(jbuf, *jpath))
    {
        TSharedPtr<FJsonObject> json;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(jbuf);
        if (FJsonSerializer::Deserialize(Reader, json))
        {
            if (json->GetStringField(TEXT("version")) == TEXT("0.1.0"))
            {
                for (const auto& va : json->Values)
                {
                    if (va.Key == TEXT("version"))
                    {
                        continue;
                    }

                    auto jobj = va.Value->AsObject();
                    Config cfg;
                    cfg.hole.uvlen = jobj->GetObjectField(TEXT("hole"))->GetNumberField(TEXT("uv_len"));
                    cfg.hole.uvto = jobj->GetObjectField(TEXT("hole"))->GetStringField(TEXT("uv_to")) == "follow_line"
                                        ? Config::FOLLOW_LINE
                                        : Config::FOLLOW_SKELETON;
                    cfg.hole.asset = jobj->GetObjectField(TEXT("hole"))->GetStringField(TEXT("asset"));

                    for (auto& expand : jobj->GetArrayField("shrink"))
                    {
                        Config::UVconfig uv;
                        uv.uvlen = expand->AsObject()->GetNumberField(TEXT("uv_len"));
                        uv.uvto = expand->AsObject()->GetStringField(TEXT("uv_to")) == "follow_line"
                                      ? Config::FOLLOW_LINE
                                      : Config::FOLLOW_SKELETON;
                        uv.asset = expand->AsObject()->GetStringField(TEXT("asset"));
                        double len = expand->AsObject()->GetNumberField(TEXT("len"));
                        cfg.shrink.Add(TPairInitializer<double, Config::UVconfig>(len, uv));
                    }
                }
            }
            return true;
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Can`t read json: %s"), *Reader->GetErrorMessage());
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("Read sign config falid."));
    return false;
}
