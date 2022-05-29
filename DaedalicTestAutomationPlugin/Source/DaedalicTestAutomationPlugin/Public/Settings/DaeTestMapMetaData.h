#pragma once

#include <CoreMinimal.h>
#include "DaeTestMapMetaData.generated.h"

/** Priority of a test. */
UENUM()
enum class EDaeTestPriority : uint8
{
	// The highest priority possible. Showstopper/blocker.
	DTP_CriticalPriority = 3 UMETA(DisplayName = "Critical"),
	// High priority. Major feature functionality etc.
	DTP_HighPriority = 2 UMETA(DisplayName = "High"),
	// Medium Priority. Minor feature functionality, major generic content issues.
	DTP_MediumPriority = 1 UMETA(DisplayName = "Medium"),
	// Low Priority. Minor content bugs. String errors. Etc.
	DTP_LowPriority = 0 UMETA(DisplayName = "Low"),

	// Default priority is medium.
	DTP_Default = DTP_MediumPriority UMETA(Hidden)
};

/**
 * Adds a regex pattern to an internal list that a test will expect to encounter in error or warning logs during its execution.
 * If an expected pattern is not encountered, it will cause this test to fail.
 */
USTRUCT()
struct FDaeTestExpectedError
{
	GENERATED_BODY()

	/** The expected message string. Supports basic regex patterns. */
	UPROPERTY(EditAnywhere)
	FString ExpectedErrorPattern;

	/** How many times to expect this error string to be seen. If > 0, the error must be seen the exact number of times. */
	UPROPERTY(EditAnywhere)
	int32 Occurrences = 1;
};

/**
 * @brief Metadata for a test level.
 * We store this data in a config file so that we can read necessary data about the test without opening the map.
 */
USTRUCT()
struct FDaeTestMapMetaData
{
	GENERATED_BODY()

	/** When running tests from the console, you can select specific tags, only tests that have one or more of the required tags
	 * will be run. */
	UPROPERTY(EditAnywhere)
	TArray<FString> Tags;

	/** When running tests from the console, you can select a priority, only tests with the given priority or higher will be run. */
	UPROPERTY(EditAnywhere)
	EDaeTestPriority Priority = EDaeTestPriority::DTP_Default;

	/** Errors to be expected while processing this test. */
	UPROPERTY(EditAnywhere)
	TArray<FDaeTestExpectedError> ExpectedErrors;
};
