// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "UxtTestUtils.h"

#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Tests/AutomationCommon.h"
#include "Engine.h"
#include "EngineUtils.h"
#include "Features/IModularFeatures.h"

#include "Input/UxtNearPointerComponent.h"
#include "PointerTestSequence.h"
#include "UxtTestHandTracker.h"

FUxtTestHandTracker UxtTestUtils::TestHandTracker;

/** Cached hand tracker implementation to restore after tests are completed. */
IUxtHandTracker* UxtTestUtils::MainHandTracker = nullptr;

UWorld* UxtTestUtils::LoadMap(const FString& MapName)
{
	// Syncronous map load is only supported in editor
#if WITH_EDITOR
	if (AutomationOpenMap(MapName))
	{
		return GetTestWorld();
	}
#else
	check(false);
#endif
	return nullptr;
}

void UxtTestUtils::ExitGame()
{
	// Copied from FExitGameCommand 
	if (APlayerController* TargetPC = UGameplayStatics::GetPlayerController(GetTestWorld(), 0))
	{
		TargetPC->ConsoleCommand(TEXT("Exit"), true);
	}
}

// Copy of the hidden method GetAnyGameWorld() in AutomationCommon.cpp.
// Marked as temporary there, hence, this one is temporary, too.
UWorld* UxtTestUtils::GetTestWorld() {
	const TIndirectArray<FWorldContext>& WorldContexts = GEngine->GetWorldContexts();
	for (const FWorldContext& Context : WorldContexts) {
		if (((Context.WorldType == EWorldType::PIE) || (Context.WorldType == EWorldType::Game))
			&& (Context.World() != nullptr)) {
			return Context.World();
		}
	}

	return nullptr;
}

UWorld* UxtTestUtils::CreateTestWorld()
{
	UWorld *world = UWorld::CreateWorld(EWorldType::PIE, true, TEXT("TestWorld"));
	return world;
}

UStaticMeshComponent* UxtTestUtils::CreateBoxStaticMesh(AActor* Owner, FVector Scale)
{
	UStaticMeshComponent* Component = NewObject<UStaticMeshComponent>(Owner);
	UStaticMesh* Mesh = LoadObject<UStaticMesh>(Owner, TEXT("/Engine/BasicShapes/Cube.Cube"));
	Component->SetStaticMesh(Mesh);
	Component->SetWorldScale3D(Scale);
	return Component;
}

FUxtTestHandTracker& UxtTestUtils::GetTestHandTracker()
{
	return TestHandTracker;
}

FUxtTestHandTracker& UxtTestUtils::EnableTestHandTracker()
{
	check(MainHandTracker == nullptr);

	// Remove and cache current hand tracker.
	MainHandTracker = &IModularFeatures::Get().GetModularFeature<IUxtHandTracker>(IUxtHandTracker::GetModularFeatureName());
	if (MainHandTracker)
	{
		IModularFeatures::Get().UnregisterModularFeature(IUxtHandTracker::GetModularFeatureName(), MainHandTracker);
	}

	// Register the test hand tracker.
	IModularFeatures::Get().RegisterModularFeature(IUxtHandTracker::GetModularFeatureName(), &TestHandTracker);

	// Reset test hand tracker defaults
	TestHandTracker = FUxtTestHandTracker();

	return TestHandTracker;
}

void UxtTestUtils::DisableTestHandTracker()
{
	// Unregister the test hand tracker.
	IModularFeatures::Get().UnregisterModularFeature(IUxtHandTracker::GetModularFeatureName(), &TestHandTracker);

	// Re-register the original hand tracker implementation
	if (MainHandTracker)
	{
		IModularFeatures::Get().RegisterModularFeature(IUxtHandTracker::GetModularFeatureName(), MainHandTracker);
		MainHandTracker = nullptr;
	}
}

UUxtNearPointerComponent* UxtTestUtils::CreateNearPointer(UWorld *World, FName Name, const FVector &Location, bool IsGrasped, bool AddMeshVisualizer)
{
	FActorSpawnParameters p;
	p.Name = Name;
	AActor *hand = World->SpawnActor<AActor>(p);

	USceneComponent* root = NewObject<USceneComponent>(hand);
	hand->SetRootComponent(root);
	root->SetWorldLocation(Location);
	root->RegisterComponent();

	UUxtNearPointerComponent* pointer = NewObject<UUxtNearPointerComponent>(hand);
	pointer->RegisterComponent();

	UxtTestUtils::GetTestHandTracker().bIsGrabbing = IsGrasped;
	UxtTestUtils::GetTestHandTracker().TestPosition = Location;

	if (AddMeshVisualizer)
	{
		UStaticMeshComponent* mesh = CreateBoxStaticMesh(hand, FVector(0.01f));
		mesh->SetupAttachment(root);
		mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		mesh->RegisterComponent();
	}

	return pointer;
}

UTestGrabTarget* UxtTestUtils::CreateNearPointerTarget(UWorld *World, const FVector &Location, const FString &meshFilename, float meshScale)
{
	AActor *actor = World->SpawnActor<AActor>();

	USceneComponent *root = NewObject<USceneComponent>(actor);
	actor->SetRootComponent(root);
	root->SetWorldLocation(Location);
	root->RegisterComponent();

	UTestGrabTarget *testTarget = NewObject<UTestGrabTarget>(actor);
	testTarget->RegisterComponent();

	if (!meshFilename.IsEmpty())
	{
		UStaticMeshComponent *mesh = NewObject<UStaticMeshComponent>(actor);
		mesh->SetupAttachment(actor->GetRootComponent());
		mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		mesh->SetCollisionProfileName(TEXT("OverlapAll"));
		mesh->SetGenerateOverlapEvents(true);

		UStaticMesh *meshAsset = LoadObject<UStaticMesh>(actor, *meshFilename);
		mesh->SetStaticMesh(meshAsset);
		mesh->SetRelativeScale3D(FVector::OneVector * meshScale);

		mesh->RegisterComponent();
	}

	return testTarget;
}

UTestGrabTarget* UxtTestUtils::CreateNearPointerBackgroundTarget(UWorld* World)
{
	AActor* actor = World->SpawnActor<AActor>();

	UTestGrabTarget* testTarget = NewObject<UTestGrabTarget>(actor);
	testTarget->RegisterComponent();

	return testTarget;
}


bool FUxtDisableTestHandTrackerCommand::Update()
{
	UxtTestUtils::DisableTestHandTracker();
	return true;
}
