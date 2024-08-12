#include "TrafficSign.h"
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
#include "ActorFactories/ActorFactory.h"
#include "Mercator.h"

bool TrafficSign::ParseText(const FString& fname, const FString& s, TArray<TArray<FString>>& sdat)
{
    FString Data;
    if (FFileHelper::LoadFileToString(Data, *fname))
    {
        FString Left;
        FString Right;
        while (Data.Split(TEXT("\r\n"), &Left, &Right))
        {
            if (!Left.IsEmpty())
            {
                FString LeftStr;
                FString RightStr;

                TArray<FString> dbs;
                while (Left.Split(s, &LeftStr, &RightStr))
                {
                    if (!LeftStr.IsEmpty())
                    {
                        dbs.Add(LeftStr);
                    }
                    Left = RightStr;
                }
                if (!Left.IsEmpty())
                {
                    dbs.Add(Left);
                }
                sdat.Add(dbs);
            }
            Data = Right;
        }
        if (!Data.IsEmpty())
        {
            Left = Data;
            FString LeftStr;
            FString RightStr;

            TArray<FString> dbs;
            while (Left.Split(s, &LeftStr, &RightStr))
            {
                if (!LeftStr.IsEmpty())
                {
                    dbs.Add(LeftStr);
                }
                Left = RightStr;
            }
            if (!Left.IsEmpty())
            {
                dbs.Add(Left);
            }
            sdat.Add(dbs);
        }

        return true;
    }
    return false;
}

bool TrafficSign::isInBox(const FVector& p)
{
    return p.X >= Box0.X && p.X < Box1.X && p.Y >= Box0.Y && p.Y < Box1.Y;
}
bool TrafficSign::LoadData(const TArray<FString>& fnames)
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
                FString dir = FPaths::GetPath(*fname);
                auto jasset = json->GetArrayField(TEXT("Signs"));
                for (const auto& jass : jasset)
                {
                    auto jassobj = jass->AsObject();
                    Sign s;
                    s.type = jassobj->GetStringField(TEXT("category_id"));

                    s.id = FString::FromInt(jassobj->GetIntegerField(TEXT("id")));
                    auto jpoint = jassobj->GetArrayField(TEXT("pos"));
                    if (jpoint.Num() == 3)
                    {
                        s.pos.X = jpoint[0]->AsNumber() - tcen.first;
                        s.pos.Y = -jpoint[1]->AsNumber() + tcen.second;
                        s.pos.Z = jpoint[2]->AsNumber();
                        s.pos *= 100;
                    }
                    auto jrpy = jassobj->GetArrayField(TEXT("rpy"));
                    if (jrpy.Num() == 3)
                    {
                        s.rot.Yaw = -FMath::RadiansToDegrees(jrpy[2]->AsNumber());
                        s.rot.Pitch = FMath::RadiansToDegrees(jrpy[1]->AsNumber());
                        s.rot.Roll = -FMath::RadiansToDegrees(jrpy[0]->AsNumber());
                    }
                    s.width = jassobj->GetNumberField(TEXT("width")) * 100;
                    s.height = jassobj->GetNumberField(TEXT("height")) * 100;
                    s.thickness = 1;
                    int32 lid = 0;
                    jassobj->TryGetNumberField(TEXT("link_to"), lid);
                    s.pid = FString::FromInt(lid);
                    if (lid > 0 && pole)
                    {
                        if (!pole->Has(s.pid))
                        {
                            continue;
                        }
                        pole->Pin(s.pos, s.rot.Yaw, s.pid, FVector(s.width, FMath::Max(s.width, s.height), s.height),
                            FMath::Min(s.width, s.height) * 0.5);
                    }
                    else if (!isInBox(s.pos))
                        continue;

                    FString imgPath = jassobj->GetStringField(TEXT("image_path"));
                    if (s.type == TEXT("999") || s.type == TEXT("0"))
                    {
                        if (imgPath.IsEmpty())
                        {
                            FString imgp = FPaths::Combine(dir, TEXT("tex"), s.id);
                            if (FPaths::FileExists(imgp + TEXT(".png")))
                            {
                                imgPath = imgp + TEXT(".png");
                            }
                            else if (FPaths::FileExists(imgp + TEXT(".jpg")))
                            {
                                imgPath = imgp + TEXT(".jpg");
                            }
                            else if (FPaths::FileExists(imgp + TEXT(".bmp")))
                            {
                                imgPath = imgp + TEXT(".bmp");
                            }
                            // imgPath = TEXT("F:/tmp/G7bzp/G7_bzp_000006-01.bmp");
                        }
                        signs.FindOrAdd(imgPath).Add(s);
                    }
                    else
                    {
                        signs.FindOrAdd(TEXT("##")).Add(s);
                    }
                }
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Can`t read json: %s"), *Reader->GetErrorMessage());
                if (EAppReturnType::Cancel ==
                    FMessageDialog::Open(EAppMsgType::OkCancel,
                        FText::FromString(TEXT("Can`t read json: ") + Reader->GetErrorMessage())))
                    return false;
            }
        }
    }
    return true;
}
bool TrafficSign::LoadConfig()
{
    FString jpath = FPaths::ProjectConfigDir() + TEXT("sign.json");

    FString jbuf;
    if (FFileHelper::LoadFileToString(jbuf, *jpath))
    {
        TSharedPtr<FJsonObject> json;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(jbuf);
        if (FJsonSerializer::Deserialize(Reader, json))
        {
            if (json->GetStringField(TEXT("version")) == TEXT("0.1.0"))
            {
                auto jsign = json->GetObjectField(TEXT("sign"));
                for (const auto& v : jsign->Values)
                {
                    signres.Add(v.Key) = v.Value->AsString();
                }
                /*auto jsten = json->GetObjectField(TEXT("stencil"));
                int default_value = jsten->GetIntegerField("default");
                for (int i = 0; i < 256; i++)
                {
                    stencils.Add(FString::FromInt(i)) = default_value;
                }
                for (const auto& v : jsign->Values)
                {
                    if (v.Key == "default")
                    {
                        continue;
                    }
                    int value = FCString::Atoi(*v.Key);
                    for (const auto& id : v.Value->AsArray())
                    {
                        stencils.Add(id->AsString()) = value;
                    }
                }*/
            }
            return true;
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Can`t read json: %s"), *Reader->GetErrorMessage());
            if (EAppReturnType::Cancel ==
                FMessageDialog::Open(EAppMsgType::OkCancel,
                    FText::FromString(TEXT("Can`t read config json: ") + Reader->GetErrorMessage())))
                return false;
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("Read sign config falid."));
    return false;
}

void TrafficSign::CreateAndPlace(const TArray<FString>& path)
{
    if (!LoadConfig())
        return;

    if (!LoadData(path))
    {
        FText Message = FText::FromString(TEXT("Load data faild."));
        FMessageDialog::Open(EAppMsgType::Ok, Message);
        return;
    }

    int falidnum = 0;
    // setp1 create normal map and save
    TArray<FString> Files;
    for (const auto& sign : signs)
    {
        if (sign.Key == TEXT("##"))
        {
            continue;
        }

        FString PathPart;
        FString FilenamePart;
        FString ExtensionPart;
        FPaths::Split(sign.Key, PathPart, FilenamePart, ExtensionPart);
        FString norName = PathPart + TEXT("/") + FilenamePart + TEXT("_nor.bmp");
        if (!FPaths::FileExists(norName))
        {
            if (!CreateNormal(sign.Key, norName))
            {
                UE_LOG(LogTemp, Warning, TEXT("Cannot create normal map : %s"), *norName);
                FMessageDialog::Open(
                    EAppMsgType::OkCancel, FText::FromString(TEXT("Cannot create normal map: ") + norName));

                falidnum++;
                continue;
            }
        }

        Files.Add(sign.Key);
        Files.Add(norName);
    }
    CreateTextures(Files);

    // STEP2 create mesh and actor
    TArray<UPackage*> PackagesToSave;
    TArray<UPackage*> PackagesToReloads;
    {
        UMaterialInstance* matrial =
            LoadObject<UMaterialInstance>(nullptr, TEXT("/Game/3DRtest/AutoTemplate/TrafficSignBacktexture"));
        // FStringAssetReference
        // assetPath(TEXT(
        //  "MaterialInstanceConstant'/Game/3DRtest/AutoTemplate/
        // TrafficSignBacktexture.TrafficSignBacktexture'"
        // ));
        // UObject* tpt = assetPath.TryLoad();
        UStaticMesh* smesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/3DRtest/AutoTemplate/TrafficSignMesh"));
        UBlueprint* bp = LoadObject<UBlueprint>(nullptr, TEXT("/Game/3DRtest/AutoTemplate/TrafficSignMeshBP"));
        for (const auto& sign : signs)
        {
            if (sign.Key.IsEmpty() || sign.Key == TEXT("##"))
            {
                continue;
            }

            FString FilenamePart;
            FString PathPart;
            FString ExtensionPart;
            FPaths::Split(sign.Key, PathPart, FilenamePart, ExtensionPart);
            FString matName = FilenamePart + TEXT("_Mat");
            FString meshName = FilenamePart + TEXT("_Mesh");
            FString actorName = FilenamePart + TEXT("_Bp");

            UObject* matObj = nullptr;
            UObject* actorObj = LoadObject<UBlueprint>(nullptr, *(PackageDir + actorName));
            if (!actorObj)
            {
                UObject* imgObj = LoadObject<UTexture2D>(nullptr, *(PackageDir + FilenamePart));
                if (!imgObj)
                {
                    UE_LOG(LogTemp, Warning, TEXT("Cannot LoadObject : %s"), *FilenamePart);
                    if (EAppReturnType::Cancel == FMessageDialog::Open(EAppMsgType::OkCancel,
                                                      FText::FromString(TEXT("Cannot LoadObject: ") + FilenamePart)))
                        return;
                    continue;
                }
                UObject* norObj = LoadObject<UTexture2D>(nullptr, *(PackageDir + FilenamePart + TEXT("_nor")));
                if (!norObj)
                {
                    UE_LOG(LogTemp, Warning, TEXT("Cannot LoadObject : %s_nor"), *FilenamePart);
                    if (EAppReturnType::Cancel == FMessageDialog::Open(EAppMsgType::OkCancel,
                                                      FText::FromString(TEXT("Cannot LoadObject: ") + FilenamePart)))
                        return;
                    continue;
                }
                matObj = CreateMatrial(matName, imgObj, norObj, matrial);
                UObject* meshObj = CreateMesh(meshName, matObj, smesh);
                actorObj = CreateMeshBp(actorName, meshObj, bp);
                if (!actorObj)
                {
                    UE_LOG(LogTemp, Warning, TEXT("Cannot CreateMeshBp : %s"), *actorName);
                    if (EAppReturnType::Cancel == FMessageDialog::Open(EAppMsgType::OkCancel,
                                                      FText::FromString(TEXT("Cannot CreateMeshBp: ") + actorName)))
                        return;
                    continue;
                }
                PackagesToSave.Add(matObj->GetOutermost());
                PackagesToSave.Add(meshObj->GetOutermost());
                PackagesToSave.Add(actorObj->GetOutermost());
                PackagesToReloads.Add(matObj->GetOutermost());
            }
        }
    }
    FEditorFileUtils::PromptForCheckoutAndSave(PackagesToSave, true, false);
    PackagesToSave.Empty();

    // setp3 reload mat
    {
        TArray<FReloadPackageData> InPackagesToReload;
        for (int i = 0; i < PackagesToReloads.Num(); i++)
        {
            InPackagesToReload.Add(FReloadPackageData(PackagesToReloads[i], 0));
        }
        TArray<UPackage*> ReloadedPackages;
        ReloadPackages(TArrayView<FReloadPackageData>(InPackagesToReload.GetData(), InPackagesToReload.Num()),
            ReloadedPackages, InPackagesToReload.Num());
    }
    PackagesToReloads.Empty();

    TSet<FString> existNames;
    for (TActorIterator<AActor> ActorItr(GWorld); ActorItr; ++ActorItr)
    {
        FString path_ = ActorItr->GetFolderPath().ToString();
        FString name = ActorItr->GetName();
        existNames.Add(path_ / name);
    }

    // step4 spawn actor
    TMap<FString, int> addNames;
    for (const auto& sns : signs)
    {
        if (sns.Key == TEXT("##"))
        {
            for (const auto& sn : sns.Value)
            {
                SpawnActorAsset(sn, existNames, addNames);
            }
        }
        else
        {
            UObject* actorObj = nullptr;
            if (sns.Key.IsEmpty())
            {
                actorObj = LoadObject<UBlueprint>(nullptr, TEXT("/Game/3DRtest/AutoTemplate/TrafficSignDefault"));
            }
            else
            {
                FString PathPart;
                FString FilenamePart;
                FString ExtensionPart;
                FPaths::Split(sns.Key, PathPart, FilenamePart, ExtensionPart);
                FString actorName = FilenamePart + TEXT("_Bp");
                actorObj = LoadObject<UBlueprint>(nullptr, *(PackageDir + actorName));
            }
            if (!actorObj)
            {
                UE_LOG(LogTemp, Warning, TEXT("Cannot LoadObject : %s"), *sns.Key);
                if (EAppReturnType::Cancel == FMessageDialog::Open(EAppMsgType::OkCancel,
                                                  FText::FromString(TEXT("Cannot LoadObject: ") + sns.Key)))
                    return;
                continue;
            }

            for (const auto& sn : sns.Value)
            {
                SpawnActorAsset(actorObj, sn, existNames, addNames);
            }
        }
    }
    if (falidnum > 0)
    {
        FText Message = FText::Format(
            FText::FromString(TEXT("{0} TrafficSign faild.")), FText::FromString(FString::FromInt(falidnum)));
        FMessageDialog::Open(EAppMsgType::Ok, Message);
    }
}

UObject* TrafficSign::CreateTexture(const FString& fimg)
{
    FAssetToolsModule& AssetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");
    auto TextureFact = NewObject<UTextureFactory>();
    TArray<FString> Files;
    Files.Add(fimg);
    auto nass = AssetToolsModule.Get().ImportAssets(Files, PackageDir, TextureFact);
    if (nass.Num() == 0)
    {
        return nullptr;
    }
    TArray<UPackage*> PackagesToSave;
    PackagesToSave.Add(nass[0]->GetOutermost());
    if (FEditorFileUtils::PR_Success != FEditorFileUtils::PromptForCheckoutAndSave(PackagesToSave, true, false))
        return nullptr;

    return nass[0];
}

void TrafficSign::CreateTextures(const TArray<FString>& Files)
{
    FAssetToolsModule& AssetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");
    auto TextureFact = NewObject<UTextureFactory>();
    auto nass = AssetToolsModule.Get().ImportAssets(Files, PackageDir, TextureFact);
    TArray<UPackage*> PackagesToSave;
    for (int i = 0; i < nass.Num(); i++)
    {
        PackagesToSave.Add(nass[i]->GetOutermost());
    }
    FEditorFileUtils::PromptForCheckoutAndSave(PackagesToSave, true, false);
}

UObject* TrafficSign::CreateMatrial(const FString& outFname, UObject* img, UObject* nor, UObject* templateObj)
{
    UTexture2D* textureImg = Cast<UTexture2D>(img);
    if (textureImg == nullptr)
        return nullptr;
    UTexture2D* textureNor = Cast<UTexture2D>(nor);
    if (textureNor == nullptr)
        return nullptr;
    UMaterialInstance* matrial = Cast<UMaterialInstance>(templateObj);
    if (!matrial)
        return nullptr;
    FString NewPackageName = PackageDir + outFname;
    const FString NewAssetName = FPackageName::GetLongPackageAssetName(NewPackageName);
    UPackage* Package = CreatePackage(*NewPackageName);
    UObject* DuplicatedAsset = StaticDuplicateObject(matrial, Package, *NewAssetName);
    FAssetRegistryModule::AssetCreated(DuplicatedAsset);
    matrial = Cast<UMaterialInstance>(DuplicatedAsset);
    matrial->TextureParameterValues[0].ParameterValue = textureImg;
    matrial->TextureParameterValues[1].ParameterValue = textureNor;

    matrial->RecacheUniformExpressions(true);
    DuplicatedAsset->MarkPackageDirty();

    //    FString PackageFileName = FPackageName::LongPackageNameToFilename(NewPackageName,
    //    FPackageName::GetAssetPackageExtension()); UPackage::SavePackage(Package, DuplicatedAsset,
    //    EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *PackageFileName);

    return DuplicatedAsset;
}

bool TrafficSign::CreateAtlas(const TArray<FString>& fimg, const FString& outFname)
{
    return false;
}

bool TrafficSign::CreateNormal(const FString& fimg, const FString& outFname)
{
    IImageWrapperModule& ImageWrapperModule =
        FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
    TSharedPtr<IImageWrapper> ImageWrapper;
    if (fimg.Right(3).ToLower() == TEXT("png"))
    {
        ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);
    }
    else if (fimg.Right(3).ToLower() == TEXT("jpg"))
    {
        ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::JPEG);
    }
    else if (fimg.Right(3).ToLower() == TEXT("bmp"))
    {
        ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::BMP);
    }
    else
        return false;

    // Load From File
    TArray<uint8> RawFileData;
    if (!FFileHelper::LoadFileToArray(RawFileData, *fimg))
    {
        return false;
    }
    // Create T2D!
    if (!ImageWrapper.IsValid() || !ImageWrapper->SetCompressed(RawFileData.GetData(), RawFileData.Num()))
    {
        return false;
    }

    TArray<uint8> imageOri;
    if (!ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, imageOri))
    {
        return false;
    }
    int wid = ImageWrapper->GetWidth();
    int hei = ImageWrapper->GetHeight();
    check(imageOri.Num() == wid * hei * 4);
    TArray<int> gray;
    gray.SetNum(wid * hei);

    for (int i = 0; i < hei; i++)
    {
        for (int j = 0; j < wid; j++)
        {
            int c = i * wid * 4 + j * 4;
            gray[i * wid + j] = FMath::Min(
                255, FMath::Max(0, (int) (0.2126 * imageOri[c + 2] + 0.7152 * imageOri[c + 1] + 0.0722 * imageOri[c])));
        }
    }

    TArray<FColor> imageDest;
    imageDest.SetNum(wid * hei);
    for (int i = 0; i < hei; i++)
    {
        for (int j = 0; j < wid; j++)
        {
            imageDest[i * wid + j] = FColor(0, 0, 255, 255);
        }
    }
    for (int i = 1; i < hei - 1; i++)
    {
        for (int j = 1; j < wid - 1; j++)
        {
            auto& c = imageDest[i * wid + j];
            int dy = (int) gray[(i - 1) * wid + j] - (int) gray[(i + 1) * wid + j];
            int dx = (int) gray[i * wid + j - 1] - (int) gray[i * wid + j + 1];
            int dz = 127;
            int base = FMath::CeilToInt(FMath::Sqrt(dx * dx + dy * dy + dz * dz));
            c.R = dx * 127 / base + 127;
            c.G = dy * 127 / base + 127;
            c.B = dz * 255 / base;
        }
    }
    if (!FFileHelper::CreateBitmap(*outFname, wid, hei, imageDest.GetData()))
    {
        return false;
    }

    return true;
}

UObject* TrafficSign::CreateMesh(const FString& outFname, UObject* mat, UObject* templateObj)
{
    UMaterialInstance* material = Cast<UMaterialInstance>(mat);
    if (material == nullptr)
    {
        return nullptr;
    }
    UStaticMesh* smesh = Cast<UStaticMesh>(templateObj);
    if (!smesh)
        return nullptr;

    FString NewPackageName = PackageDir + outFname;
    const FString NewAssetName = FPackageName::GetLongPackageAssetName(NewPackageName);
    UPackage* Package = CreatePackage(*NewPackageName);
    UObject* DuplicatedAsset = StaticDuplicateObject(smesh, Package, *NewAssetName);
    FAssetRegistryModule::AssetCreated(DuplicatedAsset);
    smesh = Cast<UStaticMesh>(DuplicatedAsset);

    smesh->GetStaticMaterials()[0] = FStaticMaterial(material);
    smesh->UpdateUVChannelData(false);
    DuplicatedAsset->MarkPackageDirty();
    return DuplicatedAsset;
}

UObject* TrafficSign::CreateMeshBp(const FString& outFname, UObject* mesh, UObject* templateObj)
{
    UStaticMesh* umesh = Cast<UStaticMesh>(mesh);
    if (umesh == nullptr)
        return nullptr;
    UBlueprint* bp = Cast<UBlueprint>(templateObj);
    if (!bp)
        return nullptr;

    FString NewPackageName = PackageDir + outFname;
    const FString NewAssetName = FPackageName::GetLongPackageAssetName(NewPackageName);
    UPackage* Package = CreatePackage(*NewPackageName);
    UObject* DuplicatedAsset = StaticDuplicateObject(bp, Package, *NewAssetName);
    FAssetRegistryModule::AssetCreated(DuplicatedAsset);
    bp = Cast<UBlueprint>(DuplicatedAsset);
    AStaticMeshActor* actor = Cast<AStaticMeshActor>(bp->GeneratedClass->GetDefaultObject());
    if (!actor)
        return nullptr;

    actor->GetStaticMeshComponent()->SetStaticMesh(umesh);
    actor->GetStaticMeshComponent()->bRenderCustomDepth = 1;
    actor->GetStaticMeshComponent()->CustomDepthStencilWriteMask = ERendererStencilMask::ERSM_255;
    actor->GetStaticMeshComponent()->CustomDepthStencilValue = 26;
    actor->MarkPackageDirty();

    DuplicatedAsset->MarkPackageDirty();
    return DuplicatedAsset;
}

void TrafficSign::SpawnActorAsset(
    UObject* obj, const Sign& s, const TSet<FString>& existNames, TMap<FString, int>& addNames)
{
    FName dir(WorldDir + TEXT("/AutoTrafficSign"));
    FString actorname = TEXT("TrafficSign_") + s.id + TEXT("_") + s.type;
    int nametail = 0;
    if (auto fd = addNames.Find(actorname))
    {
        nametail = ++(*fd);
    }
    else
    {
        addNames.Add(actorname) = 0;
    }
    actorname += "_" + FString::FromInt(nametail);
    ULevel* DesiredLevel = GWorld->GetCurrentLevel();
    FString fullname = dir.ToString() / actorname;
    if (existNames.Contains(fullname))
    {
        UE_LOG(LogTemp, Warning, TEXT("The path has exist : %s"), *fullname);
        // if (EAppReturnType::Cancel == FMessageDialog::Open(EAppMsgType::OkCancel, FText::FromString(TEXT("The path
        // has exist: ") + fullname)))
        //     return;
        return;
    }
    UActorFactory* factory = FActorFactoryAssetProxy::GetFactoryForAssetObject(obj);
    if (!factory)
    {
        UE_LOG(LogTemp, Warning, TEXT("GetFactoryForAssetObject faild : %s"), *fullname);
        if (EAppReturnType::Cancel == FMessageDialog::Open(EAppMsgType::OkCancel,
                                          FText::FromString(TEXT("GetFactoryForAssetObject faild: ") + fullname)))
            return;
        return;
    }
    FTransform ActorTransform(s.rot, s.pos);
    FActorSpawnParameters SpawnParams;
    SpawnParams.ObjectFlags = RF_Transactional;
    SpawnParams.Name = *actorname;
    SpawnParams.NameMode = FActorSpawnParameters::ESpawnActorNameMode::Requested;
    AActor* actor = factory->CreateActor(obj, DesiredLevel, ActorTransform, SpawnParams);
    actor->SetActorLabel(actorname);
    actor->SetActorLocation(s.pos);
    actor->SetActorRotation(s.rot);
    actor->SetActorScale3D(FVector(s.thickness, s.width * 0.01, s.height * 0.01));
    actor->GetRootComponent()->Mobility = EComponentMobility::Static;
    if (UPrimitiveComponent* PrimitiveComponent = actor->FindComponentByClass<UPrimitiveComponent>())
    {
        PrimitiveComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        PrimitiveComponent->Mobility = EComponentMobility::Static;
    }
    FActorFolders::Get().CreateFolder(*actor->GetWorld(), dir);
    actor->SetFolderPath_Recursively(dir);
}

void TrafficSign::SpawnActorAsset(const Sign& s, const TSet<FString>& existNames, TMap<FString, int>& addNames)
{
    FName dir(WorldDir + TEXT("/AutoTrafficSign"));
    if (!signres.Find(s.type))
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot find sign type : %s"), *s.type);
        if (EAppReturnType::Cancel ==
            FMessageDialog::Open(EAppMsgType::OkCancel, FText::FromString(TEXT("Cannot find sign type: ") + s.type)))
            return;
        return;
    }
    FString actorname = TEXT("TrafficSign_") + s.id + TEXT("_") + s.type;
    int nametail = 0;
    if (auto fd = addNames.Find(actorname))
    {
        nametail = ++(*fd);
    }
    else
    {
        addNames.Add(actorname) = 0;
    }
    actorname += "_" + FString::FromInt(nametail);
    ULevel* DesiredLevel = GWorld->GetCurrentLevel();
    FString fullname = dir.ToString() / actorname;
    if (existNames.Contains(fullname))
    {
        UE_LOG(LogTemp, Warning, TEXT("The path has exist : %s"), *fullname);
        return;
    }

    UObject* obj = LoadObject<UObject>(nullptr, *signres[s.type]);
    if (!obj)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot load sign type : %s"), *signres[s.type]);
        if (EAppReturnType::Cancel == FMessageDialog::Open(EAppMsgType::OkCancel,
                                          FText::FromString(TEXT("Cannot load sign type: ") + signres[s.type])))
            return;
        return;
    }
    UActorFactory* factory = FActorFactoryAssetProxy::GetFactoryForAssetObject(obj);
    if (!factory)
    {
        UE_LOG(LogTemp, Warning, TEXT("GetFactoryForAssetObject faild : %s"), *fullname);
        if (EAppReturnType::Cancel == FMessageDialog::Open(EAppMsgType::OkCancel,
                                          FText::FromString(TEXT("GetFactoryForAssetObject faild: ") + fullname)))
            return;
        return;
    }
    FTransform ActorTransform(s.rot, s.pos);
    FActorSpawnParameters SpawnParams;
    SpawnParams.ObjectFlags = RF_Transactional;
    SpawnParams.Name = *actorname;
    SpawnParams.NameMode = FActorSpawnParameters::ESpawnActorNameMode::Requested;
    AActor* actor = factory->CreateActor(obj, DesiredLevel, ActorTransform, SpawnParams);
    actor->SetActorLabel(actorname);
    actor->SetActorLocation(s.pos);
    actor->GetRootComponent()->Mobility = EComponentMobility::Static;
    FRotator rot = s.rot;
    rot.Yaw -= 90;
    actor->SetActorRotation(rot);
    actor->SetActorScale3D(FVector(s.thickness, s.width * 0.01, s.height * 0.01));
    if (UPrimitiveComponent* PrimitiveComponent = actor->FindComponentByClass<UPrimitiveComponent>())
    {
        PrimitiveComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        PrimitiveComponent->Mobility = EComponentMobility::Static;
    }
    FActorFolders::Get().CreateFolder(*actor->GetWorld(), dir);
    actor->SetFolderPath_Recursively(dir);
}
