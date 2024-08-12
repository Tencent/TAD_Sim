// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MeshMasterComp.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DISPLAY_API UMeshMasterComp : public UActorComponent
{
    GENERATED_BODY()

public:
    // Sets default values for this component's properties
    UMeshMasterComp();

protected:
    // Called when the game starts
    virtual void BeginPlay() override;

public:
    bool LoadSkeletalMeshByName(const FString& _Name, const FString& _FileName, USkeletalMeshComponent* _SkeletalMesh);
    bool LoadAnimByName(const FString& _Name, const FString& _FileName, USkeletalMeshComponent* _SkeletalMesh);
};
