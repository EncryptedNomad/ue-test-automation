#pragma once

#include <CoreMinimal.h>
#include <UObject/Object.h>
#include "DaeTestAutomationPluginSettings.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FDaeTestAutomationPluginSettingsTestMapPathChangedSignature,
                                    const FString&);
DECLARE_MULTICAST_DELEGATE_OneParam(
    FDaeTestAutomationPluginSettingsAdditionalTestMapsChangedSignature, const TArray<FName>&);

/** Custom settings for this plugin. */
UCLASS(config = Game, defaultconfig)
class DAEDALICTESTAUTOMATIONPLUGIN_API UDaeTestAutomationPluginSettings : public UObject
{
    GENERATED_BODY()

public:
    /** Path to look for test maps in. */
    UPROPERTY(config, EditAnywhere)
    FString TestMapPath;

    /** Names of additional maps to test. */
    UPROPERTY(config, EditAnywhere)
    TArray<FName> AdditionalTestMaps;

    /** Console variables to set before running batches of tests (e.g. Automation Window, Gauntlet). */
    UPROPERTY(config, EditAnywhere)
    TMap<FString, FString> ConsoleVariables;

    UDaeTestAutomationPluginSettings();

#if WITH_EDITOR
    virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

    /** Event when the path to look for test maps in has changed. */
    FDaeTestAutomationPluginSettingsTestMapPathChangedSignature OnTestMapPathChanged;

    /** Event when the names of additional maps to test have changed. */
    FDaeTestAutomationPluginSettingsAdditionalTestMapsChangedSignature OnAdditionalTestMapsChanged;
};
