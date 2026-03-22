// EjectedDriver.h - The driver that gets yeeted out of the car like a true FlatOut champion.
// "Do you like what you see?" - Billy Herrington

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EjectedDriver.generated.h"

class USpringArmComponent;
class UCameraComponent;
class USkeletalMeshComponent;

/**
 * Actor representing the driver ejected from the vehicle on heavy impact.
 * Ragdoll via PhysicsAsset on the skeletal mesh — pure physics, no proxy shapes.
 */
UCLASS()
class CARGAME_API AEjectedDriver : public AActor
{
	GENERATED_BODY()

public:
	AEjectedDriver();

	/** Skeletal mesh with ragdoll physics — the driver himself */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> RagdollMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> Camera;

	/** Launch the ragdoll with the given velocity and add random tumble spin */
	UFUNCTION(BlueprintCallable, Category = "Ejection")
	void Launch(FVector LaunchVelocity);
};
