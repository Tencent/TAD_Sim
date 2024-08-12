// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/PointLightComponent.h"
#include "LightMasterComp.generated.h"

// #if WITH_EDITOR
// USTRUCT(BlueprintType)
// struct FLampInfo
//{
//     GENERATED_USTRUCT_BODY()
// public:
//     UPROPERTY(VisibleAnywhere)
//         FName name;
//     UPROPERTY(EditAnyWhere)
//         bool bActive = false;
// };
// #endif //WITH_EDITOR

USTRUCT(BlueprintType)
struct FLampMaterialData
{
    GENERATED_USTRUCT_BODY()
public:
    UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
    class UMaterialInstanceDynamic* matInstance = NULL;
    UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
    FString paramName;
    UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
    float defaultScaleValue = 0;
};

USTRUCT(BlueprintType)
struct FLampInfo
{
    GENERATED_USTRUCT_BODY()
public:
    UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
    TArray<FLampMaterialData> materialArry;
    UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
    TArray<class ULocalLightComponent*> lightArry;
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    bool bActive = false;

    bool IsEmpty() const
    {
        if (lightArry.Num() <= 0 && materialArry.Num() <= 0)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DISPLAY_API ULightMasterComp : public UActorComponent
{
    GENERATED_BODY()

public:
    // Sets default values for this component's properties
    ULightMasterComp();

protected:
    // Called when the game starts
    virtual void BeginPlay() override;

    // USkeletalMeshComponent* targetMesh = NULL;

    // TTuple<int, FString, UPointLightComponent*, float> lamp;

    // TMap< FString, TTuple<UMaterialInstanceDynamic*, UPointLightComponent*, float> > lampMap;

    // TMap< UMeshComponent*, TMap< FString, TTuple<int, FString, UPointLightComponent*, bool> > > meshLampMap;

    TMap<class UMeshComponent*, TMap<FString, TTuple<int, FString, class ULocalLightComponent*, bool> > > meshLampMap;

    UPROPERTY(EditAnywhere, EditFixedSize)
    TMap<FString, FLampInfo> lampMap;

    TMap<FString, TArray<FString> > groupLampMap;

    // UPointLightComponent* CreateLightComp(ELightType _Tpye);

    /*UMaterialInstanceDynamic* CreateMID(USkeletalMeshComponent* _Mesh, int32 _MatId);*/

    // bool SwitchLight(class UMeshComponent* _Mesh, FString _Name, bool _TurnOn);

    void SwitchLightByName(FString _Name, bool _TurnOn);

    bool SwitchLightByGroup(FString _Name, bool _TurnOn);

public:
    // Called every frame
    virtual void TickComponent(
        float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif    // WITH_EDITOR
    // void SwitchLight(FString _Name, bool _TurnOn);
public:
    template <typename T>
    bool CreateLampWithLightComponent(const FString& _LampName, const FString& _MatSlot, const FString& _VarName,
        const FString& _SocketName, UMeshComponent* _TransportMesh);
    bool CreateLamp(
        const FString& _LampName, const FString& _MatSlot, const FString& _VarName, UMeshComponent* _TransportMesh);
    UFUNCTION(BlueprintCallable, Category = "Lamp")
    bool CreateLamp(const FString& _LampName, const FLampInfo& _LampData);
    void SwitchLight(FString _Name, bool _TurnOn);
    FLampInfo* GetLamp(const FString& _LampName);
    void DeleteAllLamp();

    bool GroupLamp(const FString& _GroupName, const TArray<FString>& _LampNames);

    const TMap<FString, TTuple<int, FString, class ULocalLightComponent*, bool> >* GetLampMapByMesh(
        const UMeshComponent* _Mesh);
    TTuple<int, FString, class ULocalLightComponent*, bool>* GetLamp(UMeshComponent* _Mesh, const FString& _LampName);
    // TTuple<int, FString, class ULocalLightComponent*, bool>* GetLamp(const FString& _LampName);
    bool IsLampNameExist(const FString& _NewLampName);
    bool IsGroupNameExist(const FString& _GroupName);
};

template <typename T>
bool ULightMasterComp::CreateLampWithLightComponent(const FString& _LampName, const FString& _MatSlot,
    const FString& _VarName, const FString& _SocketName, UMeshComponent* _TransportMesh)
{
    if (!_TransportMesh)
    {
        UE_LOG(LogTemp, Error, TEXT("Mesh is null!(ActorName: %s, LampName: %s)"), *this->GetOwner()->GetName(),
            *_LampName);
        return false;
    }
    if (IsLampNameExist(_LampName))
    {
        UE_LOG(LogTemp, Error, TEXT("Lamp name is exist!(ActorName: %s, LampName: %s)"), *this->GetOwner()->GetName(),
            *_LampName);
        return false;
    }
    // Set material
    TTuple<int, FString, class ULocalLightComponent*, bool> NewLamp;
    int32 MatIndex = _TransportMesh->GetMaterialIndex(*_MatSlot);
    if (MatIndex != INDEX_NONE)
    {
        UMaterialInterface* MatInterface = _TransportMesh->GetMaterial(MatIndex);
        ULocalLightComponent* Light = NULL;
        if (MatInterface)
        {
            UMaterialInstanceDynamic* MatInsDynamic =
                _TransportMesh->CreateDynamicMaterialInstance(MatIndex, MatInterface);
            float DefaultValue = 0;
            if (MatInsDynamic && MatInsDynamic->GetScalarParameterValue(*_VarName, DefaultValue))
            {
                MatInsDynamic->SetScalarParameterValue(*_VarName, 0.f);
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Can`t get lamp material parameter!(ActorName: %s, LampName: %s)"),
                    *this->GetOwner()->GetName(), *_LampName);
                return false;
            }
            NewLamp.Get<0>() = MatIndex;
            NewLamp.Get<1>() = _VarName;
            NewLamp.Get<2>() = NULL;
            NewLamp.Get<3>() = false;

            // Set LightComponent
            if (_TransportMesh->DoesSocketExist(*_SocketName))
            {
                Light = NewObject<T>(this->GetOwner(), *_LampName);
                if (Light)
                {
                    Light->RegisterComponent();
                    Light->AttachToComponent(
                        _TransportMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, *_SocketName);
                    Light->SetVisibility(false);
                    NewLamp.Get<2>() = Light;
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("Create LightComponent failed!(ActorName: %s, LampName: %s)"),
                        *this->GetOwner()->GetName(), *_LampName);
                    return false;
                }
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Can`t find socket in mesh!(ActorName: %s, LampName: %s, SocketName: %s)"),
                    *this->GetOwner()->GetName(), *_LampName, *_SocketName);
                return false;
            }

            // Add to map
            TMap<FString, TTuple<int, FString, class ULocalLightComponent*, bool> >* ExistLampMap =
                meshLampMap.Find(_TransportMesh);
            if (ExistLampMap)
            {
                TTuple<int, FString, class ULocalLightComponent*, bool>* ExistLamp = ExistLampMap->Find(_LampName);
                if (ExistLamp)
                {
                    *ExistLamp = NewLamp;
                }
                else
                {
                    ExistLampMap->Add(_LampName, NewLamp);
                }
            }
            else
            {
                TMap<FString, TTuple<int, FString, class ULocalLightComponent*, bool> > NewLampMap;
                NewLampMap.Add(_LampName, NewLamp);
                meshLampMap.Add(_TransportMesh, NewLampMap);
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Can`t get lamp material!(ActorName: %s, LampName: %s)"),
                *this->GetOwner()->GetName(), *_LampName);
            return false;
        }
    }
    return true;
}
