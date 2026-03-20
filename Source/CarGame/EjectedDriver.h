// EjectedDriver.h - The driver that gets yeeted out of the car like a true FlatOut champion.
// "Do you like what you see?" - Billy Herrington

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EjectedDriver.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UStaticMeshComponent;
class USkeletalMeshComponent;

/**
 * Actor representing the driver ejected from the vehicle on heavy impact.
 * Spawned with physics simulation and launched into the air.
 * Invisible sphere handles physics, wolf skeletal mesh provides the visuals.
 */
UCLASS()
class CARGAME_API AEjectedDriver : public AActor
{
	GENERATED_BODY()

public:
	AEjectedDriver();

	/** Invisible sphere for physics collision */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> BodyMesh;

	/** Visual wolf mesh — the actual driver that gets yeeted */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> DriverVisualMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> Camera;

	/** Launch the body with the given velocity and add random tumble spin */
	UFUNCTION(BlueprintCallable, Category = "Ejection")
	void Launch(FVector LaunchVelocity);
};
