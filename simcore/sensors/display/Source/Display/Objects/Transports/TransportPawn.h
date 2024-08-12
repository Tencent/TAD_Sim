// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Display/Objects/SimActorInterface.h"
#include "TransportInterface.h"
#include "TransportPawn.generated.h"

// #if WITH_EDITORONLY_DATA
USTRUCT(BlueprintType)
struct FLampInfoEd
{
    GENERATED_USTRUCT_BODY()
public:
    UPROPERTY(VisibleAnywhere)
    FName name;
    UPROPERTY(EditAnyWhere)
    bool bActive = false;
};
// #endif //WITH_EDITORONLY_DATA

/**
 *
 */
UCLASS(Abstract, config = game /*, perObjectConfig*/)
class DISPLAY_API ATransportPawn : public APawn, public ISimActorInterface, public ITransportInterface
{
    GENERATED_BODY()

public:
    // Sets default values for this pawn's properties
    ATransportPawn();

    virtual void ApplyCatalogOffset(const FVector OffSet);

    UFUNCTION(BlueprintCallable)
    FVector BP_GetVelocity() const;

    UFUNCTION(BlueprintCallable)
    float BP_GetTimeStamp() const;

public:
    /* ~ Object Interface ~ */

    // Get the configuration from SimActor
    virtual const FSimActorConfig* GetConfig() const;
    // Init SimActor
    virtual void Init(const FSimActorConfig& _Config);
    // Update SimActor
    virtual void Update(const FSimActorInput& _Input, FSimActorOutput& _Output);
    // Destroy the SimActor
    virtual void Destroy();
    // Get current timestamp
    virtual double GetTimeStamp() const;

    /* ~ Object Interface ~ */

    /** Allow actors to initialize themselves on the C++ side after all of their components have been initialized, only
     * called during gameplay */
    virtual void PostInitializeComponents();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    // Basic root component
    class UBasicInfoComp* basicInfoComp = NULL;

    // Sim move component
    class USimMoveComponent* simMoveComponent = NULL;

    // Skeleton mesh component
    UPROPERTY(BlueprintReadWrite, VisibleDefaultsOnly, meta = (AllowPrivateAccess = "true"))
    class USkeletalMeshComponent* meshComp = NULL;

    // Light component
    UPROPERTY(VisibleDefaultsOnly, meta = (AllowPrivateAccess = "true"))
    class ULightMasterComp* lightMasterComp = NULL;

    // Camera component
    class UCameraMasterComponent* cameraMasterComp = NULL;

    // Camera for view
    UPROPERTY(VisibleDefaultsOnly, meta = (AllowPrivateAccess = "true"))
    class UCameraComponent* camera_BirdView = NULL;
    UPROPERTY(VisibleDefaultsOnly, meta = (AllowPrivateAccess = "true"))
    class UCameraComponent* camera_Driver = NULL;
    UPROPERTY(VisibleDefaultsOnly, meta = (AllowPrivateAccess = "true"))
    class UCameraComponent* camera_Roof = NULL;
    UPROPERTY(VisibleDefaultsOnly, meta = (AllowPrivateAccess = "true"))
    class UCameraComponent* camera_Free = NULL;
    UPROPERTY(VisibleDefaultsOnly, meta = (AllowPrivateAccess = "true"))
    class USpringArmComponent* springArm_Free = NULL;
    UPROPERTY(VisibleDefaultsOnly, meta = (AllowPrivateAccess = "true"))
    class USpringArmComponent* springArm_Bird = NULL;

    UPROPERTY()
    class UMannedControlComponent* mannedControlComponent = NULL;

    UPROPERTY()
    class UTrajectoryComponent* trajectoryComponent = nullptr;

    // Default camera name
    UPROPERTY(config)
    FString defaultCameraName = TEXT("Camera_BirdView");

    //// lamp map
    // TMap< FString, TTuple< int32, class ULocalLightComponent* > > lampMap;

    // DayNight
    bool bBeingNight = false;
    bool bBeingDay = true;

protected:
    bool bActiveRotateView = false;

    /* Snap Ground Var */
    FRotator sumRpy = FRotator();
    float sumZ = 0;
    TArray<FRotator> smoothRpyBuf;
    TArray<float> smoothZBuf;
    int32 curIdx = 0;
    bool isEgoSnap = true;
    ETrafficType trafficType = ETrafficType::ST_TRAFFIC;
    // float raycastingDistance = 300;// unit is cm
    /* Snap Ground Var */

    void BeginRotateView();
    void EndRotateView();

    void ZoomIn();
    void ZoomOut();

    // template<typename T>
    // bool CreateLampWithLightComponent(const FString& _LampName, USkeletalMeshComponent* _TransportMesh, TMap<
    // FString, TTuple< int32, class ULocalLightComponent* > >& _LampMap); bool CreateLamp(const FString& _LampName,
    // USkeletalMeshComponent* _TransportMesh, TMap< FString, TTuple< int32, class ULocalLightComponent* > >& _LampMap);

    virtual void SetupCameras();
    virtual void SetupLights();
    // #if WITH_EDITOR
    //     virtual void SetupAllCameras_EDITOR();
    // #endif

    // virtual void SetupLights();

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Called to bind functionality to input
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    virtual void PostLoad() override;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif    // WITH_EDITOR
#if WITH_EDITORONLY_DATA
    UPROPERTY(EditAnyWhere, EditFixedSize, Category = "Transport Lamp")
    TArray<FLampInfoEd> lampInfoArray;
#endif    // #endif // WITH_EDITOR

    /**
     * Called when an instance of this class is placed (in editor) or spawned.
     * @param    Transform            The transform the actor was constructed at.
     */
    virtual void OnConstruction(const FTransform& Transform);

    virtual void SwitchCamera(FString _Name /*= TEXT("")*/);

    void SwitchCamera();

    //// Switch light in the vehicle by light name.
    // virtual void SwitchLight(bool _TurnOn, FString _Name);

    // virtual FTransform GetSnapGroundTransform(const FVector& _Location, const FRotator& _Rotation);

    virtual FVector GetVelocity() const override;

    virtual bool SwitchDrivingMode(EDrivingMode _DrivingMode);

    bool InstallCamera(const FString& _Name, class UCameraComponent* _Camera);
    bool UninstallCamera(const FString& _Name);
    bool SetDefaultCamera(const FString& _Name);

    UPROPERTY()
    int modeDrive = 0;
    void ToggleDriveMode();

    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);

    UPROPERTY(config)
    bool bUseRemoteMannedControl = false;
    UPROPERTY(config)
    bool bConstrainAspectRatio_CameraDriver = false;
    UPROPERTY(config)
    float aspectRatio_CameraDriver = 1.7777;
    UPROPERTY(config)
    FVector offset_CameraDriver = FVector(0);
    UPROPERTY(config)
    float fov_CameraDriver = 90;
    UPROPERTY(config)
    bool bRenderTrajectory = false;

    UPROPERTY(config)
    float spring_length = 500;
    UPROPERTY(config)
    FRotator spring_rotator = FRotator(0, 0, 0);

    UPROPERTY(EditAnyWhere)
    bool bDisableCatalog = false;

    FString GetDefaultCameraName() const
    {
        return defaultCameraName;
    }

public:
    AActor* physicProxyActor = nullptr;
};

// template<typename T>
// bool ATransportPawn::CreateLampWithLightComponent(const FString& _LampName, USkeletalMeshComponent* _TransportMesh,
// TMap< FString, TTuple< int32, class ULocalLightComponent* > >& _LampMap)
//{
//     if (!_TransportMesh)
//     {
//         return false;
//     }
//     TTuple<int32, ULocalLightComponent*> NewLamp;
//     const USkeletalMeshSocket* Socket = meshComp->GetSocketByName(*_LampName);
//     int32 MatIndex = meshComp->GetMaterialIndex(TEXT("Mat_carlight"));
//     if (Socket && MatIndex != INDEX_NONE)
//     {
//         UMaterialInterface* MatInterface = meshComp->GetMaterial(MatIndex);
//         ULocalLightComponent* Light = NULL;
//         if (MatInterface)
//         {
//             UMaterialInstanceDynamic* MatInsDynamic = _TransportMesh->CreateDynamicMaterialInstance(MatIndex,
//             MatInterface); float DefaultValue = 0; if (MatInsDynamic &&
//             MatInsDynamic->GetScalarParameterValue(*_LampName, DefaultValue))
//             {
//                 MatInsDynamic->SetScalarParameterValue(*_LampName, 0.f);
//             }
//             else
//             {
//                 UE_LOG(LogTemp, Error, TEXT("Can`t get lamp material parameter!(ActorName: %s, LampName: %s)"),
//                 *this->GetName(), *_LampName); return false;
//             }
//             NewLamp.Get<0>() = MatIndex;
//
//             Light = NewObject<T>(this, *_LampName);
//             if (Light)
//             {
//                 Light->RegisterComponent();
//                 Light->AttachToComponent(meshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale,
//                 *_LampName); Light->SetVisibility(false); NewLamp.Get<1>() = Light;
//             }
//             else
//             {
//                 NewLamp.Get<1>() = NULL;
//                 UE_LOG(LogTemp, Error, TEXT("Create LightComponent failed!(ActorName: %s, LampName: %s)"),
//                 *this->GetName(), *_LampName); return false;
//             }
//
//             TTuple<int32, ULocalLightComponent*>* RepeatLamp = _LampMap.Find(_LampName);
//             if (RepeatLamp)
//             {
//                 *RepeatLamp = NewLamp;
//             }
//             else
//             {
//                 _LampMap.Add(_LampName, NewLamp);
//             }
//         }
//         else
//         {
//             UE_LOG(LogTemp, Error, TEXT("Can`t get lamp material!(ActorName: %s, LampName: %s)"), *this->GetName(),
//             *_LampName); return false;
//         }
//     }
//     return true;
// }
