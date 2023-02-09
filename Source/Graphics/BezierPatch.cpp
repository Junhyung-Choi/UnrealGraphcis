// Fill out your copyright notice in the Description page of Project Settings.


#include "BezierPatch.h"

#include "DrawDebugHelpers.h"

// Sets default values
ABezierPatch::ABezierPatch()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));

	RootComponent = Mesh;

	FVector initVector = FVector(0,0,0);
	
	controlPointsMatrix.Init(initVector, controlRow * controlCol);
	timeSlicePointsMatrix.Init(initVector, 1024 * 1024);

	FTreeNode initNode = FTreeNode();
	BVH.Init(initNode, 1 + 8 + 64 + 512);
}

// Called when the game starts or when spawned
void ABezierPatch::BeginPlay()
{
	Super::BeginPlay();

	SetTimeSliceMatrix();

	SetBVH();

	FindBVHLeaf();
}
// Called every frame
void ABezierPatch::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

FVector ABezierPatch::GetVector(int rowIndex, int colIndex, int matRowSize, TArray<FVector>& matrix)
{
	int realIndex = rowIndex * matRowSize + colIndex;
	return matrix[realIndex];
}

void ABezierPatch::SetVector(FVector vector, int rowIndex, int colIndex, int matRowSize, TArray<FVector>& matrix)
{
	int realIndex = rowIndex * matRowSize + colIndex;
	matrix[realIndex] = vector;
}



FVector ABezierPatch::CalculateBezierPoint(int rowTime, int colTime)
{	
	FVector result = FVector(0, 0, 0);

	for (int32 i = 0; i < controlRow; i++)
	{
		for (int32 j = 0; j < controlCol; j++)
		{
			FVector controlPoint = GetVector(i, j, controlCol, controlPointsMatrix);

			float rowPolynomial, colPolynomial;
			
			rowPolynomial = CalculatePolynomial(controlRow, i, rowTime);
			colPolynomial = CalculatePolynomial(controlCol, j, colTime);
			
			result += controlPoint * rowPolynomial * colPolynomial;
		}
	}

	return result;
}

void ABezierPatch::SetTimeSliceMatrix()
{
	for (int32 i = 0; i < timeslice; i += 4)
	{
		for (int32 j = 0; j < timeslice; j += 4)
		{
			FVector sliceVector = CalculateBezierPoint(i, j);
			SetVector(sliceVector, i, j, timeslice, timeSlicePointsMatrix);

			DrawDebugPoint(GetWorld(), sliceVector, 2, FColor(50, 25, 50), true);
		}
	}
}

double ABezierPatch::CalculatePolynomial(int size, int count, int time)
{
	double result = 0;
	int first, second;
	
	result = bernsteinBasis[size - 1][count];

	if (time == 0)
	{
		if (count != 0)
		{
			result = 0;
			return result;
		}
	}

	if (time == timeslice)
	{
		if (count != size - 1)
		{
			result = 0;
			return result;
		}
	}
	
	first = timeslice - time;
	second = time;

	result *= pow(first / (double)timeslice, size - 1 - count);
	result *= pow(second / (double)timeslice,count);

	return result;
}

void ABezierPatch::SetBVH()
{
	int minX =  123456789, minY =  123456789, minZ =  123456789;
	int	maxX = -123456789, maxY = -123456789, maxZ = -123456789;

	for (int32 i = 0; i < controlPointsMatrix.Num(); i++)
	{
		FVector point = controlPointsMatrix[i];
		minX = FMath::Min(minX, point.X);
		minY = FMath::Min(minY, point.Y);
		minZ = FMath::Min(minZ, point.Z);
		maxX = FMath::Max(maxX, point.X);
		maxY = FMath::Max(maxY, point.Y);
		maxZ = FMath::Max(maxZ, point.Z);
	}

	FTreeNode node = FTreeNode();
	node.minX = minX; node.minY = minY; node.minZ = minZ;
	node.maxX = maxX; node.maxY = maxY; node.maxZ = maxZ;

	BVH[0] = node;
	
	for (int32 i = 0; i < 1 + 8; i++)
	{
		SetBVHNode(i);
	}
}

void ABezierPatch::SetBVHNode(int32 index)
{
	// 0 : 12345678
	// 1 : 9 ~ 16
	// 2 : 17 ~ 
	FTreeNode node1 = FTreeNode();
	FTreeNode node2 = FTreeNode();
	FTreeNode node3 = FTreeNode();
	FTreeNode node4 = FTreeNode();
	FTreeNode node5 = FTreeNode();
	FTreeNode node6 = FTreeNode();
	FTreeNode node7 = FTreeNode();
	FTreeNode node8 = FTreeNode();

	int midX = (BVH[index].minX + BVH[index].maxX) / 2;
	int midY = (BVH[index].minY + BVH[index].maxY) / 2;
	int midZ = (BVH[index].minZ + BVH[index].maxZ) / 2;

	node1.minX = BVH[index].minX;
	node1.maxX = midX;
	node1.minY = midY;
	node1.maxY = BVH[index].maxY;
	node1.minZ = midZ;
	node1.maxZ = BVH[index].maxZ;
	BVH[index * 8 + 1] = node1;

	node2.minX = midX; 
	node2.maxX = BVH[index].maxX;
	node2.minY = midY;
	node2.maxY = BVH[index].maxY;
	node2.minZ = midZ;
	node2.maxZ = BVH[index].maxZ;
	BVH[index * 8 + 2] = node2;

	node3.minX = BVH[index].minX;
	node3.maxX = midX;
	node3.minY = BVH[index].minY;;
	node3.maxY = midY;
	node3.minZ = midZ;
	node3.maxZ = BVH[index].maxZ;
	BVH[index * 8 + 3] = node3;

	node4.minX = midX;
	node4.maxX = BVH[index].maxX;
	node4.minY = BVH[index].minY;;
	node4.maxY = midY;
	node4.minZ = midZ;
	node4.maxZ = BVH[index].maxZ;
	BVH[index * 8 + 4] = node4;

	node5.minX = BVH[index].minX;
	node5.maxX = midX;
	node5.minY = midY;
	node5.maxY = BVH[index].maxY;
	node5.minZ = BVH[index].minZ;
	node5.maxZ = midZ;
	BVH[index * 8 + 5] = node5;

	node6.minX = midX;
	node6.maxX = BVH[index].maxX;
	node6.minY = midY;
	node6.maxY = BVH[index].maxY;
	node6.minZ = BVH[index].minZ;
	node6.maxZ = midZ;
	BVH[index * 8 + 6] = node6;

	node7.minX = BVH[index].minX;
	node7.maxX = midX;
	node7.minY = midY;
	node7.maxY = BVH[index].maxY;
	node7.minZ = BVH[index].minZ;
	node7.maxZ = midZ;
	BVH[index * 8 + 7] = node7;

	node8.minX = midX;
	node8.maxX = BVH[index].maxX;
	node8.minY = midY;
	node8.maxY = BVH[index].maxY;
	node8.minZ = BVH[index].minZ;
	node8.maxZ = midZ;
	BVH[index * 8 + 8] = node8;
}

void ABezierPatch::FindBVHLeaf()
{
	FVector position = bullet->GetActorLocation();

	int bulletMinX = position.X - 50;
	int bulletMaxX = position.X + 50;
	int bulletMinY = position.Y - 50;
	int bulletMaxY = position.Y + 50;
	int bulletMinZ = position.Z - 50;
	int bulletMaxZ = position.Z + 50;

	TQueue<int32> queue;
	int32 leafThreshold = 1 + 8 - 1;

	queue.Enqueue(0);

	while (!queue.IsEmpty())
	{
		int32 index;
		queue.Dequeue(index);

		if (index > leafThreshold)
		{
			UE_LOG(LogTemp, Log, TEXT("%d"), index);

			UE_LOG(LogTemp, Log, TEXT("X : %d ~  %d, Y : %d ~  %d, Z : %d ~  %d, "), BVH[index].minX, BVH[index].maxX, BVH[index].minY, BVH[index].maxY, BVH[index].minZ, BVH[index].maxZ);

			if (BVH[index].minX <= position.X &&
				BVH[index].maxX >= position.X &&
				BVH[index].minY <= position.Y &&
				BVH[index].maxY >= position.Y &&
				BVH[index].minZ <= position.Z &&
				BVH[index].maxZ >= position.Z)
			{
				int x = (BVH[index].minX + BVH[index].maxX) / 2;
				int y = (BVH[index].minY + BVH[index].maxY) / 2;
				int z = (BVH[index].minZ + BVH[index].maxZ) / 2;

				FVector drawPoint = FVector(x, y, z);

				int ex = (BVH[index].maxX - BVH[index].minX);
				int ey = (BVH[index].maxY - BVH[index].minY);
				int ez = (BVH[index].maxZ - BVH[index].minZ);

				FVector extent = FVector(ex, ey, ez);

				DrawDebugBox(GetWorld(), drawPoint, extent, FColor::Purple, true, -1, 0, 10);
			}
			
		}
		else
		{
			if (BVH[index].minX <= position.X &&
				BVH[index].maxX >= position.X &&
				BVH[index].minY <= position.Y &&
				BVH[index].maxY >= position.Y &&
				BVH[index].minZ <= position.Z &&
				BVH[index].maxZ >= position.Z)
			{

				UE_LOG(LogTemp, Log, TEXT("%d"), index);

				UE_LOG(LogTemp, Log, TEXT("X : %d ~  %d, Y : %d ~  %d, Z : %d ~  %d, "), BVH[index].minX, BVH[index].maxX, BVH[index].minY, BVH[index].maxY, BVH[index].minZ, BVH[index].maxZ);

				int x = (BVH[index].minX + BVH[index].maxX) / 2;
				int y = (BVH[index].minY + BVH[index].maxY) / 2;
				int z = (BVH[index].minZ + BVH[index].maxZ) / 2;

				FVector drawPoint = FVector(x, y, z);

				int ex = (BVH[index].maxX - BVH[index].minX);
				int ey = (BVH[index].maxY - BVH[index].minY);
				int ez = (BVH[index].maxZ - BVH[index].minZ);

				FVector extent = FVector(ex, ey, ez);

				DrawDebugBox(GetWorld(), drawPoint, extent, FColor::Purple, true, -1, 0, 10);

				for (int32 i = 1; i < 9; i++)
				{
					queue.Enqueue(index * 8 + i);
				}
			}
		}
	}
}

