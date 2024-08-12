// Fill out your copyright notice in the Description page of Project Settings.

#include "Env/EnvDepthTrace.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Runtime/Engine/Classes/Components/SceneCaptureComponent2D.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AEnvDepthTrace::AEnvDepthTrace()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    CaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(FName(TEXT("CaptureCamera")));
    CaptureComponent->SetActive(true);

    DepthTraceRT = LoadObject<UTextureRenderTarget2D>(
        nullptr, TEXT("TextureRenderTarget2D'/Game/Weather/VFX_Weather/VFX_Weather_Particles/RT_EnvMask.RT_EnvMask'"));

    CaptureComponent->ProjectionType = ECameraProjectionMode::Orthographic;
    CaptureComponent->OrthoWidth = 2000.f;
    CaptureComponent->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
    CaptureComponent->CaptureSource = ESceneCaptureSource::SCS_SceneDepth;
    CaptureComponent->bCaptureEveryFrame = true;
    CaptureComponent->bCaptureOnMovement = true;
    CaptureComponent->TextureTarget = DepthTraceRT;
    CaptureComponent->SetWorldRotation(FRotator(-90, 0, 90));
}

// Called when the game starts or when spawned
void AEnvDepthTrace::BeginPlay()
{
    Super::BeginPlay();

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("EnvOcclusion"), FoundActors);
    CaptureComponent->ShowOnlyActors = FoundActors;
}

// Called every frame
void AEnvDepthTrace::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}
