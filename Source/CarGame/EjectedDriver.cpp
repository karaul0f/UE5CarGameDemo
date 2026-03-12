// EjectedDriver.cpp - Physics body goes full send through the windshield.
// "That's the right way to do it!" - Van Darkholme

#include "EjectedDriver.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

AEjectedDriver::AEjectedDriver()
{
	// Body mesh - skeletal, will be copied from CarPawn's DriverMesh at spawn
	BodyMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BodyMesh"));
	SetRootComponent(BodyMesh);
	BodyMesh->SetSimulatePhysics(true);
	BodyMesh->SetEnableGravity(true);
	BodyMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BodyMesh->SetCollisionResponseToAllChannels(ECR_Block);
	BodyMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	BodyMesh->SetNotifyRigidBodyCollision(true);
	BodyMesh->BodyInstance.bUseCCD = true;

	// Spring arm for smooth camera follow — absolute rotation so it doesn't
	// tumble with the physics body like a dungeon slave
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(BodyMesh);
	SpringArm->TargetArmLength = 500.f;
	SpringArm->SetAbsolute(false, true, false);
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 3.f;
	SpringArm->bEnableCameraRotationLag = true;
	SpringArm->CameraRotationLagSpeed = 3.f;
	SpringArm->bDoCollisionTest = true;
	SpringArm->SetWorldRotation(FRotator(-25.f, 0.f, 0.f));

	// Camera
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
}

void AEjectedDriver::Launch(FVector LaunchVelocity)
{
	if (BodyMesh)
	{
		BodyMesh->SetPhysicsLinearVelocity(LaunchVelocity);

		// Add random angular velocity for that sweet tumbling ragdoll effect
		FVector AngularVelocity = FMath::VRand() * 720.f;
		BodyMesh->SetPhysicsAngularVelocityInDegrees(AngularVelocity);
	}
}
