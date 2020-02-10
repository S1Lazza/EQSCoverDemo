// Fill out your copyright notice in the Description page of Project Settings.

#include "CustomCoverGenerator.h"
#include "EngineGlobals.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "Engine/StaticMesh.h"
#include "CoverMesh.h"
#include "AI/Navigation/NavigationTypes.h"
#include "Math/UnrealMathUtility.h"
#include "EnvironmentQuery/Contexts/EnvQueryContext_Querier.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"

#define LOCTEXT_NAMESPACE "EnvQueryGenerator"

UEQSCoverPointsGenerator::UEQSCoverPointsGenerator(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	m_ActorClassToFiler = ACoverMesh::StaticClass();
	m_SearchCenter = UEnvQueryContext_Querier::StaticClass();
	m_SearchRadius.DefaultValue = 500.0f;
	m_GenerateOnlyActorsInRadius.DefaultValue = false;
	m_OffSetFromCover.DefaultValue = 50.0f;
}

void UEQSCoverPointsGenerator::GenerateItems(FEnvQueryInstance& QueryInstance) const
{
	TArray<AActor*> MatchingActors = FindMatchingActors(QueryInstance);

	if (MatchingActors.Num() == 0)
	{
		return;
	};

	TArray<FVector> CoverPointsLocation = FindBaseCoverPoints(MatchingActors);
	TArray<FNavLocation> CoverNavPoints;
	CoverNavPoints.Reserve(CoverPointsLocation.Num());

	for (int32 it = 0; it < CoverPointsLocation.Num(); it++)
	{
		const FNavLocation CoverNavLocation = FNavLocation(CoverPointsLocation[it]);
		CoverNavPoints.Add(CoverNavLocation);
	}

	ProjectAndFilterNavPoints(CoverNavPoints, QueryInstance);
	StoreNavPoints(CoverNavPoints, QueryInstance);
}

TArray<int32> UEQSCoverPointsGenerator::PopulatePPSArray(int PointsPerSide) const
{
	TArray<int32> PPS;
	int Range = FMath::TruncateToHalfIfClose(PointsPerSide / 2, 0.5);

	for (int32 it = 0; it <= Range; it++)
	{
		if (it != 0)
		{
			PPS.Add(it);
			PPS.Add(-it);
		}
		else
		{
			PPS.Add(it);
		}
	}
	return PPS;
}

int UEQSCoverPointsGenerator::ElementsToReserve(TArray<AActor*>& ValidActors) const
{
	int NumberOfSides = 2;
	int TotalAmount = 0;

	for (int32 it = 0; it < ValidActors.Num(); it++)
	{
		ACoverMesh* Cover = CastChecked<ACoverMesh>(ValidActors[it]);
		
		checkf(Cover->m_PointsPerSideX % 2 != 0 || Cover->m_PointsPerSideY % 2 != 0, TEXT("The number inserted is even, to work correctly an odd number is required"));
		checkf(Cover->m_PointsPerSideX > 0 || Cover->m_PointsPerSideY > 0, TEXT("The number inserted is negative, insert a positive number"));
		
		TotalAmount += (Cover->m_PointsPerSideX + Cover->m_PointsPerSideY) * NumberOfSides;
	}
	return TotalAmount;
}

TArray<AActor*> UEQSCoverPointsGenerator::FindMatchingActors(FEnvQueryInstance & QueryInstance) const
{
	if (m_ActorClassToFiler == nullptr)
	{
		return TArray<AActor*>();
	}

	UObject* QueryOwner = QueryInstance.Owner.Get();
	if (QueryOwner == nullptr)
	{
		return TArray<AActor*>();
	}

	UWorld* World = GEngine->GetWorldFromContextObject(QueryOwner, EGetWorldErrorMode::LogAndReturnNull);
	if (World == nullptr)
	{
		return TArray<AActor*>();
	}

	m_GenerateOnlyActorsInRadius.BindData(QueryOwner, QueryInstance.QueryID);
	bool bUseRadius = m_GenerateOnlyActorsInRadius.GetValue();

	TArray<FVector> ContextLocations;
	QueryInstance.PrepareContext(m_SearchCenter, ContextLocations);

	TArray<AActor*> MatchingActors;
	if (bUseRadius)
	{
		m_SearchRadius.BindData(QueryOwner, QueryInstance.QueryID);
		const float RadiusValue = m_SearchRadius.GetValue();
		const float RadiusSq = FMath::Square(RadiusValue);

		for (TActorIterator<AActor> ItActor = TActorIterator<AActor>(World, m_ActorClassToFiler); ItActor; ++ItActor)
		{
			for (int32 ContextIndex = 0; ContextIndex < ContextLocations.Num(); ++ContextIndex)
			{
				if (FVector::DistSquared(ContextLocations[ContextIndex], ItActor->GetActorLocation()) < RadiusSq)
				{
					MatchingActors.Add(*ItActor);
					break;
				}
			}
		}
	}
	else
	{	// If radius is not positive, ignore Search Center and Search Radius
		for (TActorIterator<AActor> ItActor = TActorIterator<AActor>(World, m_ActorClassToFiler); ItActor; ++ItActor)
		{
			MatchingActors.Add(*ItActor);
		}
	}

	return MatchingActors;
}

TArray<FVector> UEQSCoverPointsGenerator::FindBaseCoverPoints(TArray<AActor*>& ValidActors) const
{
	TArray<FVector> CoverPointsLocation;
	int TotalPoints = ElementsToReserve(ValidActors);
	CoverPointsLocation.Reserve(TotalPoints);

	for (int32 it = 0; it < ValidActors.Num(); it++)
	{
		UStaticMeshComponent* ActorMesh = ValidActors[it]->FindComponentByClass<UStaticMeshComponent>();
		ACoverMesh* Cover = CastChecked<ACoverMesh>(ValidActors[it]);

		if (ActorMesh && ActorMesh->GetMemberNameChecked_StaticMesh() != NAME_None)
		{
			FVector ActorLocation = ValidActors[it]->GetActorLocation();
			FVector MeshScale = ValidActors[it]->GetActorScale();
			FVector Fwd = ValidActors[it]->GetActorForwardVector();
			FVector Right = ValidActors[it]->GetActorRightVector();

			FVector MeshMinBounds, MeshMaxBounds;
			ActorMesh->GetLocalBounds(MeshMinBounds, MeshMaxBounds);
			FVector MaxBoundsRotatedX = MeshMaxBounds * MeshScale.X * Fwd;
			FVector MaxBoundsRotatedY = MeshMaxBounds * MeshScale.Y * Right;

			FVector OffSetFromCover(m_OffSetFromCover.GetValue(), m_OffSetFromCover.GetValue(), m_OffSetFromCover.GetValue());
			FVector OffSetFromCoverX = OffSetFromCover * Fwd;
			FVector OffSetFromCoverY = OffSetFromCover * Right; 

			int SideDividerX = FMath::TruncateToHalfIfClose(Cover->m_PointsPerSideX / 2, 0.5);
			int SideDividerY = FMath::TruncateToHalfIfClose(Cover->m_PointsPerSideY / 2, 0.5);
			FVector PointsOffSetX((MaxBoundsRotatedY) / SideDividerX);
			FVector PointsOffSetY((MaxBoundsRotatedX) / SideDividerY);

			FVector PointOffSetX;
			FVector PointOffSetY;

			TArray<int32> PointsValuePerSideX = PopulatePPSArray(Cover->m_PointsPerSideX);
			for (int32 it_ = 0; it_ < PointsValuePerSideX.Num(); it_++)
			{
				if (it_ != 0)
				{
					PointOffSetX = FVector(PointsValuePerSideX[it_] * PointsOffSetX.X, PointsValuePerSideX[it_] * PointsOffSetX.Y, 0);
				}
				else
				{
					PointOffSetX = FVector(0.0f, 0.0f, 0.0f);
				}

				FVector Point1((ActorLocation.X + MaxBoundsRotatedX.X + OffSetFromCoverX.X + PointOffSetX.X), (ActorLocation.Y + MaxBoundsRotatedX.Y + OffSetFromCoverX.Y + PointOffSetX.Y), ActorLocation.Z);
				FVector Point2((ActorLocation.X - MaxBoundsRotatedX.X - OffSetFromCoverX.X - PointOffSetX.X), (ActorLocation.Y - MaxBoundsRotatedX.Y - OffSetFromCoverX.Y - PointOffSetX.Y), ActorLocation.Z);

				CoverPointsLocation.Add(Point1);
				CoverPointsLocation.Add(Point2);
			}

			TArray<int32> PointsValuePerSideY = PopulatePPSArray(Cover->m_PointsPerSideY);
			for (int32 it_ = 0; it_ < PointsValuePerSideY.Num(); it_++)
			{
				if (it_ != 0)
				{
					PointOffSetY = FVector(PointsValuePerSideY[it_] * PointsOffSetY.X, PointsValuePerSideY[it_] * PointsOffSetY.Y, 0);
				}
				else
				{
					PointOffSetY = FVector(0.0f, 0.0f, 0.0f);
				}

				FVector Point3((ActorLocation.X + MaxBoundsRotatedY.X + OffSetFromCoverY.X + PointOffSetY.X), (ActorLocation.Y + MaxBoundsRotatedY.Y + OffSetFromCoverY.Y + PointOffSetY.Y), ActorLocation.Z);
				FVector Point4((ActorLocation.X - MaxBoundsRotatedY.X - OffSetFromCoverY.X - PointOffSetY.X), (ActorLocation.Y - MaxBoundsRotatedY.Y - OffSetFromCoverY.Y - PointOffSetY.Y), ActorLocation.Z);
				
				CoverPointsLocation.Add(Point3);
				CoverPointsLocation.Add(Point4);
			}
		}
	}
	return CoverPointsLocation;
}

FText UEQSCoverPointsGenerator::GetDescriptionTitle() const
{
	return FText::Format(LOCTEXT("CoverPointsDescriptionGenerateAroundContext", "{0}: generate around {1}"),
		Super::GetDescriptionTitle(), UEnvQueryTypes::DescribeContext(m_SearchCenter));
};

FText UEQSCoverPointsGenerator::GetDescriptionDetails() const
{
	FText Desc = FText::Format(LOCTEXT("CoverPointsDescription", "Radius: {0}, GenerateOnlyActorsInRadius: {1}, OffSet: {2}"),
		FText::FromString(m_SearchRadius.ToString()), FText::FromString(m_GenerateOnlyActorsInRadius.ToString()), FText::FromString(m_OffSetFromCover.ToString()));

	FText ProjDesc = ProjectionData.ToText(FEnvTraceData::Brief);
	if (!ProjDesc.IsEmpty())
	{
		FFormatNamedArguments ProjArgs;
		ProjArgs.Add(TEXT("Description"), Desc);
		ProjArgs.Add(TEXT("ProjectionDescription"), ProjDesc);
		Desc = FText::Format(LOCTEXT("CoverPointsDescriptionWithProjection", "{Description}, {ProjectionDescription}"), ProjArgs);
	}

	return Desc;
}

#undef LOCTEXT_NAMESPACE


