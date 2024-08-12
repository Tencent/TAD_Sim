// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DataFunctionLibrary.generated.h"

class UDataTable;

/**
 *
 */
UCLASS()
class DISPLAY_API UDataFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
public:
    /**
     * Empty and fill a Data Table from CSV file.
     * @param    CSVFilePath    The file path of the CSV file.
     * @return    True if the operation succeeds, check the log for errors if it didn't succeed.
     */
    UFUNCTION(BlueprintCallable, Category = "Utils | DataTable", DisplayName = "Fill Data Table from CSV File")
    static bool FillDataTableFromCSVFile(UDataTable* DataTable, const FString& CSVFilePath);

    /**
     * Empty and fill a Data Table from CSV string.
     * @param    CSVString    The Data that representing the contents of a CSV file.
     * @return    True if the operation succeeds, check the log for errors if it didn't succeed.
     */
    UFUNCTION(BlueprintCallable, Category = "Utils | DataTable", DisplayName = "Fill Data Table from CSV String")
    static bool FillDataTableFromCSVString(UDataTable* DataTable, const FString& CSVString);

    UFUNCTION(BlueprintCallable, Category = "Utils | Material", DisplayName = "Create Material DynamicInstance")
    static class UMaterialInstanceDynamic* CreateMaterialDynamicInstance(
        class UMeshComponent* _Mesh, const FString& _SlotName);

    UFUNCTION(BlueprintCallable)
    static void UpdateDecalSizeWithScale(class UDecalComponent* Decal, const FVector& Scale);

    UFUNCTION(BlueprintPure, Category = "Utils | Config")
    static bool GetLogoConfig(FString& LogoConfig);

    UFUNCTION(BlueprintPure, Category = "Utils | Config")
    static bool CompareTexture(UTexture2D* Tex1, UTexture2D* Tex2);
};
