// EjectedDriver.cpp - Physics body goes full send through the windshield.
// "That's the right way to do it!" - Van Darkholme

#include "EjectedDriver.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "UObject/ConstructorHelpers.h"

AEjectedDriver::AEjectedDriver()
{
	// Skeletal mesh as root — ragdoll physics, no proxy shapes, pure muscle
	RagdollMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RagdollMesh"));
	SetRootComponent(RagdollMesh);
	RagdollMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	RagdollMesh->SetCollisionResponseToAllChannels(ECR_Block);
	RagdollMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	RagdollMesh->SetEnableGravity(true);

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> WolfMeshFinder(
		TEXT("/Game/Fab/Realistic_Wolf_3D_Model_2_0_Demo_Free_Download_/wolf_demo.wolf_demo"));
	if (WolfMeshFinder.Succeeded())
	{
		RagdollMesh->SetSkeletalMesh(WolfMeshFinder.Object);
	}

	// Spring arm for smooth camera follow — absolute rotation so it doesn't
	// tumble with the physics body like a dungeon slave
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RagdollMesh);
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
	if (RagdollMesh)
	{
		// Enable ragdoll — "it's all physics from here, boy!"
		RagdollMesh->SetSimulatePhysics(true);
		RagdollMesh->SetAllBodiesSimulatePhysics(true);
		RagdollMesh->SetAllBodiesPhysicsBlendWeight(1.0f);
		RagdollMesh->WakeAllRigidBodies();

		// Apply velocity to all bones for proper ragdoll launch
		RagdollMesh->SetAllPhysicsLinearVelocity(LaunchVelocity);

		// Random angular velocity on each bone for tumbling ragdoll goodness
		FVector AngularVelocity = FMath::VRand() * 720.f;
		RagdollMesh->SetAllPhysicsAngularVelocityInDegrees(AngularVelocity);
	}
}
