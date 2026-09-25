#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TMOPLanguageSubsystem.generated.h"

/** Application-wide language, shared by all local players and independent of save slots. */
UCLASS()
class TMOPENGINE_API UTMOPLanguageSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category="TMOP|Language")
    bool SetLanguage(const FString& Language, bool bSave = true);

    UFUNCTION(BlueprintPure, Category="TMOP|Language")
    FString GetLanguage() const;

    static bool IsSupportedLanguage(const FString& Language);
    static void RegisterMenuTranslations();
};
