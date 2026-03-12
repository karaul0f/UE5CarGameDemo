// CarPawn.cpp - Arcade physics car. No ChaosVehicles required.
// "Do you like what you see?" - Billy Herrington

#include "CarPawn.h"
#include "EjectedDriver.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "GameFramework/PlayerController.h"
#include "UObject/ConstructorHelpers.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

ACarPawn::ACarPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	// ---- Physics collision body (invisible box) ----
	VehicleCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("VehicleCollision"));
	SetRootComponent(VehicleCollision);
	VehicleCollision->SetBoxExtent(FVector(200.f, 100.f, 50.f));
	VehicleCollision->SetSimulatePhysics(true);
	VehicleCollision->SetEnableGravity(true);
	VehicleCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	VehicleCollision->SetCollisionObjectType(ECC_PhysicsBody);
	VehicleCollision->SetCollisionResponseToAllChannels(ECR_Block);
	VehicleCollision->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	VehicleCollision->SetNotifyRigidBodyCollision(true);
	VehicleCollision->SetLinearDamping(0.5f);
	VehicleCollision->SetAngularDamping(5.f);
	VehicleCollision->BodyInstance.bLockXRotation = true;
	VehicleCollision->BodyInstance.bLockYRotation = true;
	VehicleCollision->BodyInstance.COMNudge = FVector(0.f, 0.f, -30.f);
	VehicleCollision->SetHiddenInGame(true);

	// ---- Car body visual ----
	CarBodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CarBodyMesh"));
	CarBodyMesh->SetupAttachment(VehicleCollision);
	CarBodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Try PS1 car mesh, fall back to cube
	static ConstructorHelpers::FObjectFinder<UStaticMesh> PS1CarFinder(
		TEXT("/Game/Fab/PS1-Style_Abandoned_Convertible_Car/ps1_style_abandoned_convertible_car/StaticMeshes/ps1_style_abandoned_convertible_car.ps1_style_abandoned_convertible_car"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(
		TEXT("/Engine/BasicShapes/Cube.Cube"));

	if (PS1CarFinder.Succeeded())
	{
		CarBodyMesh->SetStaticMesh(PS1CarFinder.Object);
		CarBodyMesh->SetRelativeLocation(FVector(0.f, 0.f, -50.f));
	}
	else if (CubeFinder.Succeeded())
	{
		CarBodyMesh->SetStaticMesh(CubeFinder.Object);
		CarBodyMesh->SetRelativeScale3D(FVector(4.f, 2.f, 0.8f));
	}

	// ---- Driver mesh (skeletal — assign in Blueprint) ----
	DriverMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("DriverMesh"));
	DriverMesh->SetupAttachment(VehicleCollision);
	DriverMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DriverMesh->SetRelativeLocation(FVector(-30.f, -25.f, 60.f));
	DriverMesh->SetRelativeScale3D(FVector(0.4f));

	// ---- Third-person camera ----
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(VehicleCollision);
	SpringArm->TargetArmLength = 700.f;
	SpringArm->SetRelativeRotation(FRotator(-20.f, 0.f, 0.f));
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 5.f;
	SpringArm->bEnableCameraRotationLag = true;
	SpringArm->CameraRotationLagSpeed = 5.f;
	SpringArm->bDoCollisionTest = true;
	SpringArm->bUsePawnControlRotation = false;
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritRoll = false;
	SpringArm->bInheritYaw = false;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);

	EjectedDriverClass = AEjectedDriver::StaticClass();
}

void ACarPawn::BeginPlay()
{
	Super::BeginPlay();

	// Low friction physics material — box simulates a car with wheels, not a slab dragging on ground
	UPhysicalMaterial* CarPhysMat = NewObject<UPhysicalMaterial>();
	CarPhysMat->Friction = 0.05f;
	CarPhysMat->Restitution = 0.3f;
	VehicleCollision->SetPhysMaterialOverride(CarPhysMat);

	VehicleCollision->SetSimulatePhysics(true);
	VehicleCollision->WakeRigidBody();
	VehicleCollision->SetMassOverrideInKg(NAME_None, VehicleMass);
	VehicleCollision->OnComponentHit.AddDynamic(this, &ACarPawn::OnVehicleHit);

	TrySetupInput();
}

// ---- Vehicle Physics ----

void ACarPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bInputFullySetup)
	{
		TrySetupInput();
	}

	TimeSinceSpawn += DeltaTime;

	if (!bIsDriverEjected)
	{
		ApplyVehiclePhysics(DeltaTime);
	}
}

void ACarPawn::ApplyVehiclePhysics(float DeltaTime)
{
	FVector Forward = GetActorForwardVector();
	FVector Right = GetActorRightVector();
	FVector Velocity = VehicleCollision->GetPhysicsLinearVelocity();
	float Speed = Velocity.Size();
	float ForwardSpeed = FVector::DotProduct(Velocity, Forward);

	// Throttle
	if (FMath::Abs(CurrentThrottle) > KINDA_SMALL_NUMBER)
	{
		float Force = EngineForce;
		if (CurrentThrottle < 0.f)
		{
			Force *= ReverseForceMultiplier;
		}
		VehicleCollision->AddForce(Forward * CurrentThrottle * Force);
	}

	// Brake
	if (bIsBraking && Speed > 10.f)
	{
		VehicleCollision->AddForce(-Velocity.GetSafeNormal() * BrakeDeceleration, NAME_None, true);
	}

	// Handbrake (stronger brake, less grip for drifting)
	if (bIsHandbraking && Speed > 10.f)
	{
		VehicleCollision->AddForce(-Velocity.GetSafeNormal() * BrakeDeceleration * 2.f, NAME_None, true);
	}

	// Steering (needs some speed to turn)
	if (FMath::Abs(CurrentSteering) > KINDA_SMALL_NUMBER && Speed > 50.f)
	{
		float SpeedFactor = FMath::Clamp(Speed / 500.f, 0.f, 1.f);
		float HighSpeedReduction = 1.f - FMath::Clamp(Speed / MaxSpeed, 0.f, 1.f) * 0.5f;
		float SteerSign = (ForwardSpeed >= 0.f) ? 1.f : -1.f;

		float EffectiveTorque = CurrentSteering * SteeringTorque * SpeedFactor * HighSpeedReduction * SteerSign;
		VehicleCollision->AddTorqueInDegrees(FVector(0.f, 0.f, EffectiveTorque), NAME_None, true);
	}

	// Lateral friction (tire grip - prevents sideways sliding)
	float LateralSpeed = FVector::DotProduct(Velocity, Right);
	float GripMultiplier = bIsHandbraking ? 0.3f : 1.f;
	VehicleCollision->AddForce(-Right * LateralSpeed * LateralFriction * GripMultiplier);

	// Speed limit
	if (Speed > MaxSpeed)
	{
		VehicleCollision->SetPhysicsLinearVelocity(Velocity.GetSafeNormal() * MaxSpeed);
	}
}

// ---- Input Setup ----

void ACarPawn::CreateDefaultInputActions()
{
	if (!AccelerateAction)
	{
		AccelerateAction = NewObject<UInputAction>(this, TEXT("IA_Accelerate"));
		AccelerateAction->ValueType = EInputActionValueType::Boolean;
	}
	if (!ReverseAction)
	{
		ReverseAction = NewObject<UInputAction>(this, TEXT("IA_Reverse"));
		ReverseAction->ValueType = EInputActionValueType::Boolean;
	}
	if (!SteeringAction)
	{
		SteeringAction = NewObject<UInputAction>(this, TEXT("IA_Steering"));
		SteeringAction->ValueType = EInputActionValueType::Axis1D;
	}
	if (!BrakeAction)
	{
		BrakeAction = NewObject<UInputAction>(this, TEXT("IA_Brake"));
		BrakeAction->ValueType = EInputActionValueType::Boolean;
	}
	if (!HandbrakeAction)
	{
		HandbrakeAction = NewObject<UInputAction>(this, TEXT("IA_Handbrake"));
		HandbrakeAction->ValueType = EInputActionValueType::Boolean;
	}
	if (!ResetDriverAction)
	{
		ResetDriverAction = NewObject<UInputAction>(this, TEXT("IA_ResetDriver"));
		ResetDriverAction->ValueType = EInputActionValueType::Boolean;
	}
	if (!LookAction)
	{
		LookAction = NewObject<UInputAction>(this, TEXT("IA_Look"));
		LookAction->ValueType = EInputActionValueType::Axis2D;
	}

	if (!DrivingMappingContext)
	{
		DrivingMappingContext = NewObject<UInputMappingContext>(this, TEXT("IMC_Driving"));

		DrivingMappingContext->MapKey(AccelerateAction, EKeys::W);
		DrivingMappingContext->MapKey(ReverseAction, EKeys::S);

		DrivingMappingContext->MapKey(SteeringAction, EKeys::D);
		FEnhancedActionKeyMapping& Left = DrivingMappingContext->MapKey(SteeringAction, EKeys::A);
		Left.Modifiers.Add(NewObject<UInputModifierNegate>(this));

		DrivingMappingContext->MapKey(BrakeAction, EKeys::SpaceBar);
		DrivingMappingContext->MapKey(HandbrakeAction, EKeys::LeftShift);
		DrivingMappingContext->MapKey(ResetDriverAction, EKeys::R);
		DrivingMappingContext->MapKey(LookAction, EKeys::Mouse2D);
	}
}

void ACarPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	TrySetupInput();
}

bool ACarPawn::TrySetupInput()
{
	if (bInputFullySetup) return true;

	CreateDefaultInputActions();

	// Need controller + subsystem for mapping context
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("CarPawn: TrySetupInput - no PlayerController yet"));
		return false;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
	if (!Subsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("CarPawn: TrySetupInput - no EnhancedInput subsystem yet"));
		return false;
	}

	// Need enhanced input component for bindings
	UEnhancedInputComponent* EIC = InputComponent
		? Cast<UEnhancedInputComponent>(InputComponent)
		: nullptr;
	if (!EIC)
	{
		UE_LOG(LogTemp, Warning, TEXT("CarPawn: TrySetupInput - no EnhancedInputComponent yet (InputComponent=%s)"),
			InputComponent ? *InputComponent->GetClass()->GetName() : TEXT("null"));
		return false;
	}

	// All prerequisites met — full send
	Subsystem->AddMappingContext(DrivingMappingContext, 0);

	// Force game-only input mode (fixes PIE focus issues)
	FInputModeGameOnly InputMode;
	PC->SetInputMode(InputMode);

	EIC->BindAction(AccelerateAction, ETriggerEvent::Triggered, this, &ACarPawn::HandleAccelerate);
	EIC->BindAction(AccelerateAction, ETriggerEvent::Completed, this, &ACarPawn::HandleAccelerate);
	EIC->BindAction(ReverseAction, ETriggerEvent::Triggered, this, &ACarPawn::HandleReverse);
	EIC->BindAction(ReverseAction, ETriggerEvent::Completed, this, &ACarPawn::HandleReverse);
	EIC->BindAction(SteeringAction, ETriggerEvent::Triggered, this, &ACarPawn::HandleSteering);
	EIC->BindAction(SteeringAction, ETriggerEvent::Completed, this, &ACarPawn::HandleSteering);
	EIC->BindAction(BrakeAction, ETriggerEvent::Triggered, this, &ACarPawn::HandleBrake);
	EIC->BindAction(BrakeAction, ETriggerEvent::Completed, this, &ACarPawn::HandleBrake);
	EIC->BindAction(HandbrakeAction, ETriggerEvent::Triggered, this, &ACarPawn::HandleHandbrake);
	EIC->BindAction(HandbrakeAction, ETriggerEvent::Completed, this, &ACarPawn::HandleHandbrake);
	EIC->BindAction(ResetDriverAction, ETriggerEvent::Started, this, &ACarPawn::HandleResetDriver);
	EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACarPawn::HandleLook);

	bInputFullySetup = true;
	UE_LOG(LogTemp, Log, TEXT("CarPawn: Input fully set up!"));
	return true;
}

// ---- Hit Detection ----

void ACarPawn::OnVehicleHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (bIsDriverEjected) return;
	if (TimeSinceSpawn < EjectionGracePeriod) return;

	const float ImpactMagnitude = NormalImpulse.Size();
	if (ImpactMagnitude >= EjectionImpactThreshold)
	{
		UE_LOG(LogTemp, Warning, TEXT("IMPACT! Force: %.0f -> EJECTING DRIVER!"), ImpactMagnitude);
		EjectDriver(Hit.ImpactNormal);
	}
}

// ---- Ejection & Reset ----

void ACarPawn::EjectDriver(FVector ImpactNormal)
{
	if (bIsDriverEjected) return;
	bIsDriverEjected = true;

	DriverMesh->SetVisibility(false);

	FVector CarVelocity = VehicleCollision->GetPhysicsLinearVelocity();
	FVector LaunchDir = (-ImpactNormal + FVector::UpVector).GetSafeNormal();
	FVector LaunchVelocity = CarVelocity * EjectionLaunchSpeedMultiplier
		+ LaunchDir * EjectionUpwardBoost
		+ FVector::UpVector * EjectionUpwardBoost;

	FVector SpawnLocation = VehicleCollision->GetComponentLocation() + FVector(0.f, 0.f, 200.f);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ActiveEjectedDriver = GetWorld()->SpawnActor<AEjectedDriver>(
		EjectedDriverClass, SpawnLocation, GetActorRotation(), SpawnParams);

	if (ActiveEjectedDriver)
	{
		// Prevent sphere from colliding with the car — otherwise it gets stuck
		// or physics freaks out from the overlap at spawn
		if (ActiveEjectedDriver->BodyMesh)
		{
			ActiveEjectedDriver->BodyMesh->IgnoreActorWhenMoving(this, true);
			VehicleCollision->IgnoreActorWhenMoving(ActiveEjectedDriver, true);
		}

		// Initialize ejected camera rotation from current car camera — smooth
		// transition, no jarring snap. "Thank you sir!" — Billy Herrington
		if (ActiveEjectedDriver->SpringArm)
		{
			ActiveEjectedDriver->SpringArm->SetWorldRotation(
				SpringArm->GetComponentRotation());
		}

		ActiveEjectedDriver->Launch(LaunchVelocity);

		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			PC->SetViewTargetWithBlend(ActiveEjectedDriver, CameraBlendTime);
		}
	}
}

void ACarPawn::ResetDriver()
{
	if (!bIsDriverEjected) return;

	if (ActiveEjectedDriver)
	{
		ActiveEjectedDriver->Destroy();
		ActiveEjectedDriver = nullptr;
	}

	DriverMesh->SetVisibility(true);
	bIsDriverEjected = false;

	CurrentThrottle = 0.f;
	CurrentSteering = 0.f;
	bIsBraking = false;
	bIsReversing = false;
	bIsHandbraking = false;

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->SetViewTargetWithBlend(this, CameraBlendTime);
	}
}

// ---- Input Handlers ----

void ACarPawn::HandleAccelerate(const FInputActionValue& Value)
{
	bool bPressed = Value.Get<bool>();
	if (bIsDriverEjected) bPressed = false;
	CurrentThrottle = bPressed ? 1.f : (bIsReversing ? -1.f : 0.f);
}

void ACarPawn::HandleReverse(const FInputActionValue& Value)
{
	bIsReversing = Value.Get<bool>();
	if (bIsDriverEjected) bIsReversing = false;
	CurrentThrottle = bIsReversing ? -1.f : (CurrentThrottle > 0.f ? 1.f : 0.f);
}

void ACarPawn::HandleSteering(const FInputActionValue& Value)
{
	CurrentSteering = bIsDriverEjected ? 0.f : Value.Get<float>();
}

void ACarPawn::HandleBrake(const FInputActionValue& Value)
{
	bIsBraking = bIsDriverEjected ? false : Value.Get<bool>();
}

void ACarPawn::HandleHandbrake(const FInputActionValue& Value)
{
	bIsHandbraking = bIsDriverEjected ? false : Value.Get<bool>();
}

void ACarPawn::HandleResetDriver(const FInputActionValue& Value)
{
	if (bIsDriverEjected)
	{
		ResetDriver();
	}
}

void ACarPawn::HandleLook(const FInputActionValue& Value)
{
	FVector2D LookValue = Value.Get<FVector2D>();

	// When ejected, control the EjectedDriver's camera (absolute rotation)
	if (bIsDriverEjected && ActiveEjectedDriver && ActiveEjectedDriver->SpringArm)
	{
		FRotator CurrentRot = ActiveEjectedDriver->SpringArm->GetComponentRotation();
		CurrentRot.Yaw += LookValue.X;
		CurrentRot.Pitch = FMath::Clamp(CurrentRot.Pitch - LookValue.Y, -80.f, 10.f);
		ActiveEjectedDriver->SpringArm->SetWorldRotation(CurrentRot);
		return;
	}

	FRotator CurrentRot = SpringArm->GetRelativeRotation();
	CurrentRot.Yaw += LookValue.X;
	CurrentRot.Pitch = FMath::Clamp(CurrentRot.Pitch - LookValue.Y, -80.f, 10.f);
	SpringArm->SetRelativeRotation(CurrentRot);
}
