// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Manager.generated.h"

class ISimActorInterface;
struct FSimActorInput;
struct FSimActorOutput;
enum class ECatalogType : uint8;

USTRUCT()
struct FManagerConfig
{
    GENERATED_BODY();
};

USTRUCT()
struct FManagerIn
{
    GENERATED_BODY();
    double timeStamp = 0.f;
    double timeStamp_ego = 0.f;
    double timeStamp_tail = 0.f;
};

USTRUCT()
struct FManagerOut
{
    GENERATED_BODY();
};

UCLASS()
class DISPLAY_API AManager : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    AManager();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    static TArray<ISimActorInterface*> registeredSimActorArry;

    TArray<AManager*> childrenManagerArry;

    TArray<ISimActorInterface*> simActorArry;

    template <typename T, typename V>
    T* Cast_Data(V& From)
    {
        V* Ptr = &From;
        T* TPtr = static_cast<T*>(Ptr);
        return TPtr;
    }

    template <typename T>
    TSubclassOf<T> GetBPResourceDetails(const FString& _Name, TMap<FString, TSubclassOf<T> >& _Map)
    {
        TSubclassOf<T>* Value = _Map.Find(_Name);
        if (Value)
        {
            return *Value;
        }
        FString Path;
        GConfig->GetString(TEXT("/TypeList"), *_Name, Path, GGameIni);
        int32 Pos = INDEX_NONE;
        Path.FindLastChar(**FString(TEXT("\'")), Pos);
        Path.InsertAt(Pos, TEXT("_C"));
        TSubclassOf<T> Resource = LoadClass<T>(NULL, *Path);
        if (Resource.Get())
        {
            _Map.Add(_Name, Resource);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Type: %s Can not load BP asset!"), *_Name);
        }
        return Resource;
    }

    /*
    1、从cahce中获取class
    2、以过name从config中获取class
    3、以相同ModelPath的其他name从config中获取class
    */
    template <typename T>
    TSubclassOf<T> GetBPResource(const FString& _Name, ECatalogType CatalogType, TMap<FString, TSubclassOf<T> >& _Map)
    {
        TSubclassOf<T>* Value = _Map.Find(_Name);
        if (Value)
        {
            return *Value;
        }
        FString Path;
        GConfig->GetString(TEXT("/TypeList"), *_Name, Path, GGameIni);
        int32 Pos = INDEX_NONE;
        Path.FindLastChar(**FString(TEXT("\'")), Pos);
        Path.InsertAt(Pos, TEXT("_C"));
        TSubclassOf<T> Resource = LoadClass<T>(NULL, *Path);
        if (Resource.Get())
        {
            _Map.Add(_Name, Resource);
        }
        else
        {
            TSubclassOf<T> ResClass = GetBPResourceDetails(_Name, _Map);
            if (ResClass.Get())
            {
                return ResClass.Get();
            }

            UE_LOG(LogTemp, Log,
                TEXT("GetBPResource::current type is not valid UE asset ,search from other name that same model path"),
                *_Name);
        }
        return Resource;
    }
    // void UpdateSimActorArry(TArray<ISimActorInterface*> _SimActorArray, const FSimActorInput& _Input,
    // FSimActorOutput& _Output);
public:
    virtual void Init(const FManagerConfig& Config);

    virtual void Update(const FManagerIn& Input, FManagerOut& Output);

    // SimActor call this function to output data to manager in a step.
    virtual void Output(const FSimActorOutput& Output);

    static TArray<ISimActorInterface*> GetAllRegisteredSimActor()
    {
        return registeredSimActorArry;
    };

    static bool RegisterSimActor(ISimActorInterface* _Actor);
};
