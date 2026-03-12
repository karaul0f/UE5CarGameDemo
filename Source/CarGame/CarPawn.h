// CarPawn.h - Arcade car with simple physics and FlatOut-style ejection.
// "Take it boy!" - the wall to the driver

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "CarPawn.generated.h"

class UBoxComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class AEjectedDriver;

UCLASS()
class CARGAME_API ACarPawn : public APawn
{
	GENERATED_BODY()

public:
	ACarPawn();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	// ---- Components ----

	/** Invisible box collision - the physics body of the car */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle")
	TObjectPtr<UBoxComponent> VehicleCollision;

	/** Visual mesh for the car body */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle")
	TObjectPtr<UStaticMeshComponent> CarBodyMesh;

	/** Visual mesh for the driver sitting inside */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Driver")
	TObjectPtr<UStaticMeshComponent> DriverMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> Camera;

	// ---- Vehicle Physics ----

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|Physics")
	float EngineForce = 1600000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|Physics")
	float ReverseForceMultiplier = 0.6f;

	/** Degrees/s^2 angular acceleration for steering (mass-independent) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|Physics")
	float SteeringTorque = 360.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|Physics")
	float BrakeDeceleration = 1500.f;

	/** Max speed in cm/s (3500 = ~126 km/h) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|Physics")
	float MaxSpeed = 3500.f;

	/** Lateral velocity damping - simulates tire grip */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|Physics")
	float LateralFriction = 80.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|Physics")
	float VehicleMass = 1500.f;

	// ---- Ejection Settings ----

	/** Seconds after spawn before ejection can trigger (prevents spawn-drop ejection) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ejection")
	float EjectionGracePeriod = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ejection")
	float EjectionImpactThreshold = 200000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ejection")
	float EjectionLaunchSpeedMultiplier = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ejection")
	float EjectionUpwardBoost = 800.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ejection")
	float CameraBlendTime = 0.3f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Ejection")
	TSubclassOf<AEjectedDriver> EjectedDriverClass;

	// ---- Input (auto-created if not assigned in Blueprint) ----

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DrivingMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> AccelerateAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> ReverseAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> SteeringAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> BrakeAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> HandbrakeAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> ResetDriverAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	// ---- State ----

	UPROPERTY(BlueprintReadOnly, Category = "Ejection")
	bool bIsDriverEjected = false;

	UPROPERTY(BlueprintReadOnly, Category = "Ejection")
	TObjectPtr<AEjectedDriver> ActiveEjectedDriver = nullptr;

	UFUNCTION(BlueprintCallable, Category = "Ejection")
	void EjectDriver(FVector ImpactNormal);

	UFUNCTION(BlueprintCallable, Category = "Ejection")
	void ResetDriver();

private:
	float CurrentThrottle = 0.f;
	float CurrentSteering = 0.f;
	bool bIsBraking = false;
	bool bIsReversing = false;
	float TimeSinceSpawn = 0.f;
	bool bIsHandbraking = false;
	bool bInputFullySetup = false;

	bool TrySetupInput();
	void CreateDefaultInputActions();
	void ApplyVehiclePhysics(float DeltaTime);

	UFUNCTION()
	void OnVehicleHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	void HandleAccelerate(const FInputActionValue& Value);
	void HandleReverse(const FInputActionValue& Value);
	void HandleSteering(const FInputActionValue& Value);
	void HandleBrake(const FInputActionValue& Value);
	void HandleHandbrake(const FInputActionValue& Value);
	void HandleResetDriver(const FInputActionValue& Value);
	void HandleLook(const FInputActionValue& Value);
};
