#include "AutomationTestFramework/DaeTestAutomationPluginAutomationTestFrameworkIntegration.h"
#include "DaeTestEditorLogCategory.h"
#include "Settings/DaeTestAutomationPluginSettings.h"
#include <FileHelpers.h>
#include <Misc/PackageName.h>
#include <Misc/Paths.h>

void FDaeTestAutomationPluginAutomationTestFrameworkIntegration::DiscoverTests()
{
    const UDaeTestAutomationPluginSettings* TestAutomationPluginSettings =
        GetDefault<UDaeTestAutomationPluginSettings>();

    for (const FString& TestMapFolder : TestAutomationPluginSettings->TestMapFolders)
    {
        UE_LOG(LogDaeTestEditor, Log, TEXT("Discovering tests from: %s"), *TestMapFolder);
    }

    // Unregister existing tests.
    Tests.Empty();

    // Register new tests (based on FLoadAllMapsInEditorTest).
    TArray<FString> PackageFiles;
    FEditorFileUtils::FindAllPackageFiles(PackageFiles);

    // Iterate over all files, adding the ones with the map extension.
    for (const FString& FileName : PackageFiles)
    {
        const bool bIsMap = FPaths::GetExtension(FileName, true)
                            == FPackageName::GetMapPackageExtension();
        FName MapName = FName(*FPaths::GetBaseFilename(FileName));

        if (bIsMap && TestAutomationPluginSettings->IsTestMap(FileName, MapName))
        {
            const FDaeTestMapMetaData& MapMetaData =
                TestAutomationPluginSettings->TestMapsMetaData.FindRef(MapName.ToString());
            TSharedPtr<FDaeTestAutomationPluginAutomationTestFrameworkTest> NewTest = MakeShareable(
                new FDaeTestAutomationPluginAutomationTestFrameworkTest(FileName, MapMetaData));
            Tests.Add(NewTest);

            UE_LOG(LogDaeTestEditor, Log, TEXT("Discovered test: %s"), *NewTest->GetMapName());
        }
    }
}
