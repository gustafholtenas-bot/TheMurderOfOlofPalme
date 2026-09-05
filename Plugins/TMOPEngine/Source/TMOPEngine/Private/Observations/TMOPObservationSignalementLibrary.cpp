#include "Observations/TMOPObservationSignalementLibrary.h"

namespace
{
bool HasAgeRange(const FTMOPObservationWitnessSignalement& Signalement)
{
    return Signalement.EstimatedAgeMinimum > 0 &&
        Signalement.EstimatedAgeMaximum >= Signalement.EstimatedAgeMinimum;
}

bool HasHeightRange(const FTMOPObservationWitnessSignalement& Signalement)
{
    return Signalement.EstimatedHeightMinimumCm > 0.0f &&
        Signalement.EstimatedHeightMaximumCm >=
            Signalement.EstimatedHeightMinimumCm;
}

FString TraitLabel(const ETMOPSignalementTraitType Type)
{
    if (const UEnum* Enum = StaticEnum<ETMOPSignalementTraitType>())
        return Enum->GetDisplayNameTextByValue(
            static_cast<int64>(Type)).ToString();
    return TEXT("trait");
}

void CompareWitnessPair(
    const FTMOPObservationWitnessSignalement& First,
    const FTMOPObservationWitnessSignalement& Second,
    float& InOutSupportingWeight,
    float& InOutContradictingWeight,
    FTMOPSignalementComparison& OutComparison)
{
    if (HasAgeRange(First) && HasAgeRange(Second))
    {
        OutComparison.bHasComparableEvidence = true;
        const bool bOverlaps = First.EstimatedAgeMinimum <=
                Second.EstimatedAgeMaximum &&
            Second.EstimatedAgeMinimum <= First.EstimatedAgeMaximum;
        (bOverlaps ? InOutSupportingWeight : InOutContradictingWeight) +=
            bOverlaps ? 1.0f : 1.5f;
        (bOverlaps ? OutComparison.SupportingTraits :
            OutComparison.ContradictingTraits).AddUnique(TEXT("age"));
    }

    if (HasHeightRange(First) && HasHeightRange(Second))
    {
        OutComparison.bHasComparableEvidence = true;
        const bool bOverlaps = First.EstimatedHeightMinimumCm <=
                Second.EstimatedHeightMaximumCm &&
            Second.EstimatedHeightMinimumCm <=
                First.EstimatedHeightMaximumCm;
        (bOverlaps ? InOutSupportingWeight : InOutContradictingWeight) +=
            bOverlaps ? 1.0f : 1.5f;
        (bOverlaps ? OutComparison.SupportingTraits :
            OutComparison.ContradictingTraits).AddUnique(TEXT("height"));
    }

    for (const FTMOPObservedSignalementTrait& FirstTrait : First.Traits)
    {
        for (const FTMOPObservedSignalementTrait& SecondTrait : Second.Traits)
        {
            if (FirstTrait.TraitType != SecondTrait.TraitType)
                continue;

            bool bPairComparable = false;
            bool bPairSupports = false;
            bool bPairContradicts = false;
            for (const FName Value : FirstTrait.NormalizedValues)
            {
                bPairComparable |= SecondTrait.NormalizedValues.Contains(Value) ||
                    SecondTrait.ExplicitlyExcludedValues.Contains(Value);
                bPairSupports |= SecondTrait.NormalizedValues.Contains(Value);
                bPairContradicts |=
                    SecondTrait.ExplicitlyExcludedValues.Contains(Value);
            }
            for (const FName Value : SecondTrait.NormalizedValues)
            {
                bPairComparable |=
                    FirstTrait.ExplicitlyExcludedValues.Contains(Value);
                bPairContradicts |=
                    FirstTrait.ExplicitlyExcludedValues.Contains(Value);
            }

            if (!bPairComparable)
                continue;
            OutComparison.bHasComparableEvidence = true;
            const FString Label = TraitLabel(FirstTrait.TraitType);
            if (bPairSupports)
            {
                InOutSupportingWeight += 1.0f;
                OutComparison.SupportingTraits.AddUnique(Label);
            }
            if (bPairContradicts)
            {
                InOutContradictingWeight += 1.5f;
                OutComparison.ContradictingTraits.AddUnique(Label);
            }
        }
    }
}
}

bool UTMOPObservationSignalementLibrary::HasUsableSignalement(
    const FTMOPObservationDefinition& Observation)
{
    for (const FTMOPObservationWitnessSignalement& Signalement :
        Observation.WitnessSignalements)
    {
        if (HasAgeRange(Signalement) || HasHeightRange(Signalement) ||
            !Signalement.Traits.IsEmpty())
            return true;
    }
    return false;
}

FTMOPSignalementComparison
UTMOPObservationSignalementLibrary::CompareSignalements(
    const FTMOPObservationDefinition& FirstObservation,
    const FTMOPObservationDefinition& SecondObservation)
{
    FTMOPSignalementComparison Result;
    float SupportingWeight = 0.0f;
    float ContradictingWeight = 0.0f;

    for (const FTMOPObservationWitnessSignalement& First :
        FirstObservation.WitnessSignalements)
        for (const FTMOPObservationWitnessSignalement& Second :
            SecondObservation.WitnessSignalements)
            CompareWitnessPair(First, Second, SupportingWeight,
                ContradictingWeight, Result);

    const float TotalWeight = SupportingWeight + ContradictingWeight;
    Result.CompatibilityScore = TotalWeight > KINDA_SMALL_NUMBER
        ? SupportingWeight / TotalWeight : 0.5f;
    if (!Result.bHasComparableEvidence)
        Result.Summary = TEXT("No comparable structured signalement evidence.");
    else
        Result.Summary = FString::Printf(
            TEXT("Compatibility %.0f%%: %d supporting and %d contradicting trait groups."),
            Result.CompatibilityScore * 100.0f,
            Result.SupportingTraits.Num(), Result.ContradictingTraits.Num());
    return Result;
}
