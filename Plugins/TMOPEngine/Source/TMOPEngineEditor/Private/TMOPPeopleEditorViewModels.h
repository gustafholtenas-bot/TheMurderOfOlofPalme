#pragma once

#include "CoreMinimal.h"
#include "People/TMOPPersonProfileTypes.h"
#include "TMOPPeopleEditorViewModels.generated.h"

/** Characteristics-only projection of a person row for the middle editor panel. */
USTRUCT()
struct FTMOPPersonCharacteristicsEditorData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category="Characteristics",
        meta=(DisplayName="Age at Event"))
    int32 AgeAtEvent = 0;

    UPROPERTY(EditAnywhere, Category="Characteristics",
        meta=(ClampMin="0.0", DisplayName="Height Centimeters"))
    float HeightCentimeters = 0.0f;

    UPROPERTY(EditAnywhere, Category="Visual Appearance",
        meta=(DisplayName="Runtime Appearance Profile"))
    FTMOPAppearanceProfile AppearanceProfile;

    UPROPERTY(EditAnywhere, Category="Held Items")
    FTMOPHeldItemDefinition LeftHandItem;

    UPROPERTY(EditAnywhere, Category="Held Items")
    FTMOPHeldItemDefinition RightHandItem;

    UPROPERTY(EditAnywhere, Category="Held Items", meta=(TitleProperty="ItemId"))
    TArray<FTMOPHeldItemDefinition> AdditionalCarriedItems;

    UPROPERTY(EditAnywhere, Category="Image Reference",
        meta=(DisplayName="Reference Image"))
    TSoftObjectPtr<UTexture2D> ReferenceImage;

    UPROPERTY(EditAnywhere, Category="Head")
    ETMOPHairColor HairColorCategory = ETMOPHairColor::Unknown;

    UPROPERTY(EditAnywhere, Category="Head")
    ETMOPHeadwearType HeadwearCategory = ETMOPHeadwearType::Unknown;

    UPROPERTY(EditAnywhere, Category="Head")
    ETMOPFacialHairType FacialHairCategory =
        ETMOPFacialHairType::Unknown;

    UPROPERTY(EditAnywhere, Category="Head")
    FTMOPAppearanceSlot Hair;

    UPROPERTY(EditAnywhere, Category="Head")
    FTMOPAppearanceSlot BeardOrMustache;

    UPROPERTY(EditAnywhere, Category="Head")
    FTMOPAppearanceSlot FaceShape;

    UPROPERTY(EditAnywhere, Category="Head")
    FTMOPAppearanceSlot Nose;

    UPROPERTY(EditAnywhere, Category="Head")
    FTMOPAppearanceSlot Scarf;

    UPROPERTY(EditAnywhere, Category="Head")
    FTMOPAppearanceSlot Glasses;

    UPROPERTY(EditAnywhere, Category="Head")
    FTMOPAppearanceSlot Headwear;

    UPROPERTY(EditAnywhere, Category="Upper Body")
    ETMOPBodyBuild BodyBuildCategory = ETMOPBodyBuild::Unknown;

    UPROPERTY(EditAnywhere, Category="Upper Body")
    ETMOPOuterwearType OuterwearCategory =
        ETMOPOuterwearType::Unknown;

    UPROPERTY(EditAnywhere, Category="Upper Body")
    FTMOPAppearanceSlot BodyBuild;

    UPROPERTY(EditAnywhere, Category="Upper Body")
    FTMOPAppearanceSlot JacketOrCoat;

    UPROPERTY(EditAnywhere, Category="Upper Body")
    FTMOPAppearanceSlot ShirtOrSweater;

    UPROPERTY(EditAnywhere, Category="Lower Body")
    FTMOPAppearanceSlot Trousers;

    UPROPERTY(EditAnywhere, Category="Lower Body")
    FTMOPAppearanceSlot Shoes;

    UPROPERTY(EditAnywhere, Category="Other Characteristics")
    FTMOPAppearanceSlot OtherCharacteristics;
};

/** Timeline-independent general data projection for the right editor panel. */
USTRUCT()
struct FTMOPPersonGeneralEditorData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category="Identity", meta=(DisplayName="Entity ID"))
    FName EntityId = NAME_None;

    UPROPERTY(EditAnywhere, Category="Identity", meta=(DisplayName="Category ID"))
    FName CategoryId = NAME_None;

    UPROPERTY(EditAnywhere, Category="Identity")
    FText FullName;

    UPROPERTY(EditAnywhere, Category="Identity")
    FText FirstName;

    UPROPERTY(EditAnywhere, Category="Identity")
    FText LastName;

    UPROPERTY(EditAnywhere, Category="Identity")
    ETMOPPersonGender Gender = ETMOPPersonGender::Unknown;

    UPROPERTY(EditAnywhere, Category="Identity")
    FString Nationality;

    UPROPERTY(EditAnywhere, Category="Identity")
    FString Occupation;

    UPROPERTY(EditAnywhere, Category="History")
    FString HistoricalAddress;

    UPROPERTY(EditAnywhere, Category="History")
    int32 BirthYear = 0;

    UPROPERTY(EditAnywhere, Category="Source")
    FString GeneralSourceReference;

    UPROPERTY(EditAnywhere, Category="Source",
        meta=(DisplayName="Uppslag"))
    FString Uppslag;

    UPROPERTY(EditAnywhere, Category="Simulation")
    TSubclassOf<ATMOPHistoricalAgent> AgentClass;

    UPROPERTY(EditAnywhere, Category="Simulation")
    bool bSpawnInSimulation = true;

    UPROPERTY(EditAnywhere, Category="Simulation",
        meta=(DisplayName="Main Character"))
    bool bMainCharacter = false;

    UPROPERTY(EditAnywhere, Category="Simulation")
    FTMOPMovementProfile MovementProfile;

    UPROPERTY(EditAnywhere, Category="Vehicles")
    TArray<FName> AssociatedVehicleIds;

    UPROPERTY(EditAnywhere, Category="Group")
    FName SocialGroupId = NAME_None;

    UPROPERTY(EditAnywhere, Category="Group")
    FName GroupLeaderEntityId = NAME_None;

    UPROPERTY(EditAnywhere, Category="Group")
    ETMOPGroupFormation GroupFormation = ETMOPGroupFormation::SideBySide;

    UPROPERTY(EditAnywhere, Category="Group", meta=(ClampMin="30.0"))
    float GroupFormationSpacingCm = 110.0f;

    UPROPERTY(EditAnywhere, Category="Group")
    bool bFollowGroupLeaderSchedule = true;

    UPROPERTY(EditAnywhere, Category="Dialog")
    FTMOPPersonDialog Dialog;

    UPROPERTY(EditAnywhere, Category="Automatic Speech",
        meta=(TitleProperty="LineId"))
    TArray<FTMOPTimedSpeechLine> AutomaticSpeech;

    UPROPERTY(EditAnywhere, Category="Notes", meta=(MultiLine="true"))
    FString Notes;
};
