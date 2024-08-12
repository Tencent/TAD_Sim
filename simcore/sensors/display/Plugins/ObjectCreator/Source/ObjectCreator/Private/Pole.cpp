#include "Pole.h"
// #include "StaticMeshAttributes.h"
#include "MeshAttributes.h"
#include "UObject/Package.h"
#include "Misc/FileHelper.h"
#include "EngineUtils.h"
#include "IAssetTools.h"
#include "AssetToolsModule.h"
#include "ObjectTools.h"
#include "Misc/Paths.h"
#include "FileHelpers.h"
#include "UObject/UObjectGlobals.h"
#include "Misc/MessageDialog.h"
#include "EditorActorFolders.h"
#include "AssetSelection.h"
#include "Internationalization/Text.h"
#include "UObject/PackageReload.h"
#include "Developer/DesktopPlatform/Public/IDesktopPlatform.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"
#include "Misc/FeedbackContext.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "AssetRegistryModule.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Editor.h"
#include <functional>
#include "MeshUtilities.h"
#include "ActorFactories/ActorFactory.h"
#include "Mercator.h"

#define BINNING_H 10.0
#define BINNING_W 10.0

bool Pole::isInBox(const FVector& p)
{
    return p.X >= Box0.X && p.X < Box1.X && p.Y >= Box0.Y && p.Y < Box1.Y;
}
bool Pole::LoadData(const TArray<FString>& fnames)
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
                int32 num = 0;
                auto jasset = json->GetArrayField(TEXT("Poles"));
                for (const auto& jass : jasset)
                {
                    auto jassobj = jass->AsObject();
                    FString type = jassobj->GetStringField(TEXT("category_id"));
                    double width = jassobj->GetNumberField(TEXT("width")) * 100;
                    double height = jassobj->GetNumberField(TEXT("height")) * 100;
                    auto jpoint = jassobj->GetArrayField(TEXT("pos"));
                    if (jpoint.Num() != 3)
                    {
                        UE_LOG(LogTemp, Warning, TEXT("Can`t read pos: %d"), jassobj->GetIntegerField(TEXT("id")));
                        if (EAppReturnType::Cancel ==
                            FMessageDialog::Open(EAppMsgType::OkCancel,
                                FText::FromString(TEXT("Pole Cannot read pos: ") +
                                                  FString::FromInt(jassobj->GetIntegerField(TEXT("id"))))))
                            return false;
                        continue;
                    }
                    const auto comp = poleComponent.Find(type);
                    bool pin_height = comp && comp->pin_height;

                    FVector pos;
                    pos.X = jpoint[0]->AsNumber() - tcen.first;
                    pos.Y = -jpoint[1]->AsNumber() + tcen.second;
                    pos.Z = jpoint[2]->AsNumber();
                    pos *= 100;
                    pos.Z -= height;

                    if (!isInBox(pos))
                        continue;

                    float z_adjust = 0;
                    if (true)
                    {
                        FVector tp = pos;
                        tp.Z += height;

                        float pinr = 3000;
                        FVector End = tp - FVector::UpVector * pinr;

                        FHitResult Result;
                        bool bHit =
                            GWorld->LineTraceSingleByChannel(Result, tp, End, ECollisionChannel::ECC_GameTraceChannel2);
                        if (bHit)
                        {
                            z_adjust = Result.ImpactPoint.Z - pos.Z;
                            if (pin_height)
                                height += pos.Z - Result.ImpactPoint.Z;
                            pos = Result.ImpactPoint;
                        }
                    }
                    double binning_vert = BINNING_H, binning_hori = BINNING_W;
                    if (comp)
                    {
                        binning_hori = FMath::Max(comp->hori_rounding, BINNING_W);
                        binning_vert = FMath::Max(comp->vert_rounding, BINNING_H);
                        if (comp->horizontal.Num() == 0)
                            width = 0;
                        if (comp->vertical.Num() == 0)
                            height = 0;
                    }

                    int binning_w = FMath::RoundToInt(width / binning_hori);
                    int binning_h = FMath::RoundToInt(height / binning_vert);
                    width = binning_w * binning_hori;
                    height = binning_h * binning_vert;
                    FString gid = type + TEXT("_") + FString::FromInt(width) + TEXT("_") + FString::FromInt(height);
                    Para* fd = poles.FindByKey<FString>(gid);
                    if (fd == nullptr)
                    {
                        poles.AddDefaulted();
                        fd = &poles.Last();
                    }
                    fd->gid = gid;
                    fd->type = type;
                    fd->width = width;
                    fd->height = height;

                    Pos s;
                    s.id = FString::FromInt(jassobj->GetIntegerField(TEXT("id")));
                    s.pos = pos;
                    s.z_adjust = z_adjust;
                    auto jrpy = jassobj->GetArrayField(TEXT("rpy"));
                    if (jrpy.Num() == 3)
                    {
                        s.yaw = 270 - FMath::RadiansToDegrees(jrpy[2]->AsNumber());
                    }
                    fd->pos.Add(s);
                    num++;
                }
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Can`t read json: %s"), *Reader->GetErrorMessage());
                FMessageDialog::Open(
                    EAppMsgType::OkCancel, FText::FromString(TEXT("Cannot read json: ") + Reader->GetErrorMessage()));
            }
        }
    }
    return true;
}

void Pole::CreateAndPlace(const TArray<FString>& path)
{
    if (!LoadConfig(false))
        return;
    if (!LoadData(path))
        return;

    FString PackageDir = TEXT("/Game/3DRtest/Auto_out/pole/");
    TArray<UPackage*> PackagesToSave;

    TArray<FString> package_paths;
    package_paths.SetNum(poles.Num());
    for (int32 i = 0; i < poles.Num(); i++)
    {
        const auto& pole = poles[i];
        const auto comp = poleComponent.Find(pole.type);
        if (comp)
        {
            if (comp->pluginmode.IsEmpty())
            {
                FString name = comp->name + TEXT("-") + pole.gid;
                FString NewPackageName = PackageDir + name;
                package_paths[i] = NewPackageName;

                UObject* poleObj = CreatePole(*comp, pole.width, pole.height, NewPackageName);
                if (poleObj)
                    PackagesToSave.Add(poleObj->GetOutermost());
            }
            else
            {
                package_paths[i] = comp->pluginmode;
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Unknow pole type: %s."), *pole.type);
            if (EAppReturnType::Cancel ==
                FMessageDialog::Open(EAppMsgType::OkCancel, FText::FromString(TEXT("Unknow pole type: ") + pole.type)))
                return;
        }
    }
    FEditorFileUtils::PromptForCheckoutAndSave(PackagesToSave, true, false);
    PackagesToSave.Empty();

    FString dir = WorldDir;
    FActorFolders::Get().CreateFolder(*GWorld, *dir);
    dir += TEXT("/Pole");
    FActorFolders::Get().CreateFolder(*GWorld, *dir);
    dir += TEXT("/");
    TSet<FString> existNames;
    for (TActorIterator<AActor> ActorItr(GWorld); ActorItr; ++ActorItr)
    {
        FString path_ = ActorItr->GetFolderPath().ToString();
        FString name = ActorItr->GetName();
        existNames.Add(path_ / name);
    }

    auto spawn = [&](UObject* obj, Pos& pos, const FName& Name = NAME_None, const FName& Dir = NAME_None,
                     bool pluginmode = false, float height = 0)
    {
        FName actorname = Name;
        ULevel* DesiredLevel = GWorld->GetCurrentLevel();

        FString fullname = Dir.ToString() / Name.ToString();
        if (existNames.Contains(fullname))
        {
            // UE_LOG(LogTemp, Warning, TEXT("The path has exist : %s"), *fullname);
            return;
        }
        if (pluginmode)
        {
            float pinr = 600;
            FVector Start = pos.pos + FVector::UpVector * pinr;
            FVector End = pos.pos - FVector::UpVector * pinr;

            FHitResult Result;
            bool bHit = GWorld->LineTraceSingleByChannel(Result, Start, End, ECollisionChannel::ECC_GameTraceChannel2);
            if (bHit)
            {
                pos.pos = Result.ImpactPoint;
            }
            UBlueprint* bp = Cast<UBlueprint>(obj);
            AStaticMeshActor* actor = Cast<AStaticMeshActor>(bp->GeneratedClass->GetDefaultObject());
            if (actor)
            {
                auto s = actor->GetStaticMeshComponent()->GetStaticMesh()->GetBoundingBox().GetSize();
                float hdown = FMath::Max(0.f, s.Z - height);
                pos.pos.Z -= hdown;
            }
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
        FTransform ActorTransform(FRotator(0, pos.yaw, 0), pos.pos);
        FActorSpawnParameters SpawnParams;
        SpawnParams.ObjectFlags = RF_Transactional;
        SpawnParams.Name = actorname;
        SpawnParams.NameMode = FActorSpawnParameters::ESpawnActorNameMode::Requested;
        AActor* actor = factory->CreateActor(obj, DesiredLevel, ActorTransform, SpawnParams);

        actor->SetActorLabel(actorname.ToString());
        actor->SetActorLocation(pos.pos);
        actor->SetActorRotation(FRotator(0, pos.yaw, 0));
        actor->GetRootComponent()->Mobility = EComponentMobility::Static;
        if (UPrimitiveComponent* PrimitiveComponent = actor->FindComponentByClass<UPrimitiveComponent>())
        {
            PrimitiveComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            PrimitiveComponent->Mobility = EComponentMobility::Static;
        }
        actor->SetFolderPath_Recursively(Dir);
    };

    TMap<FString, int> addNames;
    for (int32 i = 0; i < poles.Num(); i++)
    {
        auto& pole = poles[i];
        const auto comp = poleComponent.Find(pole.type);
        if (!comp)
            continue;
        UObject* actorObj = LoadObject<UBlueprint>(nullptr, *(package_paths[i]));
        if (!actorObj)
            continue;
        FString name = comp->name;
        FString dirpath = dir + name;
        FActorFolders::Get().CreateFolder(*GWorld, *dirpath);

        for (int32 j = 0; j < pole.pos.Num(); j++)
        {
            if (!isInBox(pole.pos[j].pos))
            {
                continue;
            }
            FString actorname = name + TEXT("_") + pole.pos[j].id + TEXT("_") + pole.type;
            int nametail = 0;
            if (auto fd = addNames.Find(actorname))
            {
                nametail = ++(*fd);
            }
            else
            {
                addNames.Add(actorname) = 0;
            }

            spawn(actorObj, pole.pos[j], FName(*(actorname + "_" + FString::FromInt(nametail))), FName(*dirpath),
                !comp->pluginmode.IsEmpty(), pole.height);
        }
    }
}

UObject* Pole::CreatePole(const Component& comp, float width, float height, const FString& NewPackageName)
{
    UBlueprint* NewBlueprint = LoadObject<UBlueprint>(nullptr, *NewPackageName);
    if (NewBlueprint)
        return nullptr;

    /*UBlueprint* bp = LoadObject<UBlueprint>(nullptr, TEXT("/Game/AutoLib/Template/pole"));
    check(bp);

    const FString NewAssetName = FPackageName::GetLongPackageAssetName(NewPackageName);
    UPackage* Package = CreatePackage(nullptr, *NewPackageName);
    UObject* DuplicatedAsset = StaticDuplicateObject(bp, Package, *NewAssetName);
    FAssetRegistryModule::AssetCreated(DuplicatedAsset);
    bp = Cast<UBlueprint>(DuplicatedAsset);*/

    AActor* Actor =
        NewObject<AActor>(GetTransientPackage(), TEXT("ROOT"), RF_Transactional | RF_Public | RF_Standalone);

    auto spawn = [&](const FString& objName, const FVector& pos, const FRotator& rot, const FVector& scale,
                     const FName& Name = NAME_None) -> bool
    {
        UObject* objt = FStringAssetReference(*objName).TryLoad();
        UStaticMesh* mesh = Cast<UStaticMesh>(objt);
        if (mesh == nullptr)
            return false;
        UStaticMeshComponent* NewComponent = NewObject<UStaticMeshComponent>(
            Actor, UStaticMeshComponent::StaticClass(), Name, RF_Transactional | RF_Public | RF_Standalone);

        NewComponent->SetStaticMesh(mesh);
        NewComponent->SetRelativeLocation(pos);
        NewComponent->SetRelativeRotation(rot);
        NewComponent->SetRelativeScale3D(scale);
        NewComponent->bRenderCustomDepth = 1;
        NewComponent->CustomDepthStencilWriteMask = ERendererStencilMask::ERSM_255;
        NewComponent->CustomDepthStencilValue = comp.stencil;
        NewComponent->Mobility = EComponentMobility::Static;

        Actor->AddInstanceComponent(NewComponent);
        return true;
    };

    if (height > 1e-6f && comp.vertical.Num() > 0)
    {
        for (int32 v = 0; v < comp.vertical.Num(); ++v)
        {
            const auto& ver = comp.vertical[v];
            TArray<FVector> poss;
            poss.Add(FVector(0));
            if (comp.gantry)
            {
                poss.Add(FVector(width, 0, 0));
            }
            float adjscale = ver.sol > 1 ? FMath::Max(1.0, height / ver.sol) : 1;

            TArray<FVector> aposs;
            TArray<float> alens;
            for (int32 i = 0; i < poss.Num(); ++i)
            {
                const auto& pos = poss[i];
                if (ver.repeat)
                {
                    int32 k = FMath::Max(1, FMath::RoundToInt(height / ver.len));
                    float ks = height / k;
                    float s = ks / ver.len;
                    for (int32 j = 0; j < k; j++)
                    {
                        aposs.Add(pos + FVector(ver.ox, ver.oy, ks * j));
                        alens.Add(s + 0.01f);
                    }
                }
                else
                {
                    aposs.Add(pos + FVector(ver.ox, ver.oy, 0));
                    alens.Add(height / ver.len);
                }
                for (int32 k = 0; k < ver.objects.Num(); k++)
                {
                    const auto& obj = ver.objects[k];
                    float z = (obj.p > 1e-6f ? 0 : height) + obj.p;
                    spawn(obj.path, pos + FVector(obj.ox, obj.oy, obj.oz + z), FRotator(0, obj.yaw, 0),
                        FVector(obj.sx * adjscale, obj.sy * adjscale, obj.sz),
                        *(TEXT("VO_") + FString::FromInt(v) + TEXT("_") + FString::FromInt(i) + TEXT("_") +
                            FString::FromInt(k)));
                }
            }

            for (int32 k = 0; k < aposs.Num(); k++)
            {
                FVector scale(ver.sx * adjscale, ver.sy * adjscale, alens[k]);
                spawn(ver.path, aposs[k], FRotator(0), scale,
                    *(TEXT("V_") + FString::FromInt(v) + TEXT("_") + FString::FromInt(k)));
            }
        }
    }
    if (width > 1e-6f && comp.horizontal.Num() > 0)
    {
        FVector p0(0);
        FVector nor(1, 0, 0);
        for (int32 j = 0; j < comp.horizontal.Num(); ++j)
        {
            const auto& hor = comp.horizontal[j];
            TArray<FVector> aposs;
            TArray<float> alens;
            float z = (hor.z > 1e-6f ? 0 : height) + hor.z;
            if (hor.repeat)
            {
                int32 ks = FMath::Max(1, FMath::RoundToInt(width / hor.len));
                float v = width / ks;
                float s = v / hor.len;
                for (int32 k = 0; k < ks; k++)
                {
                    aposs.Add(p0 + nor * v * k + FVector(0, hor.oy, z));
                    alens.Add(s + 0.01f);
                }
            }
            else
            {
                aposs.Add(p0 + FVector(0, hor.oy, z));
                alens.Add(width / hor.len);
            }
            float adjscale = hor.sol > 1 ? FMath::Max(1.0, width / hor.sol) : 1;

            for (int32 k = 0; k < aposs.Num(); k++)
            {
                spawn(hor.path, aposs[k], FRotator(0, hor.yaw, 0),
                    FVector(alens[k], hor.sy * adjscale, hor.sz * adjscale),
                    *(TEXT("H_") + FString::FromInt(j) + TEXT("_") + FString::FromInt(k)));
            }
            for (int32 k = 0; k < hor.objects.Num(); k++)
            {
                const auto& obj = hor.objects[k];
                float a = (obj.p > 1e-6f ? 0 : width) + obj.p;
                spawn(obj.path, nor * a + FVector(obj.ox, obj.oy, obj.oz), FRotator(0, hor.yaw + obj.yaw, 0),
                    FVector(obj.sx, obj.sy * adjscale, obj.sz * adjscale),
                    *(TEXT("HO_") + FString::FromInt(j) + TEXT("_") + FString::FromInt(k)));
            }
        }
    }

    // UBlueprint* bp = FKismetEditorUtilities::CreateBlueprintFromActor(NewPackageName, Actor, false);
    // copy from
    if (UPackage* Package = CreatePackage(*NewPackageName))
    {
        FString AssetName = FPackageName::GetLongPackageAssetName(NewPackageName);
        NewBlueprint = FKismetEditorUtilities::CreateBlueprint(Actor->GetClass(), Package, *AssetName,
            EBlueprintType::BPTYPE_Normal, UBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass(),
            FName("CreateFromAutoPole"));
        if (NewBlueprint != nullptr)
        {
            // Notify the asset registry
            FAssetRegistryModule::AssetCreated(NewBlueprint);

            // Mark the package dirty
            Package->MarkPackageDirty();

            // If the source Actor has Instance Components we need to translate these in to SCS Nodes
            if (Actor->GetInstanceComponents().Num() > 0)
            {
                FKismetEditorUtilities::FAddComponentsToBlueprintParams Params;
                Params.HarvestMode = FKismetEditorUtilities::EAddComponentToBPHarvestMode::None;
                Params.OptionalNewRootNode = nullptr;
                Params.bKeepMobility = false;
                FKismetEditorUtilities::AddComponentsToBlueprint(NewBlueprint, Actor->GetInstanceComponents(), Params);
            }
            if (NewBlueprint->GeneratedClass != nullptr)
            {
                AActor* CDO = CastChecked<AActor>(NewBlueprint->GeneratedClass->GetDefaultObject());
                const auto CopyOptions = (EditorUtilities::ECopyOptions::Type)(
                    EditorUtilities::ECopyOptions::OnlyCopyEditOrInterpProperties |
                    EditorUtilities::ECopyOptions::PropagateChangesToArchetypeInstances);
                EditorUtilities::CopyActorProperties(Actor, CDO, CopyOptions);
            }
            FKismetEditorUtilities::CompileBlueprint(NewBlueprint);
        }
    }

    FAssetRegistryModule::AssetCreated(NewBlueprint);
    return NewBlueprint;
}

bool Pole::LoadConfig(bool sub_category)
{
    FString jpath = FPaths::ProjectConfigDir() + TEXT("pole.json");

    FString jbuf;
    if (FFileHelper::LoadFileToString(jbuf, *jpath))
    {
        TSharedPtr<FJsonObject> json;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(jbuf);
        if (FJsonSerializer::Deserialize(Reader, json))
        {
            if (json->GetStringField(TEXT("version")) == TEXT("0.1.0"))
            {
                auto jpoles = json->GetArrayField(TEXT("poles"));
                for (const auto& jpole : jpoles)
                {
                    auto jpobject = jpole->AsObject();
                    FString id = jpobject->GetStringField(TEXT("id"));
                    Component& comp = poleComponent.Add(id);
                    comp.stencil = sub_category ? FCString::Atoi(*id) : 27;
                    jpobject->TryGetStringField(TEXT("name"), comp.name);
                    jpobject->TryGetStringField(TEXT("plugin_mode"), comp.pluginmode);
                    jpobject->TryGetBoolField(TEXT("gantry"), comp.gantry);
                    jpobject->TryGetBoolField(TEXT("pin_height"), comp.pin_height);
                    jpobject->TryGetNumberField(TEXT("vert_rounding"), comp.vert_rounding);
                    comp.vert_rounding *= 100;
                    jpobject->TryGetNumberField(TEXT("hori_rounding"), comp.hori_rounding);
                    comp.hori_rounding *= 100;
                    const TArray<TSharedPtr<FJsonValue>>* jhorarr;
                    if (jpobject->TryGetArrayField(TEXT("vertical"), jhorarr))
                    {
                        for (const auto& jsub : *jhorarr)
                        {
                            auto jver = jsub->AsObject();
                            Component::Vertical ver;
                            jver->TryGetNumberField(TEXT("len"), ver.len);
                            ver.len *= 100;
                            jver->TryGetBoolField(TEXT("repeat"), ver.repeat);
                            jver->TryGetStringField(TEXT("path"), ver.path);
                            jver->TryGetNumberField(TEXT("offx"), ver.ox);
                            ver.ox *= 100;
                            jver->TryGetNumberField(TEXT("offy"), ver.oy);
                            ver.oy *= 100;
                            jver->TryGetNumberField(TEXT("sx"), ver.sx);
                            jver->TryGetNumberField(TEXT("sy"), ver.sy);
                            jver->TryGetNumberField(TEXT("sol"), ver.sol);
                            ver.sol *= 100;
                            jver->TryGetNumberField(TEXT("ra"), ver.ra);
                            ver.ra *= 100;
                            jver->TryGetNumberField(TEXT("rb"), ver.rb);
                            ver.rb *= 100;

                            const TArray<TSharedPtr<FJsonValue>>* jsubarr;
                            if (jver->TryGetArrayField(TEXT("object"), jsubarr))
                            {
                                for (const auto& jobj : *jsubarr)
                                {
                                    auto jobjobj = jobj->AsObject();
                                    Component::Object obj;
                                    jobjobj->TryGetNumberField(TEXT("p"), obj.p);
                                    obj.p *= 100;
                                    jobjobj->TryGetNumberField(TEXT("yaw"), obj.yaw);
                                    obj.yaw *= -1;
                                    jobjobj->TryGetNumberField(TEXT("offx"), obj.ox);
                                    obj.ox *= 100;
                                    jobjobj->TryGetNumberField(TEXT("offy"), obj.oy);
                                    obj.oy *= 100;
                                    jobjobj->TryGetNumberField(TEXT("offz"), obj.oz);
                                    obj.oz *= 100;
                                    jobjobj->TryGetNumberField(TEXT("sx"), obj.sx);
                                    jobjobj->TryGetNumberField(TEXT("sy"), obj.sy);
                                    jobjobj->TryGetNumberField(TEXT("sz"), obj.sz);
                                    jobjobj->TryGetStringField(TEXT("path"), obj.path);
                                    ver.objects.Add(obj);
                                }
                            }
                            comp.vertical.Add(ver);
                        }
                    }
                    if (jpobject->TryGetArrayField(TEXT("horizontal"), jhorarr))
                    {
                        for (const auto& jsub : *jhorarr)
                        {
                            auto jhor = jsub->AsObject();
                            Component::Horizontal hor;
                            jhor->TryGetNumberField(TEXT("len"), hor.len);
                            hor.len *= 100;
                            jhor->TryGetBoolField(TEXT("repeat"), hor.repeat);
                            jhor->TryGetStringField(TEXT("path"), hor.path);
                            jhor->TryGetNumberField(TEXT("z"), hor.z);
                            hor.z *= 100;
                            jhor->TryGetNumberField(TEXT("sy"), hor.sy);
                            jhor->TryGetNumberField(TEXT("sz"), hor.sz);
                            jhor->TryGetNumberField(TEXT("sol"), hor.sol);
                            hor.sol *= 100;
                            jhor->TryGetNumberField(TEXT("yaw"), hor.yaw);
                            hor.yaw *= -1;
                            jhor->TryGetNumberField(TEXT("offy"), hor.oy);
                            hor.oy *= 100;
                            jhor->TryGetNumberField(TEXT("ra"), hor.ra);
                            hor.ra *= 100;
                            jhor->TryGetNumberField(TEXT("rb"), hor.rb);
                            hor.rb *= 100;

                            const TArray<TSharedPtr<FJsonValue>>* jsubarr;
                            if (jhor->TryGetArrayField(TEXT("object"), jsubarr))
                            {
                                for (const auto& jobj : *jsubarr)
                                {
                                    auto jobjobj = jobj->AsObject();
                                    Component::Object obj;
                                    jobjobj->TryGetNumberField(TEXT("p"), obj.p);
                                    obj.p *= 100;
                                    jobjobj->TryGetNumberField(TEXT("yaw"), obj.yaw);
                                    obj.yaw *= -1;
                                    jobjobj->TryGetNumberField(TEXT("offx"), obj.ox);
                                    obj.ox *= 100;
                                    jobjobj->TryGetNumberField(TEXT("offy"), obj.oy);
                                    obj.oy *= 100;
                                    jobjobj->TryGetNumberField(TEXT("offz"), obj.oz);
                                    obj.oz *= 100;
                                    jobjobj->TryGetNumberField(TEXT("sx"), obj.sx);
                                    jobjobj->TryGetNumberField(TEXT("sy"), obj.sy);
                                    jobjobj->TryGetNumberField(TEXT("sz"), obj.sz);
                                    jobjobj->TryGetStringField(TEXT("path"), obj.path);
                                    hor.objects.Add(obj);
                                }
                            }
                            comp.horizontal.Add(hor);
                        }
                    }
                }
            }
            return true;
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Can`t read json: %s"), *Reader->GetErrorMessage());
            FMessageDialog::Open(
                EAppMsgType::OkCancel, FText::FromString(TEXT("Can`t read json: ") + Reader->GetErrorMessage()));
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("Read pole config falid."));
    return false;
}

bool Pole::Has(const FString& id)
{
    if (id.IsEmpty())
    {
        return false;
    }
    for (int32 i = 0; i < poles.Num(); i++)
    {
        const auto& pole = poles[i];
        for (int32 j = 0; j < pole.pos.Num(); j++)
        {
            const auto& ppos = pole.pos[j];
            if (ppos.id == id)
            {
                return true;
            }
        }
    }
    return false;
}

bool Pole::Pin(FVector& p, float yaw, const FString& id, const FVector& r, float moveto)
{
    if (id.IsEmpty())
    {
        return false;
    }
    for (int32 i = 0; i < poles.Num(); i++)
    {
        const auto& pole = poles[i];
        if (pole.height < 1e-6)
        {
            continue;
        }
        float width = pole.width;
        for (int32 j = 0; j < pole.pos.Num(); j++)
        {
            const auto& ppos = pole.pos[j];
            if (ppos.id == id)
            {
                FVector lp = p - ppos.pos;
                lp.Z += ppos.z_adjust;
                const auto comp = poleComponent.Find(pole.type);
                if (comp)
                {
                    if (lp.Z < -r.Z || lp.Z > (pole.height + r.Z))
                    {
                        UE_LOG(LogTemp, Warning, TEXT("pole Pin faild: out of range %s."), *pole.gid);
                        p.Z += ppos.z_adjust;
                        return false;
                    }
                    if (FMath::Abs(lp.X) > FMath::Max(pole.width * 3, 2000.f) ||
                        FMath::Abs(lp.Y) > FMath::Max(pole.width * 3, 2000.f))
                    {
                        UE_LOG(LogTemp, Warning, TEXT("pole Pin faild: out of range %s."), *pole.gid);
                        p.Z += ppos.z_adjust;
                        return false;
                    }
                    if (comp->horizontal.Num() == 0)
                    {
                        width = 0;
                    }
                    float pyaw = ppos.yaw;
                    if (width < 1e-6)
                    {
                        pyaw = yaw - 90;
                    }
                    lp = FRotator(0, -pyaw, 0).RotateVector(lp);
                    float deltaX = -FLT_MAX;
                    float deltaY = -FLT_MAX;
                    float deltaZ = -FLT_MAX;
                    yaw -= pyaw;

                    if (width > 1e-6)
                    {
                        float maxz = -FLT_MAX, minz = FLT_MAX;
                        for (int32 k = 0; k < comp->horizontal.Num(); ++k)
                        {
                            const auto& hor = comp->horizontal[k];
                            float z = (hor.z > 1e-6f ? 0 : pole.height) + hor.z;
                            maxz = FMath::Max(maxz, z);
                            minz = FMath::Min(minz, z);
                            if (lp.Z > (z - r.Z) && lp.Z < (z + r.Z))
                            {
                                float x = FMath::Min(FMath::Max(lp.X, 0.f), width);
                                if (hor.repeat)
                                {
                                    x = FMath::Fmod(x, hor.len);
                                }
                                else
                                {
                                    x = x * hor.len / width;
                                }
                                float adjscale = hor.sol > 1 ? FMath::Max(1.0, pole.width / hor.sol) : 1;
                                float y = hor.sy * FMath::Abs(hor.ra * x + hor.rb * adjscale) + FMath::Abs(hor.oy);
                                y += FMath::Abs(FRotator(0, yaw, 0).RotateVector(FVector(r.X, 0, 0)).X);
                                if (FMath::Abs(lp.Y - y) < r.Y)
                                {
                                    deltaY = FMath::Max(deltaY, y);
                                    if (FMath::Abs(lp.Z - (minz + maxz) * 0.5) < moveto)
                                        deltaZ = (minz + maxz) * 0.5;
                                }
                            }
                        }
                    }

                    if (deltaY == -FLT_MAX &&
                        (FMath::Abs(lp.X) < r.X || (comp->gantry && FMath::Abs(lp.X - width) < r.X)))
                    {
                        for (int32 k = 0; k < comp->vertical.Num(); ++k)
                        {
                            const auto& ver = comp->vertical[k];
                            float x = FMath::Min(FMath::Max(lp.Z, 0.f), pole.height);
                            x = x * ver.len / pole.height;
                            float adjscale = ver.sol > 1 ? FMath::Max(1.0, pole.height / ver.sol) : 1;
                            float y = ver.sy * FMath::Abs(ver.ra * x + ver.rb * adjscale) + FMath::Abs(ver.oy);
                            if (FMath::Abs(lp.Y - y) < r.Y && deltaY < y)
                            {
                                deltaY = y;
                                if (FMath::Abs(lp.X - (comp->gantry ? width : 0)) < moveto)
                                    deltaX = comp->gantry ? width : 0;
                            }
                        }
                    }

                    if (FRotator(0, yaw, 0).RotateVector(FVector(1, 0, 0)).Y < 0)
                    {
                        deltaY *= -1;
                    }
                    lp.X = deltaX == -FLT_MAX ? lp.X : deltaX;
                    lp.Y = deltaY == -FLT_MAX ? lp.Y : deltaY;
                    lp.Z = deltaZ == -FLT_MAX ? lp.Z : deltaZ;

                    p = FRotator(0, pyaw, 0).RotateVector(lp) + ppos.pos;
                    return true;
                }
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("pole Pin faild: cannot find."));
    return false;
}
