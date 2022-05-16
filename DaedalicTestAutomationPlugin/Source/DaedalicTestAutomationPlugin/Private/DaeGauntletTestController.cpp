#include "DaeGauntletTestController.h"
#include "DaeGauntletStates.h"
#include "DaeTestLogCategory.h"
#include "DaeTestReportWriter.h"
#include "DaeTestReportWriterSet.h"
#include "DaeTestSuiteActor.h"
#include "Settings/DaeTestAutomationPluginSettings.h"
#include <AssetRegistryModule.h>
#include <EngineUtils.h>
#include <Engine/AssetManager.h>
#include <Kismet/GameplayStatics.h>

void UDaeGauntletTestController::OnInit()
{
    Super::OnInit();

    // Get tests path.
    const UDaeTestAutomationPluginSettings* TestAutomationPluginSettings =
        GetDefault<UDaeTestAutomationPluginSettings>();

    for (const FString& TestMapFolder : TestAutomationPluginSettings->TestMapFolders)
    {
        UE_LOG(LogDaeTest, Display, TEXT("Discovering tests from: %s"), *TestMapFolder);
    }

    // Build list of tests (based on FAutomationEditorCommonUtils::CollectTestsByClass).
    FAssetRegistryModule& AssetRegistryModule =
        FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
    TArray<FAssetData> AssetDataArray;

    AssetRegistryModule.Get().SearchAllAssets(true);
    AssetRegistryModule.Get().GetAssetsByClass(UWorld::StaticClass()->GetFName(), AssetDataArray);

    for (auto ObjIter = AssetDataArray.CreateConstIterator(); ObjIter; ++ObjIter)
    {
        const FAssetData& AssetData = *ObjIter;

        FString FileName = FPackageName::LongPackageNameToFilename(AssetData.ObjectPath.ToString());
        FName MapName = AssetData.AssetName;

        const bool bIsTestMap = TestAutomationPluginSettings->IsTestMap(FileName, MapName);

        if (bIsTestMap)
        {
            MapNames.Add(MapName);

            UE_LOG(LogDaeTest, Display, TEXT("Discovered test: %s"), *MapName.ToString());
        }
    }

    // Set console variables.
    for (auto& ConsoleVariable : TestAutomationPluginSettings->ConsoleVariables)
    {
        IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(*ConsoleVariable.Key);

        if (CVar)
        {
            CVar->Set(*ConsoleVariable.Value);

            UE_LOG(LogDaeTest, Log, TEXT("Setting console variable %s = %s"), *ConsoleVariable.Key,
                   *ConsoleVariable.Value);
        }
    }

    GetGauntlet()->BroadcastStateChange(FDaeGauntletStates::Initialized);
}

void UDaeGauntletTestController::OnPostMapChange(UWorld* World)
{
    Super::OnPostMapChange(World);

    UE_LOG(LogDaeTest, Log, TEXT("UDaeGauntletTestController::OnPostMapChange - World: %s"),
           *World->GetName());

    if (GetCurrentState() != FDaeGauntletStates::LoadingNextMap)
    {
        return;
    }

    GetGauntlet()->BroadcastStateChange(FDaeGauntletStates::DiscoveringTests);
}

void UDaeGauntletTestController::OnTick(float TimeDelta)
{
    if (GetCurrentState() == FDaeGauntletStates::Initialized)
    {
        // If this isn't a test map (e.g. immediately after startup), load first test map now.
        if (!MapNames.Contains(FName(*GetCurrentMap())))
        {
            UE_LOG(LogDaeTest, Log,
                   TEXT("FDaeGauntletStates::Initialized - World is not a test world, "
                        "loading first test world."));

            MapIndex = -1;
            LoadNextTestMap();
            return;
        }
        else
        {
            GetGauntlet()->BroadcastStateChange(FDaeGauntletStates::DiscoveringTests);
        }
    }
    else if (GetCurrentState() == FDaeGauntletStates::LoadingNextMap)
    {
        UE_LOG(LogDaeTest, Display, TEXT("FDaeGauntletStates::LoadingNextMap - Loading map: %s"),
               *MapNames[MapIndex].ToString());

        UGameplayStatics::OpenLevel(this, MapNames[MapIndex]);
    }
    else if (GetCurrentState() == FDaeGauntletStates::DiscoveringTests)
    {
        // Find test suite.
        ADaeTestSuiteActor* TestSuite = nullptr;

        for (TActorIterator<ADaeTestSuiteActor> ActorIt(GetWorld()); ActorIt; ++ActorIt)
        {
            TestSuite = *ActorIt;
        }

        if (!IsValid(TestSuite))
        {
            UE_LOG(LogDaeTest, Error,
                   TEXT("FDaeGauntletStates::DiscoveringTests - No "
                        "DaeGauntletTestSuiteActor "
                        "found."));
            LoadNextTestMap();
            return;
        }

        // Start first test.
        GetGauntlet()->BroadcastStateChange(FDaeGauntletStates::Running);

        TestSuite->OnTestSuiteSuccessful.AddDynamic(
            this, &UDaeGauntletTestController::OnTestSuiteFinished);
        TestSuite->OnTestSuiteFailed.AddDynamic(this,
                                                &UDaeGauntletTestController::OnTestSuiteFinished);

        TestSuite->RunAllTests();
    }
}

void UDaeGauntletTestController::LoadNextTestMap()
{
    ++MapIndex;

	// Gather command line options.
	const FString SingleTestName = ParseCommandLineOption(TEXT("TestName"));
	const FString TestTagsString = ParseCommandLineOption(TEXT("TestTags"));
	const FString TestPriorityString = ParseCommandLineOption(TEXT("TestPriority"));

    // Check if we just want to run a single test.
    if (!SingleTestName.IsEmpty())
    {
        // Increment MapIndex until we found the map.
        while (MapNames.IsValidIndex(MapIndex) && MapNames[MapIndex].ToString() != SingleTestName)
        {
            ++MapIndex;
        }
    }
	else if (!TestTagsString.IsEmpty())
	{
		// Check if we just want to run tests with a given tag.
		TArray<FString> RequiredTestTags;
		const TCHAR* TagSeparator = TEXT(";");
		TestTagsString.ParseIntoArray(RequiredTestTags, TagSeparator);
		if (RequiredTestTags.Num() > 0)
		{
			bool bHasFoundTagInTest = false;
            while (!bHasFoundTagInTest)
            {
                // Found no test map with the required tag.
				if (!MapNames.IsValidIndex(MapIndex))
				{
					break;
				}

                const FString TestName = FPaths::GetBaseFilename(MapNames[MapIndex].ToString());
                bHasFoundTagInTest = DoesMapHasTag(TestName, RequiredTestTags);

				if (!bHasFoundTagInTest)
				{
					++MapIndex;
				}
            }
		}
	}
	if (!TestPriorityString.IsEmpty())
    {
        const UDaeTestAutomationPluginSettings* TestAutomationPluginSettings =
            GetDefault<UDaeTestAutomationPluginSettings>();
        const TMap<FString, FDaeTestMapMetaData>& TestMapsMetaData =
            TestAutomationPluginSettings->TestMapsMetaData;

        const EDaeTestPriority RequiredPriority = ConvertStringToPriority(TestPriorityString);
		bool bHasTestRequiredPriority = false;
		while (!bHasTestRequiredPriority)
		{
			// Found no test map with the required tag.
			if (!MapNames.IsValidIndex(MapIndex))
			{
				break;
			}

			EDaeTestPriority TestPriority = EDaeTestPriority::DTP_Default;
			// Check if the test has a non default priority.
            const FString TestName = FPaths::GetBaseFilename(MapNames[MapIndex].ToString());
			if (TestMapsMetaData.Contains(TestName))
			{
				// Check if the test has one of the required tags.
				TestPriority = TestMapsMetaData[TestName].Priority;
			}
			if (TestPriority >= RequiredPriority)
			{
				bHasTestRequiredPriority = true;
			}

			if (!bHasTestRequiredPriority)
			{
				++MapIndex;
			}
		}
	}

    if (MapNames.IsValidIndex(MapIndex))
    {
        // Load next test map in next tick. This is to avoid invocation list changes during OnPostMapChange.
        GetGauntlet()->BroadcastStateChange(FDaeGauntletStates::LoadingNextMap);
    }
    else
    {
        // All tests finished.
        UE_LOG(LogDaeTest, Display,
               TEXT("UDaeGauntletTestController::LoadNextTestMap - All tests finished."));

        // Finish Gauntlet.
        GetGauntlet()->BroadcastStateChange(FDaeGauntletStates::Finished);

        for (const FDaeTestSuiteResult& Result : Results)
        {
            if (Result.NumFailedTests() > 0)
            {
                EndTest(1);
                return;
            }
        }

        EndTest(0);
    }
}

bool UDaeGauntletTestController::DoesMapHasTag(const FString& TestName,
                                               const TArray<FString>& RequiredTags) const
{
    const UDaeTestAutomationPluginSettings* TestAutomationPluginSettings =
        GetDefault<UDaeTestAutomationPluginSettings>();
    const TMap<FString, FDaeTestMapMetaData>& TestMapsMetaData =
        TestAutomationPluginSettings->TestMapsMetaData;
    // Check if the test has tags.
    if (TestMapsMetaData.Contains(TestName))
    {
        // Check if the test has one of the required tags.
        const TArray<FString> Tags = TestMapsMetaData[TestName].Tags;
        for (auto& Tag : Tags)
        {
            if (RequiredTags.Contains(Tag))
            {
                return true;
            }
        }
    }

    return false;
}

EDaeTestPriority UDaeGauntletTestController::ConvertStringToPriority(
    const FString& TestPriorityString)
{
    if (TestPriorityString.Contains("Critical", ESearchCase::IgnoreCase)
        || TestPriorityString.Equals("3"))
    {
        return EDaeTestPriority::DTP_CriticalPriority;
    }
    if (TestPriorityString.Contains("High", ESearchCase::IgnoreCase)
        || TestPriorityString.Equals("2"))
    {
        return EDaeTestPriority::DTP_HighPriority;
    }
    if (TestPriorityString.Contains("Medium", ESearchCase::IgnoreCase)
        || TestPriorityString.Equals("1"))
    {
        return EDaeTestPriority::DTP_MediumPriority;
    }
    if (TestPriorityString.Contains("Low", ESearchCase::IgnoreCase)
        || TestPriorityString.Equals("0"))
    {
        return EDaeTestPriority::DTP_LowPriority;
    }

    return EDaeTestPriority::DTP_Default;
}

void UDaeGauntletTestController::OnTestSuiteFinished(ADaeTestSuiteActor* TestSuite)
{
    // Store result.
    Results.Add(TestSuite->GetResult());

    // Update test reports on disk.
    const FString ReportPath = ParseCommandLineOption(TEXT("ReportPath"));

    const FDaeTestReportWriterSet ReportWriters = TestSuite->GetReportWriters();

    for (const TSharedPtr<FDaeTestReportWriter> ReportWriter : ReportWriters.GetReportWriters())
    {
        ReportWriter->WriteReport(Results, ReportPath);
    }

    // Proceed with next test.
    LoadNextTestMap();
}

FString UDaeGauntletTestController::ParseCommandLineOption(const FString& Key) const
{
    FString Value;
    FParse::Value(FCommandLine::Get(), *Key, Value);
    return Value.Mid(1);
}
