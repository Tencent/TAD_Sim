// Fill out your copyright notice in the Description page of Project Settings.

#include "MapModelComponent.h"
#include "XmlFile.h"
#include "LoaderBPFunctionLibrary.h"

void UMapModelComponent::Init(const TMap<FString, TPair<FString, FVector>>& _ModelData, const FString& _ModelRootPath)
{
    for (auto& Elem : _ModelData)
    {
        const TPair<FString, FVector>& Info = Elem.Value;
        FMapModelData NewData(Info.Key, Info.Value);
        ModelDataMap.Emplace(Elem.Key, NewData);
    }
    ModelAssetPath = _ModelRootPath;
}

UStaticMesh* UMapModelComponent::GetModelByName(const FString& ModelName)
{
    if (FMapModelData* Data = ModelDataMap.Find(ModelName))
    {
        return URuntimeMeshLoader::Get()->LoadStaticMeshFromFBX(
            ModelAssetPath + TEXT("/") + Data->Path, true, EMeshType::MT_MAP_OBJ);
    }

    return nullptr;
}
