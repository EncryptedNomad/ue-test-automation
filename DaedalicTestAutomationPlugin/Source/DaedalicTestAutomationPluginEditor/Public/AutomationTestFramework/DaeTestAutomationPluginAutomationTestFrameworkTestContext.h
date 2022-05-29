#pragma once

#include <CoreMinimal.h>

class ADaeTestSuiteActor;

/** Context to run a single test for the Unreal Automation Test Framework within. */
class FDaeTestAutomationPluginAutomationTestFrameworkTestContext
{
public:
    ADaeTestSuiteActor* CurrentTestSuite;
    TMap<FString, FString> OldConsoleVariables;

    /** Associated automation test; all warnings, errors, etc. are routed to the automation test to track */
    FAutomationTestBase* CurTest;

    FDaeTestAutomationPluginAutomationTestFrameworkTestContext();
};
