#pragma once

#include "DaeTestSuiteResult.h"
#include "Settings/DaeTestMapMetaData.h"
#include <CoreMinimal.h>
#include <GauntletTestController.h>
#include "DaeGauntletTestController.generated.h"

class ADaeTestSuiteActor;

/** Controller for automated tests run by Gauntlet. */
UCLASS()
class DAEDALICTESTAUTOMATIONPLUGIN_API UDaeGauntletTestController : public UGauntletTestController
{
    GENERATED_BODY()

public:
    virtual void OnInit() override;
    virtual void OnPostMapChange(UWorld* World) override;
    virtual void OnTick(float TimeDelta) override;

private:
    TArray<FName> MapNames;
    int32 MapIndex;
    TArray<FDaeTestSuiteResult> Results;

    void LoadNextTestMap();
    /** Does the test has one of the required tags? */
    bool DoesMapHasTag(const FString& TestName, const TArray<FString>& RequiredTags) const;
    static EDaeTestPriority ConvertStringToPriority(const FString& TestPriorityString);

    UFUNCTION()
    void OnTestSuiteFinished(ADaeTestSuiteActor* TestSuite);

    FString ParseCommandLineOption(const FString& Key) const;
};
