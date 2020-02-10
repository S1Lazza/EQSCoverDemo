// Fill out your copyright notice in the Description page of Project Settings.

#include "CoverMesh.h"

// Sets default values
ACoverMesh::ACoverMesh()
	: m_IsPivotCentered(true),
	  m_PointsPerSideX(1),
	  m_PointsPerSideY(1)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void ACoverMesh::BeginPlay()
{
	Super::BeginPlay();
	m_CoverMesh = FindComponentByClass<UStaticMeshComponent>();
}

// Called every frame
void ACoverMesh::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

