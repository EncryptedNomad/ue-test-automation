#pragma once

#include "DaeTestMapMetaData.h"
#include "DaeTestMapSettings.h"
#include <CoreMinimal.h>
#include <UObject/Object.h>
#include "DaeTestAutomationPluginSettings.generated.h"

DECLARE_MULTICAST_DELEGATE(FDaeTestAutomationPluginSettingsTestMapsChangedSignature);

/** Custom settings for this plugin. */
UCLASS(config = Game, defaultconfig)
class DAEDALICTESTAUTOMATIONPLUGIN_API UDaeTestAutomationPluginSettings : public UObject
{
    GENERATED_BODY()

public:
    /** DEPRECATED: Use TestMapFolders instead. */
    UPROPERTY(config)
    FString TestMapPath;

    /** Paths to look for test maps in, relative to the Content root of your project (e.g. Maps/AutomatedTests). */
	UPROPERTY(config, EditAnywhere, Category = "General")
    TArray<FString> TestMapFolders;

    /** Names of additional maps to test. */
	UPROPERTY(config, EditAnywhere, Category = "General")
    TArray<FName> AdditionalTestMaps;

    /** Names of maps to ignore when found in test map folders. */
	UPROPERTY(config, EditAnywhere, Category = "General")
    TArray<FName> IgnoredMaps;

    /** Console variables to set before running batches of tests (e.g. Automation Window, Gauntlet). */
	UPROPERTY(config, EditAnywhere, Category = "General")
	TMap<FString, FString> ConsoleVariables;

	/** Additional information about test maps. */
	UPROPERTY(config)
	TMap<FString, FDaeTestMapMetaData> TestMapsMetaData;

	/** Create categories and subcategories for the tests in the session frontend. */
	UPROPERTY(config, EditAnywhere, Category = "Session Frontend")
	bool bUseFolderStructureAsCategories = false;

	/** Global settings for each test. */
	UPROPERTY(config, EditAnywhere, Category = "Global Settings")
	FDaeTestMapSettings GlobalTestMapSettings;

    UDaeTestAutomationPluginSettings();
	
	static void SetTestMetaData(const AActor* TestActor, const FDaeTestMapMetaData& TestMetaData);

    virtual void PostInitProperties() override;

#if WITH_EDITOR
    virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif

    /** Event when the set of test maps has changed. */
    FDaeTestAutomationPluginSettingsTestMapsChangedSignature OnTestMapsChanged;

    /** Checks whether the specified map contains automated tests to run. */
    bool IsTestMap(const FString& FileName, const FName& MapName) const;
};
