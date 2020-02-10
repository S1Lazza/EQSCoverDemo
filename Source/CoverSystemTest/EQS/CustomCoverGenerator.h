// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Templates/SubclassOf.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMesh.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "DataProviders/AIDataProvider.h"
#include "EnvironmentQuery/Generators/EnvQueryGenerator_ProjectedPoints.h"
#include "CustomCoverGenerator.generated.h"

UCLASS(meta = (DisplayName = "Cover Points Generator"))
class COVERSYSTEMTEST_API UEQSCoverPointsGenerator : public UEnvQueryGenerator_ProjectedPoints
{
	GENERATED_UCLASS_BODY()

public:
	virtual void GenerateItems(FEnvQueryInstance& QueryInstance) const override;

	/** Class of the actors to search in the world for. */
	UPROPERTY(EditDefaultsOnly, Category = Generator, meta = (DisplayName = "ActorToFilter"))
	TSubclassOf<AActor> m_ActorClassToFiler;

	/** Context aka actor of interest around which the query is generated */
	UPROPERTY(EditAnywhere, Category = Generator, meta = (DisplayName = "SearchCenter"))
	TSubclassOf<UEnvQueryContext> m_SearchCenter;

	/** Max distance of path between point and context.  NOTE: Zero and negative values will never return any results if
	  * UseRadius is true.  "Within" requires Distance < Radius.  Actors ON the circle (Distance == Radius) are excluded. */
	UPROPERTY(EditDefaultsOnly, Category = Generator, meta = (DisplayName = "SearchRadius"))
	FAIDataProviderFloatValue m_SearchRadius;

	/** If true, this will only returns actors of the specified class within the SearchRadius of the SearchCenter context.  If false, it will return ALL actors of the specified class in the world. */
	UPROPERTY(EditDefaultsOnly, Category = Generator, meta = (DisplayName = "GenerateOnlyActorInRadius"))
	FAIDataProviderBoolValue m_GenerateOnlyActorsInRadius;

	/** Space between the points and the margin of the geometry they reference to */
	UPROPERTY(EditDefaultsOnly, Category = Generator, meta = (DisplayName = "PointsOffSetFromCover"))
	FAIDataProviderFloatValue m_OffSetFromCover;

	virtual FText GetDescriptionTitle() const override;
	virtual FText GetDescriptionDetails() const override;

private:
	TArray<int32>   PopulatePPSArray(int PointsPerSide) const;
	int				ElementsToReserve(TArray<AActor*>& ValidActors) const;
	TArray<AActor*> FindMatchingActors(FEnvQueryInstance& QueryInstance) const;
	TArray<FVector> FindBaseCoverPoints(TArray<AActor*>& ValidActor) const;
};