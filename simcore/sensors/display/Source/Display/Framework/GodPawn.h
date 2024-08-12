// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GodPawn.generated.h"

class UInputComponent;
class UPawnMovementComponent;
class USphereComponent;
class UStaticMeshComponent;

UCLASS(config = Game, Blueprintable, BlueprintType)
class DISPLAY_API AGodPawn : public APawn
{
    GENERATED_BODY()

public:
    // Sets default values for this pawn's properties
    AGodPawn();

    /** If true, adds default input bindings for movement and camera look. */
    UPROPERTY(Category = Pawn, EditAnywhere, BlueprintReadOnly)
    uint32 bAddDefaultMovementBindings : 1;

protected:
    /** DefaultPawn movement component */
    UPROPERTY(Category = Pawn, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    UPawnMovementComponent* MovementComponent;

private:
    /** DefaultPawn collision component */
    UPROPERTY(Category = Pawn, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    USphereComponent* CollisionComponent;

    /** The mesh associated with this Pawn. */
    UPROPERTY(Category = Pawn, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* MeshComponent;

    UPROPERTY(Config)
    float moveSpeed = 1.f;

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Called to bind functionality to input
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    /**
     * Input callback to move forward in local space (or backward if Val is negative).
     * @param Val Amount of movement in the forward direction (or backward if negative).
     * @see APawn::AddMovementInput()
     */
    UFUNCTION(BlueprintCallable, Category = "Pawn")
    virtual void MoveForward(float Val);

    /**
     * Input callback to strafe right in local space (or left if Val is negative).
     * @param Val Amount of movement in the right direction (or left if negative).
     * @see APawn::AddMovementInput()
     */
    UFUNCTION(BlueprintCallable, Category = "Pawn")
    virtual void MoveRight(float Val);

    /**
     * Input callback to move up in world space (or down if Val is negative).
     * @param Val Amount of movement in the world up direction (or down if negative).
     * @see APawn::AddMovementInput()
     */
    UFUNCTION(BlueprintCallable, Category = "Pawn")
    virtual void MoveUp_World(float Val);

    /**
     * Called via input to turn at a given rate.
     * @param Rate    This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
     */
    UFUNCTION(BlueprintCallable, Category = "Pawn")
    virtual void TurnAtRate(float Rate);

    /**
     * Called via input to look up at a given rate (or down if Rate is negative).
     * @param Rate    This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
     */
    UFUNCTION(BlueprintCallable, Category = "Pawn")
    virtual void LookUpAtRate(float Rate);

    /** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pawn")
    float BaseTurnRate;

    /** Base lookup rate, in deg/sec. Other scaling may affect final lookup rate. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pawn")
    float BaseLookUpRate;
};
