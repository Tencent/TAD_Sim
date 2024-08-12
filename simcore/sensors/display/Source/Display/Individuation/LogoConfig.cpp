// Fill out your copyright notice in the Description page of Project Settings.

#include "Individuation/LogoConfig.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceConstant.h"

// Sets default values
ALogoConfig::ALogoConfig()
{
    PrimaryActorTick.bCanEverTick = false;
#if !WITH_EDITOR
    FString LogoName = "";
    FString LogoMatPath = "";
    if (GConfig->GetString(TEXT("LogoConfig"), TEXT("LogoName"), LogoName, GGameIni))
    {
        if (GConfig->GetString(TEXT("LogoConfig"), *LogoName, LogoMatPath, GGameIni))
        {
            static ConstructorHelpers::FObjectFinder<UMaterialInterface> conHelpers(*LogoMatPath);
            Mat = Cast<UMaterialInterface>(conHelpers.Object);
        }
    }
#endif
}

void ALogoConfig::PostLoad()
{
    Super::PostLoad();

#if !WITH_EDITOR
    if (Mat)
        GetStaticMeshComponent()->SetMaterial(0, Mat);
#endif
}
