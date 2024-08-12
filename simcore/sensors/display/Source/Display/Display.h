// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#define GETENUMSTRING(etype, evalue)                                                         \
    ((FindObject<UEnum>(ANY_PACKAGE, TEXT(etype), true) != nullptr)                          \
            ? FindObject<UEnum>(ANY_PACKAGE, TEXT(etype), true)->GetEnumName((int32) evalue) \
            : FString("Invalid - are you sure enum uses UENUM() macro?"))
