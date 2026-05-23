// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "spaceMapper.generated.h"

UENUM()
enum class ESpatialStatus : uint8 {
	CLEAR,
	MIXED,
	BLOCKED,
};

USTRUCT()
struct FSpatialNode
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	FVector minBound = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere)
	FVector maxBound = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere)
	ESpatialStatus status = ESpatialStatus::CLEAR;

	UPROPERTY(VisibleAnywhere)
	int32 firstChildIndex = INDEX_NONE;
};

UCLASS()
class UE3DNAVIGATION_API ASpaceMapper : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASpaceMapper();

	UPROPERTY(EditAnywhere, Category = "Controls|Parameters|Limits")
	float minVolume = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Controls|Parameters|Drawing")
	bool drawMixed = true;

	UPROPERTY(EditAnywhere, Category = "Controls|Parameters|Drawing")
	FColor mixedOutlineColor = FColor::Yellow;

	UPROPERTY(EditAnywhere, Category = "Controls|Parameters|Drawing")
	float mixedOutlineThickness = 5.0f;

	UPROPERTY(EditAnywhere, Category = "Controls|Parameters|Drawing")
	bool mixedOutlinePersistance = false;

	UPROPERTY(EditAnywhere, Category = "Controls|Parameters|Drawing")
	float mixedOutlineDrawTime = 5.0f;

	UPROPERTY(EditAnywhere, Category = "Controls|Parameters|Drawing")
	FColor clearDrawColor = FColor::Green;

	UPROPERTY(EditAnywhere, Category = "Controls|Parameters|Drawing")
	FColor blockedDrawColor = FColor::Red;

	UPROPERTY(EditAnywhere, Category = "Controls|Parameters|Drawing")
	bool shouldDrawBlocked = true;

	UPROPERTY(EditAnywhere, Category = "Controls|Parameters|Drawing")
	bool shouldDrawClear = true;

	UPROPERTY(EditAnywhere, Category = "Controls|Parameters|Drawing")
	float leafDrawTime = 5.0f;

	UPROPERTY(EditAnywhere, Category = "Controls|Parameters|Drawing")
	bool shouldDrawLeaves = true;

	UPROPERTY(EditAnywhere, Category = "Controls|Parameters|Drawing")
	bool leafDrawPersistance = false;

	UPROPERTY(EditAnywhere, Category = "Controls|Parameters|Drawing")
	uint32 minDrawDepth = 0;

	UPROPERTY(EditAnywhere, Category = "Controls|Parameters|Drawing")
	uint32 maxDrawDepth = 3;

	UPROPERTY(EditAnywhere, Category = "Controls|Parameters|Drawing")
	uint32 maxDrawCalls = 50;

	UPROPERTY(EditAnywhere, Category = "Controls|Parameters|Limits")
	TObjectPtr<UBoxComponent> boundsBox;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Data")
	TArray<FSpatialNode> nodes;

	int32 makeNode(const FVector& minBound, const FVector& maxBound);

	virtual void calculateBounds(const FSpatialNode& parentNode, int childIndex, FVector& minBoundOut, FVector& maxBoundOut) const;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(CallInEditor, Category = "Controls")
	void reset();

	UFUNCTION(CallInEditor, Category = "Controls")
	void mapSpace();

	UFUNCTION(CallInEditor, Category = "Controls")
	void drawMap();

private:
	void drawNode(uint32 curNode, uint32 depth);
	uint32 curDrawCalls = 0;
};
