// Fill out your copyright notice in the Description page of Project Settings.


#include "spaceMapper.h"
#include "Kismet/KismetSystemLibrary.h"

// Sets default values
ASpaceMapper::ASpaceMapper()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	boundsBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Bounds Box"));
	SetRootComponent(boundsBox);

	boundsBox->SetBoxExtent(FVector(500.0f, 500.0f, 500.0f));
	boundsBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	boundsBox->SetHiddenInGame(true);

	reset();
}

// Called when the game starts or when spawned
void ASpaceMapper::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASpaceMapper::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASpaceMapper::reset() {
	nodes.Empty();
}

int32 ASpaceMapper::makeNode(const FVector& minBound, const FVector& maxBound)
{
	FSpatialNode node;
	node.minBound = minBound;
	node.maxBound = maxBound;
	node.firstChildIndex = INDEX_NONE;

	const FVector boxBounds = maxBound - minBound;

	const float boxVolume = boxBounds.X * boxBounds.Y * boxBounds.Z;

	const FVector boxCenter = (minBound + maxBound) * 0.5f;
	const FVector boxExtent = boxBounds * 0.5f;

	TArray<TEnumAsByte<EObjectTypeQuery>> objectTypes;
	objectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));
	objectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));

	TArray<AActor*> actorsToIgnore;
	actorsToIgnore.Add(this);

	TArray<AActor*> overlappingActors;

	const bool foundOverlap = UKismetSystemLibrary::BoxOverlapActors(
		this,
		boxCenter,
		boxExtent,
		objectTypes,
		nullptr,
		actorsToIgnore,
		overlappingActors
	);

	if (foundOverlap) {
		node.status = boxVolume <= minVolume
			? ESpatialStatus::BLOCKED
			: ESpatialStatus::MIXED;
	}
	else {
		node.status = ESpatialStatus::CLEAR;
	}

	return nodes.Add(node);
}

void ASpaceMapper::mapSpace()
{
	reset();

	if (!boundsBox) {
		return;
	}

	const FVector boxLocation = boundsBox->GetComponentLocation();
	const FVector boxExtent = boundsBox->GetScaledBoxExtent();

	const FVector rootMinBound = boxLocation - boxExtent;
	const FVector rootMaxBound = boxLocation + boxExtent;

	const int32 rootIndex = makeNode(rootMinBound, rootMaxBound);

	TQueue<int32> nodesLeft;
	nodesLeft.Enqueue(rootIndex);

	int32 curNodeIndex = INDEX_NONE;

	while (nodesLeft.Dequeue(curNodeIndex)) {
		if (!nodes.IsValidIndex(curNodeIndex)) {
			continue;
		}

		if (nodes[curNodeIndex].status != ESpatialStatus::MIXED) {
			continue;
		}

		// Copy the parent node before adding children.
		// TArray::Add can reallocate, so don't keep a reference while adding.
		const FSpatialNode parentCopy = nodes[curNodeIndex];

		const int32 firstChildIndex = nodes.Num();
		nodes[curNodeIndex].firstChildIndex = firstChildIndex;

		for (int32 i = 0; i < 8; i++) {
			FVector childMinBound;
			FVector childMaxBound;

			calculateBounds(parentCopy, i, childMinBound, childMaxBound);

			const int32 childIndex = makeNode(childMinBound, childMaxBound);
			nodesLeft.Enqueue(childIndex);
		}
	}
}

void ASpaceMapper::calculateBounds(
	const FSpatialNode& parentNode,
	int childIndex,
	FVector& outMinBound,
	FVector& outMaxBound
) const
{
	const FVector parentMin = parentNode.minBound;
	const FVector parentMax = parentNode.maxBound;
	const FVector parentCenter = (parentMin + parentMax) * 0.5f;

	const bool highX = (childIndex & 1) != 0;
	const bool highY = (childIndex & 2) != 0;
	const bool highZ = (childIndex & 4) != 0;

	outMinBound.X = highX ? parentCenter.X : parentMin.X;
	outMaxBound.X = highX ? parentMax.X : parentCenter.X;

	outMinBound.Y = highY ? parentCenter.Y : parentMin.Y;
	outMaxBound.Y = highY ? parentMax.Y : parentCenter.Y;

	outMinBound.Z = highZ ? parentCenter.Z : parentMin.Z;
	outMaxBound.Z = highZ ? parentMax.Z : parentCenter.Z;
}

void ASpaceMapper::drawMap() {
	curDrawCalls = 0;
	drawNode(0, 0);
}

void ASpaceMapper::drawNode(uint32 curIndex, uint32 depth) {
	if (curDrawCalls >= maxDrawCalls)
		return;
	if (depth > maxDrawDepth)
		return;
	if (!nodes.IsValidIndex(curIndex))
		return;
	const FSpatialNode curNode = nodes[curIndex];

	if (depth < minDrawDepth) {
		if (curNode.firstChildIndex != INDEX_NONE) {
			for (int i = 0; i < 8; i++) {
				drawNode(curNode.firstChildIndex + i, depth + 1);
			}
		}
		return;
	}
	const FVector curNodeMin = curNode.minBound;
	const FVector curNodeMax = curNode.maxBound;
	const FVector curNodeCenter = (curNodeMin + curNodeMax) * 0.5f;
	const FVector curNodeBounds = curNodeMax - curNodeMin;
	const FVector curNodeExtent = (curNodeBounds) * 0.5f;
	if (curNode.status == ESpatialStatus::MIXED) {
		if(drawMixed) {
			DrawDebugBox(GetWorld(), curNodeCenter, curNodeExtent, mixedOutlineColor, mixedOutlinePersistance, mixedOutlineDrawTime, (uint8)0U, mixedOutlineThickness);
			curDrawCalls++;
		}
		for (int i = 0; i < 8; i++) {
			drawNode(curNode.firstChildIndex + i, depth + 1);
		}
	}
	else {
		if (!shouldDrawLeaves)
			return;
		if (curNode.status == ESpatialStatus::CLEAR && !shouldDrawClear)
			return;

		FColor drawColor = clearDrawColor;
		if (curNode.status == ESpatialStatus::BLOCKED) {
			if (!shouldDrawBlocked)
				return;
			drawColor = blockedDrawColor;
		}
		DrawDebugSolidBox(GetWorld(), curNodeCenter, curNodeExtent, drawColor, leafDrawPersistance, leafDrawTime);
		curDrawCalls++;
	}
}