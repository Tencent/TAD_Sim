// Fill out your copyright notice in the Description page of Project Settings.

#include "OcculutionTraceComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UOcculutionTraceComponent::UOcculutionTraceComponent()
{
    // Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these
    // features off to improve performance if you don't need them.
    PrimaryComponentTick.bCanEverTick = true;

    // ...
}

// Called when the game starts
void UOcculutionTraceComponent::BeginPlay()
{
    Super::BeginPlay();

    // ...
}

// Called every frame
void UOcculutionTraceComponent::TickComponent(
    float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    BirdViewOcculutionTrace();
    // ...
}

void UOcculutionTraceComponent::BirdViewOcculutionTrace()
{
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetOwner(), 0);
    FVector CameraLoc = PlayerController->PlayerCameraManager->GetCameraLocation();
    TArray<TPair<TWeakObjectPtr<AActor>, TArray<FOcculusionAssetInfo>>> NewTraceInfo;
    TArray<int32> KeepTraceIndexs;

    TArray<FHitResult> ResultArry;
    FCollisionQueryParams QP = FCollisionQueryParams::DefaultQueryParam;
    FCollisionResponseParams RP = FCollisionResponseParams::DefaultResponseParam;
    QP.bTraceComplex = true;
    QP.bIgnoreTouches = false;
    QP.bFindInitialOverlaps = true;
    QP.AddIgnoredActor(this->GetOwner());
    if (GetWorld()->LineTraceMultiByChannel(
            ResultArry, CameraLoc, CameraLoc + FVector(0, 0, -3000), ECollisionChannel::ECC_EngineTraceChannel2, QP))
    {
        for (auto& Res : ResultArry)
        {
            bool NewTrace = true;
            for (int32 i = 0; i < TracedInfoList.Num(); i++)
            {
                TPair<TWeakObjectPtr<AActor>, TArray<FOcculusionAssetInfo>>& TracedInfo = TracedInfoList[i];
                if (TracedInfo.Key == Res.GetActor())
                {
                    NewTrace = false;
                    KeepTraceIndexs.Add(i);
                    break;
                }
            }
            if (NewTrace)
            {
                if (ICameraOcculusionInterface* COI = Cast<ICameraOcculusionInterface>(Res.GetActor()))
                {
                    TArray<FOcculusionAssetInfo> OcculisionInfoList = COI->Execute_GetOcculusionInfo(Res.GetActor());
                    for (auto& Info : OcculisionInfoList)
                    {
                        for (int32 j = 0; j < Info.OriginMaterials.Num(); j++)
                        {
                            UMaterialInterface* MI = Info.OcculusionMaterials[j];
                            Info.Mesh->SetMaterial(j, MI);
                            NewTraceInfo.Add(TPair<TWeakObjectPtr<AActor>, TArray<FOcculusionAssetInfo>>(
                                Res.GetActor(), OcculisionInfoList));
                        }
                    }
                }
            }
        }
    }

    for (int32 i = TracedInfoList.Num() - 1; i >= 0; i--)
    {
        if (!KeepTraceIndexs.Contains(i))
        {
            if (TracedInfoList[i].Key.IsValid())
            {
                TArray<FOcculusionAssetInfo>& LostTracedInfo = TracedInfoList[i].Value;
                for (auto& Info : LostTracedInfo)
                {
                    for (int32 j = 0; j < Info.OriginMaterials.Num(); j++)
                    {
                        UMaterialInterface* MI = Info.OriginMaterials[j];
                        Info.Mesh->SetMaterial(j, MI);
                    }
                }
            }

            TracedInfoList.RemoveAt(i);
        }
    }
    TracedInfoList.Append(NewTraceInfo);
}
