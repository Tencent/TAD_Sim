// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SensorInterface.h"
#include "Engine/Engine.h"
#include "SensorFactory.generated.h"

UCLASS()
class DISPLAY_API ASensorFactory : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    ASensorFactory();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    template <class T = AActor>
    static T* SpawnSensor(UWorld* _World, UClass* _Class, const FSensorConfig& _InitParam)
    {
        if (!_World)
        {
            return NULL;
        }
        FActorSpawnParameters Params;
        T* TActor = _World->SpawnActor<T>(_Class);
        if (!TActor)
        {
            return NULL;
        }
        ISensorInterface* Sensor = Cast<ISensorInterface>(TActor);
        if (!Sensor)
        {
            return NULL;
        }
        if (!Sensor->Init(_InitParam))
        {
            Sensor->Destroy(TEXT("Sensor Init falid!"));
            FString SensorName;
            SensorName = _InitParam.typeName;
            SensorName += FString("(ID:") + FString::FromInt(_InitParam.id) + TEXT(")'s config has Error!");

            if (GEngine)
            {
                const int32 AlwaysAddKey = -1;
                GEngine->AddOnScreenDebugMessage(AlwaysAddKey, 100.0f, FColor::Red, SensorName);
            }
            UE_LOG(LogTemp, Warning, TEXT("%s"), *SensorName);

            // FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(SensorName));
            return NULL;
        }
        return TActor;
    }
};
