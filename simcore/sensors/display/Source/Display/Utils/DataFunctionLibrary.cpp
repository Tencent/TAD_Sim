// Fill out your copyright notice in the Description page of Project Settings.

#include "DataFunctionLibrary.h"
#include "Misc/FileHelper.h"
#include "Engine/DataTable.h"
#include "Components/MeshComponent.h"
#include "Components/DecalComponent.h"

bool UDataFunctionLibrary::FillDataTableFromCSVFile(UDataTable* DataTable, const FString& CSVFilePath)
{
    if (!DataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("Can't fill an invalid DataTable."));
        return false;
    }

    FString Data;
    if (!FFileHelper::LoadFileToString(Data, *CSVFilePath))
    {
        UE_LOG(LogTemp, Error, TEXT("Can't load the file '%s'."), *CSVFilePath);
        return false;
    }

    return FillDataTableFromCSVString(DataTable, Data);
}

bool UDataFunctionLibrary::FillDataTableFromCSVString(UDataTable* DataTable, const FString& CSVString)
{
    if (!DataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("Can't fill an invalid DataTable."));
        return false;
    }

    bool bResult = true;
    if (CSVString.Len() == 0)
    {
        DataTable->EmptyTable();
    }
    else
    {
        TArray<FString> Errors = DataTable->CreateTableFromCSVString(CSVString);
        if (Errors.Num())
        {
            for (const FString& Error : Errors)
            {
                UE_LOG(LogTemp, Warning, TEXT("%s"), *Error);
            }
        }
        bResult = Errors.Num() == 0;
    }
    return bResult;
}

UMaterialInstanceDynamic* UDataFunctionLibrary::CreateMaterialDynamicInstance(
    UMeshComponent* _Mesh, const FString& _SlotName)
{
    if (!_Mesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("MeshComponent is null!"));
        return NULL;
    }
    int32 Index = _Mesh->GetMaterialIndex(*_SlotName);
    if (Index == INDEX_NONE)
    {
        UE_LOG(LogTemp, Warning, TEXT("Can`t get index by slot name!(SlotName: %s)"), *_SlotName);
        return NULL;
    }
    UMaterialInterface* MatInterface = _Mesh->GetMaterial(Index);
    if (!MatInterface)
    {
        UE_LOG(LogTemp, Warning, TEXT("Can`t get index by index!(Index: %d)"), Index);
        return NULL;
    }
    UMaterialInstanceDynamic* MatInstance = _Mesh->CreateDynamicMaterialInstance(Index, MatInterface);
    if (MatInstance)
    {
        return MatInstance;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Create MaterialInstanceDynamic fail!"));
        return NULL;
    }
}

void UDataFunctionLibrary::UpdateDecalSizeWithScale(UDecalComponent* Decal, const FVector& Scale)
{
    if (Decal)
    {
        Decal->DecalSize *= Scale;
    }
}

bool UDataFunctionLibrary::GetLogoConfig(FString& LogoConfig)
{
    LogoConfig = "";
    if (GConfig->GetString(TEXT("LogoConfig"), TEXT("LogoName"), LogoConfig, GGameIni))
    {
        return true;
    }
    return false;
}

bool UDataFunctionLibrary::CompareTexture(UTexture2D* Tex1, UTexture2D* Tex2)
{
#if WITH_EDITOR
    TArray<FColor> Color1;
    TArray<FColor> Color2;
    int32 num = 0;
    {
        TextureCompressionSettings OldCompressionSettings = Tex1->CompressionSettings;
        TextureMipGenSettings OldMipGenSettings = Tex1->MipGenSettings;
        bool OldSRGB = Tex1->SRGB;

        Tex1->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
        Tex1->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
        Tex1->SRGB = false;
        Tex1->UpdateResource();

        const FColor* FormatedImageData =
            static_cast<const FColor*>(Tex1->PlatformData->Mips[0].BulkData.LockReadOnly());

        for (int32 X = 0; X < Tex1->GetSizeX(); X++)
        {
            for (int32 Y = 0; Y < Tex1->GetSizeY(); Y++)
            {
                FColor PixelColor = FormatedImageData[Y * Tex1->GetSizeX() + X];
                Color1.Add(PixelColor);
            }
        }

        Tex1->PlatformData->Mips[0].BulkData.Unlock();

        Tex1->CompressionSettings = OldCompressionSettings;
        Tex1->MipGenSettings = OldMipGenSettings;
        Tex1->SRGB = OldSRGB;
        Tex1->UpdateResource();
    }

    {
        TextureCompressionSettings OldCompressionSettings = Tex2->CompressionSettings;
        TextureMipGenSettings OldMipGenSettings = Tex2->MipGenSettings;
        bool OldSRGB = Tex2->SRGB;

        Tex2->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
        Tex2->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
        Tex2->SRGB = false;
        Tex2->UpdateResource();

        const FColor* FormatedImageData =
            static_cast<const FColor*>(Tex2->PlatformData->Mips[0].BulkData.LockReadOnly());

        for (int32 X = 0; X < Tex2->GetSizeX(); X++)
        {
            for (int32 Y = 0; Y < Tex2->GetSizeY(); Y++)
            {
                FColor PixelColor = FormatedImageData[Y * Tex2->GetSizeX() + X];
                Color2.Add(PixelColor);
            }
        }

        Tex2->PlatformData->Mips[0].BulkData.Unlock();

        Tex2->CompressionSettings = OldCompressionSettings;
        Tex2->MipGenSettings = OldMipGenSettings;
        Tex2->SRGB = OldSRGB;
        Tex2->UpdateResource();
    }

    if (Color1.Num() != Color2.Num())
    {
        return false;
    }

    for (int32 i = 0; i < Color1.Num(); i++)
    {
        Color2[i].A = Color1[i].A;
        if (Color1[i] != Color2[i])
        {
            num++;
        }
    }
    if (num > 0)
    {
        return false;
    }
    return true;
#else
    return true;
#endif
}
