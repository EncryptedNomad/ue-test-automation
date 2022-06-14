#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DaeTestRecordInterface.generated.h"

UINTERFACE(Category = "TestAutomation", BlueprintType, meta = (DisplayName = "Dae Test Record Interface"))
class UDaeTestRecordInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * @brief Every actor that implements this interface will be recorded in automated test scenes.
 */
class DAEDALICTESTAUTOMATIONPLUGIN_API IDaeTestRecordInterface
{
	GENERATED_BODY()

public:
	/**
	* Holds the array of components that you want to record the transform for the automated test.
	* @param Components - The Components that you want to save with the Actor.
	*/
	UFUNCTION(BlueprintNativeEvent, Category = "TestAutomation")
    void ComponentsToRecord(TArray<USceneComponent*>& Components);
};
