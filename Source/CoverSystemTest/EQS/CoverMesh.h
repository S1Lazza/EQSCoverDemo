// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "CoverMesh.generated.h"

UCLASS()
class COVERSYSTEMTEST_API ACoverMesh : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACoverMesh();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, Category = CoverAttributes, meta = (DisplayName = "IsPivotCentered"))
	bool m_IsPivotCentered;

	UPROPERTY(EditAnywhere, Category = MeshAttributes, meta = (DisplayName = "PointsPerSideX"))
	int m_PointsPerSideX;

	UPROPERTY(EditAnywhere, Category = MeshAttributes, meta = (DisplayName = "PointsPerSideY"))
	int m_PointsPerSideY;

	UPROPERTY(EditDefaultsOnly, Category = MeshAttributes, meta = (DisplayName = "CoverMesh"))
	UStaticMeshComponent* m_CoverMesh;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

};
