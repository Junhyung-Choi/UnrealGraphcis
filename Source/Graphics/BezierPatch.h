// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BezierPatch.generated.h"


UCLASS()
class GRAPHICS_API ABezierPatch : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABezierPatch();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual FVector GetVector(int rowIndex, int colIndex, int matRowSize, TArray<FVector>& matrix);

	virtual void SetVector(FVector vector, int rowIndex, int colIndex, int matRowSize, TArray<FVector>& matrix);

	virtual FVector CalculateBezierPoint(int rowTime, int colTime);

	virtual void SetTimeSliceMatrix();

	virtual double CalculatePolynomial(int size, int count, int time);

	UPROPERTY(EditAnywhere);
	UStaticMeshComponent* Mesh;

	UPROPERTY(EditAnywhere, Meta = (MakeEditWidget = true));
	TArray<FVector> controlPointsMatrix;

	int32 bernsteinBasis[10][10] =
	{
		{1,   0,   0,   0,   0,   0,   0,   0,   0,  0},
		{1,   1,   0,   0,   0,   0,   0,   0,   0,  0},
		{1,   2,   1,   0,   0,   0,   0,   0,   0,  0},
		{1,   3,   3,   1,   0,   0,   0,   0,   0,  0},
		{1,   4,   6,   4,   1,   0,   0,   0,   0,  0},
		{1,   5,  10,  10,   5,   1,   0,   0,   0,  0},
		{1,   6,  15,  20,  15,   6,   1,   0,   0,  0},
		{1,   7,  21,  35,  35,  21,   7,   1,   0,  0},
		{1,   8,  28,  56,  70,  56,  28,   8,   1,  0},
		{1,   9,  36,  84, 126, 126,  84,  36,   9,  1},
	};

	TArray<FVector> timeSlicePointsMatrix;

	int32 timeslice = 1024;

	int32 controlRow = 3;
	int32 controlCol = 4;


	int32 maxDepth = 3;
	int32 split = 8;

	TArray<FTreeNode> BVH;

	virtual void SetBVH();

	virtual void SetBVHNode(int32 index);

	virtual void FindBVHLeaf();

	UPROPERTY(EditAnywhere);
	AActor* bullet;
};


USTRUCT(Atomic, BlueprintType)
struct FTreeNode
{
	GENERATED_BODY()

public:
		int minX;
		int maxX;
		int minY;
		int maxY;
		int minZ;
		int maxZ;
};