// Copyright Epic Games, Inc. All Rights Reserved.

#include "ObjectCreator.h"
#include "ObjectCreatorStyle.h"
#include "ObjectCreatorCommands.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SNumericEntryBox.h"
// #include "ToolMenus.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Developer/DesktopPlatform/Public/IDesktopPlatform.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"
#include "Misc/FeedbackContext.h"
#include "TrafficSign.h"
#include "Pole.h"
#include "Placement.h"

#include "Engine/Selection.h"
#include "Runtime/Foliage/Public/InstancedFoliageActor.h"
#include "Runtime/Foliage/Public/InstancedFoliage.h"
#include "Runtime/Foliage/Public/FoliageType.h"

static const FName ObjectCreatorTabName("ObjectCreator");

#define LOCTEXT_NAMESPACE "FObjectCreatorModule"

struct Functions : TSharedFromThis<Functions>
{
    FString WorldDir = "MsLevel";
    FText GetWorldDir() const
    {
        return FText::FromString(WorldDir);
    }
    void SetWorldDir(const FText& InText, ETextCommit::Type InCommitType)
    {
        WorldDir = InText.ToString();
    }
    FString TrafficSignDir = "/Game/3DRtest/Auto_out/TrafficSign/";
    FText GetTrafficSignDir() const
    {
        return FText::FromString(TrafficSignDir);
    }
    void SetTrafficSignDir(const FText& InText, ETextCommit::Type InCommitType)
    {
        TrafficSignDir = InText.ToString();
    }
    FReply RunTrafficSign()
    {
        IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
        bool bOpened = false;
        TArray<FString> OpenedFiles;
        if (DesktopPlatform)
        {
            /*bOpened = DesktopPlatform->OpenDirectoryDialog(
                FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr),
                NSLOCTEXT("UnrealEd", "ImportTrafficSign", "Import TrafficSign").ToString(),
                TEXT(""),
                OpenedFiles
            );*/

            bOpened =
                DesktopPlatform->OpenFileDialog(FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr),
                    NSLOCTEXT("UnrealEd", "ImportData", "Import Data").ToString(), TEXT(""), TEXT(""),
                    TEXT("All support files(*.json) | *.json"), EFileDialogFlags::Multiple, OpenedFiles);
        }
        if (!bOpened || OpenedFiles.Num() == 0)
        {
            return FReply::Handled();
        }

        Pole pole;
        pole.WorldDir = WorldDir;
        pole.CreateAndPlace(OpenedFiles);

        TrafficSign sign;
        sign.PackageDir = TrafficSignDir;
        sign.WorldDir = WorldDir;
        sign.SetPole(&pole);
        sign.CreateAndPlace(OpenedFiles);

        FPlacement placement;
        placement.WorldDir = WorldDir;
        placement.SetPole(&pole);
        placement.CreateAndPlace(OpenedFiles);

        return FReply::Handled();
    }

    FReply Spec2Foliage()
    {
        UWorld* World = GEditor->GetEditorWorldContext().World();
        AInstancedFoliageActor* IFA = AInstancedFoliageActor::GetInstancedFoliageActorForCurrentLevel(World, true);

        USelection* selection = GEditor->GetSelectedActors();
        int32 succ = 0;
        for (int32 i = 0; i < selection->Num(); i++)
        {
            UObject* obj = selection->GetSelectedObject(i);
            FString name = obj->GetClass()->ClassGeneratedBy->GetFName().ToString();
            if (name != TEXT("BP_Specing_Actor"))
            {
                continue;
            }
            AActor* actor = Cast<AActor>(obj);
            if (!actor)
                continue;

            UFoliageType* FoliageType = nullptr;
            FFoliageInfo* MeshInfo = nullptr;
            TInlineComponentArray<UPrimitiveComponent*> PrimitiveComponents;
            actor->GetComponents(PrimitiveComponents);
            int32 suc = 0;
            for (int32 CompIdx = 0; CompIdx < PrimitiveComponents.Num() - 2; ++CompIdx)
            {
                UPrimitiveComponent* PrimComponent = PrimitiveComponents[CompIdx];
                UStaticMeshComponent* smc = Cast<UStaticMeshComponent>(PrimComponent);
                if (!smc)
                    continue;

                if (!FoliageType)
                {
                    UStaticMesh* mesh = smc->GetStaticMesh();
                    if (mesh)
                    {
                        FoliageType = IFA->GetLocalFoliageTypeForSource(mesh);
                        if (!FoliageType)
                        {
                            IFA->AddMesh(mesh, &FoliageType);
                        }
                        if (FoliageType)
                            MeshInfo = IFA->FindOrAddMesh(FoliageType);
                    }
                }
                if (!FoliageType || !MeshInfo)
                    break;

                const auto& p = PrimComponent->GetComponentToWorld().GetLocation();
                const auto& r = PrimComponent->GetComponentToWorld().GetRotation().Rotator();
                const auto& s = PrimComponent->GetComponentToWorld().GetScale3D();

                FFoliageInstance instance;
                instance.Location = p;
                instance.Rotation = r;
                instance.DrawScale3D = s;
                instance.ZOffset = 0.f;

                MeshInfo->AddInstance(IFA, FoliageType, instance);
                suc = 1;
            }
            succ += suc;
        }
        FText Message =
            FText::Format(LOCTEXT("kkTools", "{0} Object Succeed."), FText::FromString(FString::FromInt(succ)));
        FMessageDialog::Open(EAppMsgType::Ok, Message);
        return FReply::Handled();
    }

    FReply Mesh2Foliage()
    {
        UWorld* World = GEditor->GetEditorWorldContext().World();
        AInstancedFoliageActor* IFA = AInstancedFoliageActor::GetInstancedFoliageActorForCurrentLevel(World, true);

        USelection* selection = GEditor->GetSelectedActors();
        int32 succ = 0;
        for (int32 i = 0; i < selection->Num(); i++)
        {
            UObject* obj = selection->GetSelectedObject(i);
            AActor* actor = Cast<AActor>(obj);
            if (!actor)
                continue;

            TInlineComponentArray<UPrimitiveComponent*> PrimitiveComponents;
            actor->GetComponents(PrimitiveComponents);
            int32 suc = 0;
            for (int32 CompIdx = 0; CompIdx < PrimitiveComponents.Num(); ++CompIdx)
            {
                UPrimitiveComponent* PrimComponent = PrimitiveComponents[CompIdx];
                UStaticMeshComponent* smc = Cast<UStaticMeshComponent>(PrimComponent);
                if (!smc)
                    continue;

                UFoliageType* FoliageType = nullptr;
                FFoliageInfo* MeshInfo = nullptr;
                UStaticMesh* mesh = smc->GetStaticMesh();
                if (mesh)
                {
                    FoliageType = IFA->GetLocalFoliageTypeForSource(mesh);
                    if (!FoliageType)
                    {
                        IFA->AddMesh(mesh, &FoliageType);
                    }
                    if (FoliageType)
                        MeshInfo = IFA->FindOrAddMesh(FoliageType);
                }
                if (!FoliageType || !MeshInfo)
                    break;

                const auto& p = PrimComponent->GetComponentToWorld().GetLocation();
                const auto& r = PrimComponent->GetComponentToWorld().GetRotation().Rotator();
                const auto& s = PrimComponent->GetComponentToWorld().GetScale3D();

                if (smc->CustomDepthStencilValue)
                {
                    FoliageType->bRenderCustomDepth = 1;
                    FoliageType->CustomDepthStencilValue = smc->CustomDepthStencilValue;
                }

                FFoliageInstance instance;
                instance.Location = p;
                instance.Rotation = r;
                instance.DrawScale3D = s;
                instance.ZOffset = 0.f;

                MeshInfo->AddInstance(IFA, FoliageType, instance);
                suc = 1;
            }
            succ += suc;
        }
        FText Message =
            FText::Format(LOCTEXT("kkTools", "{0} Object Succeed."), FText::FromString(FString::FromInt(succ)));
        FMessageDialog::Open(EAppMsgType::Ok, Message);
        return FReply::Handled();
    }

    int stencil_value = 0;
    void SetStencilValue(int value)
    {
        stencil_value = value;
    }

    TOptional<int> GetStencilValue() const
    {
        return stencil_value;
    }
    FReply SetStencil()
    {
        UWorld* World = GEditor->GetEditorWorldContext().World();
        USelection* selection = GEditor->GetSelectedActors();
        int32 succ = 0;
        for (int32 i = 0; i < selection->Num(); i++)
        {
            UObject* obj = selection->GetSelectedObject(i);
            AActor* actor = Cast<AActor>(obj);
            if (!actor)
                continue;

            TInlineComponentArray<UPrimitiveComponent*> PrimitiveComponents;
            actor->GetComponents(PrimitiveComponents);
            int32 suc = 0;
            for (int32 CompIdx = 0; CompIdx < PrimitiveComponents.Num(); ++CompIdx)
            {
                UPrimitiveComponent* PrimComponent = PrimitiveComponents[CompIdx];
                UStaticMeshComponent* smc = Cast<UStaticMeshComponent>(PrimComponent);
                if (!smc)
                    continue;
                smc->bRenderCustomDepth = 1;
                smc->CustomDepthStencilWriteMask = ERendererStencilMask::ERSM_255;
                smc->CustomDepthStencilValue = stencil_value;
                suc++;
            }
            if (suc > 1)
            {
                // UPackage* Package = Cast<UPackage>(actor->GetOuter());
            }
            succ++;
        }

        FText Message =
            FText::Format(LOCTEXT("kkTools", "{0} Object Succeed."), FText::FromString(FString::FromInt(succ)));
        FMessageDialog::Open(EAppMsgType::Ok, Message);

        return FReply::Handled();
    }
};

TSharedPtr<Functions> functions;

void FObjectCreatorModule::StartupModule()
{
    // This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin
    // file per-module

    FObjectCreatorStyle::Initialize();
    FObjectCreatorStyle::ReloadTextures();

    FObjectCreatorCommands::Register();

    PluginCommands = MakeShareable(new FUICommandList);

    PluginCommands->MapAction(FObjectCreatorCommands::Get().OpenPluginWindow,
        FExecuteAction::CreateRaw(this, &FObjectCreatorModule::PluginButtonClicked), FCanExecuteAction());

    // UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this,
    // &FObjectCreatorModule::RegisterMenus));
    FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");

    {
        TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
        MenuExtender->AddMenuExtension("WindowLayout", EExtensionHook::After, PluginCommands,
            FMenuExtensionDelegate::CreateRaw(this, &FObjectCreatorModule::AddMenuExtension));

        LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
    }

    {
        TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
        ToolbarExtender->AddToolBarExtension("Settings", EExtensionHook::After, PluginCommands,
            FToolBarExtensionDelegate::CreateRaw(this, &FObjectCreatorModule::AddToolbarExtension));

        LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
    }

    FGlobalTabmanager::Get()
        ->RegisterNomadTabSpawner(
            ObjectCreatorTabName, FOnSpawnTab::CreateRaw(this, &FObjectCreatorModule::OnSpawnPluginTab))
        .SetDisplayName(LOCTEXT("FObjectCreatorTabTitle", "ObjectCreator"))
        .SetMenuType(ETabSpawnerMenuType::Hidden);
    functions = MakeShareable(new Functions);
}

void FObjectCreatorModule::ShutdownModule()
{
    // This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
    // we call this function before unloading the module.

    // UToolMenus::UnRegisterStartupCallback(this);

    // UToolMenus::UnregisterOwner(this);

    FObjectCreatorStyle::Shutdown();

    FObjectCreatorCommands::Unregister();

    FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(ObjectCreatorTabName);
}

TSharedRef<SDockTab> FObjectCreatorModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
    return SNew(SDockTab).TabRole(
        ETabRole::NomadTab)[SNew(SVerticalBox)

                            + SVerticalBox::Slot().Padding(
                                  0)[SNew(SHorizontalBox) +
                                     SHorizontalBox::Slot()
                                         .VAlign(VAlign_Center)
                                         .Padding(0)
                                         .MaxWidth(150)[

                                             // Put your tab content here!
                                             SNew(SBox).HeightOverride(150.f).WidthOverride(
                                                 150.f)[SNew(SButton)
                                                            .HAlign(HAlign_Center)
                                                            .VAlign(VAlign_Center)
                                                            .OnClicked(functions.Get(), &Functions::RunTrafficSign)
                                                            .Text(LOCTEXT("RunAuto", "Run Auto"))
                                                            .ToolTipText(LOCTEXT("OpenData_Tooltip", "Open Data"))]

    ]

                                     +
                                     SHorizontalBox::Slot()[SNew(
                                         SBox)[SNew(SVerticalBox) +
                                               SVerticalBox::Slot().AutoHeight().Padding(0.f, 5.f, 0.f,
                                                   0.f)[SNew(SHorizontalBox) +
                                                        SHorizontalBox::Slot()
                                                            .VAlign(VAlign_Center)
                                                            .MaxWidth(150)[

                                                                SNew(STextBlock).Text(LOCTEXT("WorldDir", "World Dir:"))

    ] +
                                                        SHorizontalBox::Slot()
                                                            .VAlign(VAlign_Center)
                                                            .HAlign(HAlign_Left)
                                                            .AutoWidth()[SNew(SEditableTextBox)
                                                                             .Text(functions.Get(),
                                                                                 &Functions::GetWorldDir)
                                                                             .OnTextCommitted(functions.Get(),
                                                                                 &Functions::SetWorldDir)]] +
                                               SVerticalBox::Slot().AutoHeight().Padding(0.f, 5.f, 0.f,
                                                   0.f)[SNew(SHorizontalBox) +
                                                        SHorizontalBox::Slot()
                                                            .VAlign(VAlign_Center)
                                                            .MaxWidth(150)[

                                                                SNew(STextBlock)
                                                                    .Text(LOCTEXT("TrafficSignDir", "TrafficSign Dir:"))

    ] +
                                                        SHorizontalBox::Slot()
                                                            .VAlign(VAlign_Center)
                                                            .HAlign(HAlign_Left)
                                                            .AutoWidth()[SNew(SEditableTextBox)
                                                                             .Text(functions.Get(),
                                                                                 &Functions::GetTrafficSignDir)
                                                                             .OnTextCommitted(functions.Get(),
                                                                                 &Functions::SetTrafficSignDir)]]]

    ]

    ] +
                            SVerticalBox::Slot().Padding(
                                0)[SNew(SHorizontalBox) +
                                   SHorizontalBox::Slot().Padding(0).VAlign(VAlign_Center)[
                                       // Put your tab content here!

                                       SNew(SBox)[SNew(SButton)
                                                      .HAlign(HAlign_Center)
                                                      .VAlign(VAlign_Center)
                                                      .OnClicked(functions.Get(), &Functions::Spec2Foliage)
                                                      .Text(LOCTEXT("Spec2Foliage", "Spec to Foliage"))
                                                      .ToolTipText(LOCTEXT("Spec2Foliage_Tooltip", "Spec to Foliage"))]

    ] +
                                   SHorizontalBox::Slot()
                                       .VAlign(VAlign_Center)
                                       .Padding(0)[SNew(
                                           SBox)[SNew(SButton)
                                                     .HAlign(HAlign_Center)
                                                     .VAlign(VAlign_Center)
                                                     .OnClicked(functions.Get(), &Functions::Mesh2Foliage)
                                                     .Text(LOCTEXT("Mesh2Foliage", "Mesh2Foliage"))
                                                     .ToolTipText(LOCTEXT("Mesh2Foliage_Tooltip", "Mesh2Foliage"))]

    ]] +
                            SVerticalBox::Slot().Padding(
                                0)[SNew(SHorizontalBox) +
                                   SHorizontalBox::Slot()
                                       .VAlign(VAlign_Center)
                                       .Padding(0)[

                                           SNew(SButton)
                                               .HAlign(HAlign_Center)
                                               .VAlign(VAlign_Center)
                                               .OnClicked(functions.Get(), &Functions::SetStencil)
                                               .Text(LOCTEXT("SetStencil", "Set Stencil"))
                                               .ToolTipText(LOCTEXT("SetStencil_Tooltip", "SetStencil"))

    ]

                                   + SHorizontalBox::Slot()
                                         .VAlign(VAlign_Center)
                                         .Padding(0)[SNew(SNumericEntryBox<int>)
                                                         .AllowSpin(true)
                                                         .MinValue(1)
                                                         .MaxSliderValue(254)
                                                         .MinDesiredValueWidth(20)
                                                         .OnValueChanged(functions.Get(), &Functions::SetStencilValue)
                                                         .Value(functions.Get(), &Functions::GetStencilValue)]

    ]];
}

void FObjectCreatorModule::PluginButtonClicked()
{
    FGlobalTabmanager::Get()->TryInvokeTab(ObjectCreatorTabName);
}
void FObjectCreatorModule::AddMenuExtension(FMenuBuilder& Builder)
{
    Builder.AddMenuEntry(FObjectCreatorCommands::Get().OpenPluginWindow);
}

void FObjectCreatorModule::AddToolbarExtension(FToolBarBuilder& Builder)
{
    Builder.AddToolBarButton(FObjectCreatorCommands::Get().OpenPluginWindow);
}
void FObjectCreatorModule::RegisterMenus()
{
    // Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
    /*FToolMenuOwnerScoped OwnerScoped(this);

    {
        UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
        {
            FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
            Section.AddMenuEntryWithCommandList(FObjectCreatorCommands::Get().OpenPluginWindow, PluginCommands);
        }
    }

    {
        UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
        {
            FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
            {
                FToolMenuEntry& Entry =
    Section.AddEntry(FToolMenuEntry::InitToolBarButton(FObjectCreatorCommands::Get().OpenPluginWindow));
                Entry.SetCommandList(PluginCommands);
            }
        }
    }*/
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FObjectCreatorModule, ObjectCreator)
