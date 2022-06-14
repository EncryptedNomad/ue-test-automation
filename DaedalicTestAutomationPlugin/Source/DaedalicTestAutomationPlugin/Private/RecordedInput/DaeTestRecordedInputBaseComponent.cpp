#include "RecordedInput/DaeTestRecordedInputBaseComponent.h"
#include "RecordedInput/DaeTestRecordInterface.h"
#include "DaeTestInputBlueprintFunctionLibrary.h"
#include <EngineUtils.h>
#include <Serialization/ArchiveLoadCompressedProxy.h>
#include <SaveGameSystem.h>
#include <GameFramework/PlayerInput.h>

UDaeTestRecordedInputBaseComponent::UDaeTestRecordedInputBaseComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bTickEvenWhenPaused = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
}

bool UDaeTestRecordedInputBaseComponent::HasRecordInterface(const AActor* Actor)
{
    return Actor->GetClass()->ImplementsInterface(UDaeTestRecordInterface::StaticClass());
}

TArray<uint8> UDaeTestRecordedInputBaseComponent::BytesFromString(const FString& String)
{
    const uint32 Size = String.Len();

    TArray<uint8> Bytes;
    Bytes.AddUninitialized(Size);
    StringToBytes(String, Bytes.GetData(), Size);

    return Bytes;
}

FString UDaeTestRecordedInputBaseComponent::StringFromBytes(const TArray<uint8>& Bytes)
{

    const uint32 Size = Bytes.Num();
    FString String = BytesToString(Bytes.GetData(), Size);

    return String;
}

void UDaeTestRecordedInputBaseComponent::Initialize()
{
    // Set the FullSavePath to "Game\Saved\Automation\RecordedTestData\".

    const FString FolderForRecordings =
        FPaths::Combine(FPaths::AutomationDir(), TEXT("RecordedTestData"));
    const FString OwnerName = GetOwner()->GetName();
    const FString Suffix = TEXT("_Recording");
    FullSavePath = FPaths::Combine(FolderForRecordings, GetWorld()->GetName(), OwnerName + Suffix);
}
