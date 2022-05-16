#include "AutomationTestFramework/DaeTestAutomationPluginAutomationTestFrameworkTest.h"
#include "AutomationTestFramework/DaeTestAutomationPluginAutomationTestFrameworkCommands.h"
#include "DaeTestEditorLogCategory.h"
#include "Settings/DaeTestAutomationPluginSettings.h"
#include <Misc/Paths.h>
#include <Tests/AutomationEditorCommon.h>

FDaeTestAutomationPluginAutomationTestFrameworkTest::FDaeTestAutomationPluginAutomationTestFrameworkTest(
	const FString& InMapName, const FDaeTestMapMetaData& InMapMetaData)
	: FAutomationTestBase(InMapName, false), MapName(InMapName), MapMetaData(InMapMetaData)
{
    // Test is automatically registered in FAutomationTestBase base class constructor.

    const UDaeTestAutomationPluginSettings* TestAutomationPluginSettings =
        GetMutableDefault<UDaeTestAutomationPluginSettings>();
    TestMapSettings = TestAutomationPluginSettings->GlobalTestMapSettings;

    for (const auto& ExpectedError : MapMetaData.ExpectedErrors)
    {
        AddExpectedError(ExpectedError.ExpectedErrorPattern,
                         EAutomationExpectedErrorFlags::Contains, ExpectedError.Occurrences);
    }
}

bool FDaeTestAutomationPluginAutomationTestFrameworkTest::SuppressLogErrors()
{
	return TestMapSettings.bSuppressLogErrors;
}

bool FDaeTestAutomationPluginAutomationTestFrameworkTest::SuppressLogWarnings()
{
	return TestMapSettings.bSuppressLogWarnings;
}

bool FDaeTestAutomationPluginAutomationTestFrameworkTest::SuppressLogs()
{
	return TestMapSettings.bSuppressLogs;
}

bool FDaeTestAutomationPluginAutomationTestFrameworkTest::ElevateLogWarningsToErrors()
{
	return TestMapSettings.bElevateLogWarningsToErrors;
}

uint32 FDaeTestAutomationPluginAutomationTestFrameworkTest::GetTestFlags() const
{
    return EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter;
}

uint32 FDaeTestAutomationPluginAutomationTestFrameworkTest::GetRequiredDeviceNum() const
{
    return 1;
}

FString FDaeTestAutomationPluginAutomationTestFrameworkTest::GetTestSourceFileName() const
{
    return GetMapName();
}

int32 FDaeTestAutomationPluginAutomationTestFrameworkTest::GetTestSourceFileLine() const
{
    return 0;
}

FString FDaeTestAutomationPluginAutomationTestFrameworkTest::GetMapName() const
{
    return MapName;
}

void FDaeTestAutomationPluginAutomationTestFrameworkTest::GetTests(
    TArray<FString>& OutBeautifiedNames, TArray<FString>& OutTestCommands) const
{
    OutBeautifiedNames.Add(GetBeautifiedTestName());
    OutTestCommands.Add(FString());
}

bool FDaeTestAutomationPluginAutomationTestFrameworkTest::RunTest(const FString& Parameters)
{
    UE_LOG(LogDaeTestEditor, Display, TEXT("Running test for map: %s"), *MapName);
	
    Context.CurTest = this;

    ADD_LATENT_AUTOMATION_COMMAND(FDaeTestAutomationPluginApplyConsoleVariables(Context));
    ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(MapName));
    ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
    ADD_LATENT_AUTOMATION_COMMAND(FDaeTestAutomationPluginWaitForEndOfTestSuite(Context));
    ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	ADD_LATENT_AUTOMATION_COMMAND(FDaeTestAutomationPluginRevertConsoleVariables(Context));
	ADD_LATENT_AUTOMATION_COMMAND(FDaeTestAutomationPluginCleanUp(Context));

    return true;
}

FString FDaeTestAutomationPluginAutomationTestFrameworkTest::GetBeautifiedTestName() const
{
	FString BeautifiedTestName = FPaths::GetBaseFilename(MapName);

	const UDaeTestAutomationPluginSettings* TestAutomationPluginSettings = GetMutableDefault<UDaeTestAutomationPluginSettings>();
	// Use the folder path of the test to create categories.
	if (TestAutomationPluginSettings->bUseFolderStructureAsCategories)
	{
		FString TestPath = FPaths::GetPath(MapName);
		// Remains false if the test is one of the AdditionalTestMaps.
        bool bHasFoundTestInTestMapFolders = false;
		for (auto& TestFolderName : TestAutomationPluginSettings->TestMapFolders)
		{
            bHasFoundTestInTestMapFolders = TestPath.Contains(TestFolderName);
            if (bHasFoundTestInTestMapFolders)
			{
				FString RelativePath;
				TestPath.Split(*TestFolderName, &RelativePath, &TestPath);
				break;
			}
		}
        if (bHasFoundTestInTestMapFolders)
        {
            TestPath = TestPath.Replace(TEXT("/"), TEXT("."));
            BeautifiedTestName = TEXT("DaedalicTestAutomationPlugin") + TestPath + TEXT(".")
                                 + BeautifiedTestName;

            return BeautifiedTestName;
        }
	}

	return TEXT("DaedalicTestAutomationPlugin.") + BeautifiedTestName;
}
